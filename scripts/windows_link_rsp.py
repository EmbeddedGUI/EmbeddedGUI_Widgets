import argparse
import pathlib
import subprocess
import sys


DEFAULT_BATCH_LENGTH_LIMIT = 18000


def strip_quotes(token: str) -> str:
    if len(token) >= 2 and token[0] == token[-1] and token[0] in ('"', "'"):
        return token[1:-1]
    return token


def is_object_token(token: str) -> bool:
    value = strip_quotes(token)
    return value.lower().endswith((".o", ".obj"))


def collect_link_command(lines):
    candidate = None
    for raw_line in lines:
        line = raw_line.strip()
        if not line:
            continue
        if " -o " in f" {line} " and any(ext in line.lower() for ext in (".o", ".obj")):
            candidate = line
    return candidate


def command_length(tokens) -> int:
    return sum(len(str(token)) + 3 for token in tokens)


def parse_make_vars(make_args):
    values = {}
    for token in make_args:
        if "=" not in token:
            continue
        name, value = token.split("=", 1)
        if not name:
            continue
        values[name] = strip_quotes(value)
    return values


def resolve_make_path(sdk_root: str, value: str) -> pathlib.Path:
    path = pathlib.Path(value)
    if path.is_absolute():
        return path
    return pathlib.Path(sdk_root) / path


def prepare_dry_run_stamp_dir(sdk_root: str, make_args) -> None:
    make_vars = parse_make_vars(make_args)
    objroot_path = make_vars.get("OBJROOT_PATH") or make_vars.get("OUTPUT_PATH") or "output"
    app_obj_suffix = make_vars.get("APP_OBJ_SUFFIX") or make_vars.get("APP") or "default"
    port = make_vars.get("PORT", "")
    build_obj_suffix = app_obj_suffix + (f"_{port}" if port else "")
    objdir = resolve_make_path(sdk_root, objroot_path) / "obj" / build_obj_suffix
    objdir.mkdir(parents=True, exist_ok=True)


def chunk_targets(base_cmd, targets, make_args, length_limit: int):
    chunk = []
    for target in targets:
        probe = [*base_cmd, *chunk, target, *make_args]
        if chunk and command_length(probe) > length_limit:
            yield chunk
            chunk = [target]
        else:
            chunk.append(target)
    if chunk:
        yield chunk


def run_make_targets(sdk_root: str, targets, make_args, length_limit: int) -> int:
    base_cmd = ["make", "-C", sdk_root]
    unique_targets = list(dict.fromkeys(targets))

    for chunk in chunk_targets(base_cmd, unique_targets, make_args, length_limit):
        result = subprocess.run([*base_cmd, *chunk, *make_args])
        if result.returncode != 0:
            return result.returncode
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Build Windows PC test targets and link with a response file.")
    parser.add_argument("--sdk-root", required=True)
    parser.add_argument("--batch-length-limit", type=int, default=DEFAULT_BATCH_LENGTH_LIMIT)
    parser.add_argument("make_args", nargs=argparse.REMAINDER)
    args = parser.parse_args()

    make_args = list(args.make_args)
    if make_args and make_args[0] == "--":
        make_args = make_args[1:]

    prepare_dry_run_stamp_dir(args.sdk_root, make_args)

    dry_run_cmd = ["make", "-C", args.sdk_root, "-n", "all", *make_args]
    dry_run = subprocess.run(dry_run_cmd, capture_output=True, text=True)

    if dry_run.returncode != 0:
        sys.stderr.write(dry_run.stdout)
        sys.stderr.write(dry_run.stderr)
        return dry_run.returncode

    link_line = collect_link_command(dry_run.stdout.splitlines())
    if link_line is None:
        return subprocess.run(["make", "-C", args.sdk_root, "all", *make_args]).returncode

    tokens = link_line.split()
    object_tokens = [strip_quotes(token) for token in tokens if is_object_token(token)]
    if not object_tokens:
        sys.stderr.write("[windows-link-rsp] no object files found in link command\n")
        return 1

    build_result = run_make_targets(args.sdk_root, object_tokens, make_args, args.batch_length_limit)
    if build_result != 0:
        return build_result

    missing_objects = [token for token in object_tokens if not pathlib.Path(token).exists()]
    if missing_objects:
        sys.stderr.write("[windows-link-rsp] missing object files after object build:\n")
        for token in missing_objects[:20]:
            sys.stderr.write(f"  {token}\n")
        if len(missing_objects) > 20:
            sys.stderr.write(f"  ... and {len(missing_objects) - 20} more\n")
        return 1

    rsp_index = None
    final_tokens = []
    for token in tokens:
        if is_object_token(token):
            if rsp_index is None:
                rsp_index = len(final_tokens)
                final_tokens.append("")
            continue
        final_tokens.append(strip_quotes(token))

    output_path = None
    for index, token in enumerate(final_tokens[:-1]):
        if token == "-o":
            output_path = pathlib.Path(final_tokens[index + 1])
            break
    if output_path is None:
        sys.stderr.write("[windows-link-rsp] failed to resolve output path from link command\n")
        return 1

    rsp_path = output_path.with_suffix(output_path.suffix + ".rsp")
    rsp_path.parent.mkdir(parents=True, exist_ok=True)
    rsp_path.write_text("\n".join(object_tokens) + "\n", encoding="ascii")
    final_tokens[rsp_index] = f"@{rsp_path.as_posix()}"

    print(f'Linking    : "{output_path}"')
    result = subprocess.run(final_tokens)
    if result.returncode == 0:
        print(f"[windows-link-rsp] linked via response file: {output_path}")
        print(f'Building   : "{output_path}"')
        print("Executing 'all' complete!")
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
