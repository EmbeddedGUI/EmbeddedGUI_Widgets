import argparse
import pathlib
import subprocess
import sys


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


def main() -> int:
    parser = argparse.ArgumentParser(description="Fallback linker for Windows PC builds using a response file.")
    parser.add_argument("--sdk-root", required=True)
    parser.add_argument("make_args", nargs=argparse.REMAINDER)
    args = parser.parse_args()

    make_args = list(args.make_args)
    if make_args and make_args[0] == "--":
        make_args = make_args[1:]

    dry_run_cmd = ["make", "-C", args.sdk_root, "-n", "all", *make_args]
    dry_run = subprocess.run(dry_run_cmd, capture_output=True, text=True)

    if dry_run.returncode != 0:
        sys.stderr.write(dry_run.stdout)
        sys.stderr.write(dry_run.stderr)
        return dry_run.returncode

    link_line = collect_link_command(dry_run.stdout.splitlines())
    if link_line is None:
        sys.stderr.write("[windows-link-rsp] failed to locate link command in make dry-run output\n")
        return 1

    tokens = link_line.split()
    object_tokens = [strip_quotes(token) for token in tokens if is_object_token(token)]
    if not object_tokens:
        sys.stderr.write("[windows-link-rsp] no object files found in link command\n")
        return 1

    missing_objects = [token for token in object_tokens if not pathlib.Path(token).exists()]
    if missing_objects:
        sys.stderr.write("[windows-link-rsp] missing object files, build likely failed before link stage:\n")
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

    result = subprocess.run(final_tokens)
    if result.returncode == 0:
        print(f"[windows-link-rsp] linked via response file: {output_path}")
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
