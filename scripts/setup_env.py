#!/usr/bin/env python3
"""EmbeddedGUI Widgets environment setup entrypoint."""

from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import ssl
import subprocess
import sys
import urllib.request


MIN_PYTHON_VERSION = (3, 8)
DEFAULT_VENV_DIR = ".venv"
PIP_INDEX_URLS = [
    ("TUNA mirror", "https://pypi.tuna.tsinghua.edu.cn/simple"),
    ("PyPI", "https://pypi.org/simple"),
]

W64DEVKIT_VERSION = "2.5.0"
W64DEVKIT_FILENAME = f"w64devkit-x64-{W64DEVKIT_VERSION}.7z.exe"
W64DEVKIT_URLS = [
    f"https://ghfast.top/https://github.com/skeeto/w64devkit/releases/download/v{W64DEVKIT_VERSION}/{W64DEVKIT_FILENAME}",
    f"https://github.com/skeeto/w64devkit/releases/download/v{W64DEVKIT_VERSION}/{W64DEVKIT_FILENAME}",
]
W64DEVKIT_SHA256 = ""

EMSDK_ENV_KEYS = ("EMSDK_PATH", "EMSDK")
EMSDK_GIT_URLS = [
    "https://ghfast.top/https://github.com/emscripten-core/emsdk.git",
    "https://github.com/emscripten-core/emsdk.git",
]
DEFAULT_EMSDK_VERSION = os.environ.get("EMSDK_VERSION", "latest")
EMSDK_COMMAND_TIMEOUT = 3600


def project_root() -> Path:
    return Path(__file__).resolve().parents[1]


def is_windows() -> bool:
    return os.name == "nt"


def run(command: list[str], *, cwd: Path | None = None, env: dict[str, str] | None = None, capture_output: bool = False, timeout: float | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command,
        cwd=str(cwd) if cwd else None,
        env=env,
        text=True,
        capture_output=capture_output,
        timeout=timeout,
    )


def log_header(title: str) -> None:
    print()
    print("=" * 40)
    print(f"  {title}")
    print("=" * 40)


def display_path(path: Path, root_dir: Path | None = None) -> str:
    try:
        if root_dir is not None:
            return str(path.resolve().relative_to(root_dir.resolve())).replace("\\", "/")
    except ValueError:
        pass
    return str(path)


def find_command(name: str, env: dict[str, str] | None = None) -> str | None:
    return shutil.which(name, path=None if env is None else env.get("PATH"))


def prepend_path(env: dict[str, str], path: Path) -> dict[str, str]:
    updated = env.copy()
    updated["PATH"] = str(path) + os.pathsep + updated.get("PATH", "")
    return updated


def venv_python_path(venv_dir: Path) -> Path:
    if is_windows():
        return venv_dir / "Scripts" / "python.exe"
    return venv_dir / "bin" / "python"


def ensure_python_version() -> None:
    version = sys.version_info
    if version < MIN_PYTHON_VERSION:
        print(f"[!!] Python {version.major}.{version.minor} is too old. Need {MIN_PYTHON_VERSION[0]}.{MIN_PYTHON_VERSION[1]}+.")
        raise SystemExit(1)
    print(f"[OK] Python {version.major}.{version.minor}.{version.micro}")


def ensure_venv(root_dir: Path, venv_dir: Path) -> Path:
    venv_python = venv_python_path(venv_dir)
    if venv_python.exists():
        result = run([str(venv_python), "--version"], capture_output=True)
        if result.returncode == 0:
            print(f"[OK] Reusing virtual environment: {display_path(venv_dir, root_dir)}")
            return venv_python
        shutil.rmtree(venv_dir, ignore_errors=True)

    print(f"Creating virtual environment: {display_path(venv_dir, root_dir)}")
    result = run([sys.executable, "-m", "venv", str(venv_dir)], cwd=root_dir)
    if result.returncode != 0:
        print("[!!] Failed to create virtual environment.")
        raise SystemExit(1)
    return venv_python


def pip_install_with_fallback(venv_python: Path, args: list[str], label: str) -> bool:
    print(f"Installing {label} ...")
    last_result: subprocess.CompletedProcess[str] | None = None
    for index_name, index_url in PIP_INDEX_URLS:
        command = [
            str(venv_python),
            "-m",
            "pip",
            "install",
            "--disable-pip-version-check",
            "--progress-bar",
            "off",
            "-i",
            index_url,
        ] + args
        result = run(command, capture_output=True)
        last_result = result
        if result.returncode == 0:
            print(f"[OK] {label}")
            return True
        print(f"[!!] Failed via {index_name}: {' '.join(args)}")

    if last_result is not None:
        lines = [line.strip() for line in ((last_result.stdout or "") + "\n" + (last_result.stderr or "")).splitlines() if line.strip()]
        if lines:
            print("     Last pip output:")
            for line in lines[-10:]:
                print(f"       {line}")
    return False


def verify_python_environment(venv_python: Path) -> bool:
    verify_script = "\n".join(
        [
            "import json5",
            "import numpy",
            "from PIL import Image",
            "import freetype",
            "from elftools.elf.elffile import ELFFile",
            "print('ok')",
        ]
    )
    result = run([str(venv_python), "-c", verify_script], capture_output=True)
    if result.returncode == 0:
        print("[OK] Python dependency verification passed.")
        return True
    print("[!!] Python dependency verification failed.")
    print((result.stderr or result.stdout).strip())
    return False


def probe_python_dependencies(venv_python: Path) -> str:
    if not venv_python.exists():
        return "missing"
    verify_script = "\n".join(
        [
            "import json5",
            "import numpy",
            "from PIL import Image",
            "import freetype",
            "from elftools.elf.elffile import ELFFile",
        ]
    )
    result = run([str(venv_python), "-c", verify_script], capture_output=True)
    return "ready" if result.returncode == 0 else "broken"


def print_manual_python_help(root_dir: Path, venv_dir: Path) -> None:
    print("Manual recovery steps:")
    print(f"  {display_path(venv_python_path(venv_dir), root_dir)} -m pip install -r requirements.txt")
    if is_windows():
        print(f"  {display_path(venv_dir, root_dir)}\\Scripts\\activate.bat")
    else:
        print(f"  source {display_path(venv_dir, root_dir)}/bin/activate")


def install_python_environment(root_dir: Path, venv_dir: Path) -> bool:
    log_header("Python Setup")
    ensure_python_version()
    venv_python = ensure_venv(root_dir, venv_dir)
    if not pip_install_with_fallback(venv_python, ["--upgrade", "pip"], "pip upgrade"):
        print_manual_python_help(root_dir, venv_dir)
        return False
    if not pip_install_with_fallback(venv_python, ["-r", str(root_dir / "requirements.txt")], "requirements.txt"):
        print_manual_python_help(root_dir, venv_dir)
        return False
    if not verify_python_environment(venv_python):
        print_manual_python_help(root_dir, venv_dir)
        return False
    return True


def download_file(url: str, destination: Path) -> bool:
    print(f"Downloading: {url}")
    try:
        request = urllib.request.Request(url, headers={"User-Agent": "EmbeddedGUI-Widgets-Setup/1.0"})
        with urllib.request.urlopen(request, context=ssl.create_default_context(), timeout=120) as response:
            with destination.open("wb") as output:
                while True:
                    chunk = response.read(256 * 1024)
                    if not chunk:
                        break
                    output.write(chunk)
        return True
    except Exception as exc:
        print(f"[!!] Download failed: {exc}")
        destination.unlink(missing_ok=True)
        return False


def verify_checksum(file_path: Path, expected_sha256: str) -> bool:
    sha = hashlib.sha256()
    with file_path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            sha.update(chunk)
    return sha.hexdigest() == expected_sha256


def local_w64devkit_bin(root_dir: Path) -> Path:
    return root_dir / "tools" / "w64devkit" / "bin"


def ensure_windows_toolchain(root_dir: Path, auto_install: bool) -> tuple[dict[str, str], bool]:
    log_header("Windows Toolchain")
    env = os.environ.copy()
    local_bin = local_w64devkit_bin(root_dir)
    if local_bin.exists():
        env = prepend_path(env, local_bin)

    make_path = find_command("make.exe", env) or find_command("make", env)
    gcc_path = find_command("gcc.exe", env) or find_command("gcc", env)
    if make_path and gcc_path:
        print(f"[OK] make: {make_path}")
        print(f"[OK] gcc : {gcc_path}")
        return env, True

    if not auto_install:
        print("[!!] make/gcc not found. Install GNU Make + GCC or rerun setup on Windows.")
        return env, False

    tools_dir = root_dir / "tools"
    archive_path = tools_dir / W64DEVKIT_FILENAME
    gcc_exe = tools_dir / "w64devkit" / "bin" / "gcc.exe"
    tools_dir.mkdir(parents=True, exist_ok=True)
    for url in W64DEVKIT_URLS:
        if archive_path.exists() or download_file(url, archive_path):
            break
    if not archive_path.exists():
        print("[!!] Unable to download w64devkit.")
        return env, False
    if W64DEVKIT_SHA256 and not verify_checksum(archive_path, W64DEVKIT_SHA256):
        print("[!!] Checksum verification failed.")
        archive_path.unlink(missing_ok=True)
        return env, False
    result = run([str(archive_path), f"-o{tools_dir}", "-y"], cwd=root_dir, capture_output=True)
    if result.returncode != 0 or not gcc_exe.exists():
        print("[!!] Failed to extract w64devkit.")
        if result.stderr.strip():
            print(result.stderr.strip())
        return env, False
    archive_path.unlink(missing_ok=True)
    env = prepend_path(os.environ.copy(), local_bin)
    print(f"[OK] Installed w64devkit to: {display_path(local_bin.parent, root_dir)}")
    return env, True


def check_posix_toolchain() -> tuple[dict[str, str], bool]:
    log_header("Build Toolchain")
    env = os.environ.copy()
    make_path = find_command("make", env)
    gcc_path = find_command("gcc", env)
    if make_path and gcc_path:
        print(f"[OK] make: {make_path}")
        print(f"[OK] gcc : {gcc_path}")
        return env, True
    print("[!!] make or gcc not found.")
    print("  Debian/Ubuntu: sudo apt install build-essential")
    print("  Fedora      : sudo dnf install make gcc")
    print("  Arch Linux  : sudo pacman -S base-devel")
    return env, False


def emsdk_candidates(root_dir: Path, env: dict[str, str] | None = None) -> list[tuple[str, Path]]:
    search_env = env or os.environ
    candidates: list[tuple[str, Path]] = []
    for key in EMSDK_ENV_KEYS:
        value = search_env.get(key)
        if value:
            candidates.append((key, Path(value)))
    candidates.extend(
        [
            ("local", root_dir / "tools" / "emsdk"),
            ("sdk-submodule", root_dir / "sdk" / "EmbeddedGUI" / "tools" / "emsdk"),
            ("sibling-sdk", root_dir.parent / "EmbeddedGUI" / "tools" / "emsdk"),
        ]
    )
    return candidates


def emsdk_script_path(emsdk_root: Path) -> Path:
    return emsdk_root / ("emsdk.bat" if is_windows() else "emsdk")


def emsdk_env_script_path(emsdk_root: Path) -> Path:
    return emsdk_root / ("emsdk_env.bat" if is_windows() else "emsdk_env.sh")


def inject_emsdk_paths(env: dict[str, str], emsdk_root: Path) -> dict[str, str]:
    updated = env.copy()
    updated["EMSDK"] = str(emsdk_root)
    updated["EMSDK_PATH"] = str(emsdk_root)
    for candidate in reversed([emsdk_root, emsdk_root / "upstream" / "emscripten"]):
        if candidate.exists():
            updated = prepend_path(updated, candidate)
    for tool_dir_name in ("node", "python"):
        tool_dir = emsdk_root / tool_dir_name
        if not tool_dir.exists():
            continue
        for child in sorted(tool_dir.iterdir(), reverse=True):
            if not child.is_dir():
                continue
            updated = prepend_path(updated, child / "bin" if (child / "bin").exists() else child)
    em_config = Path.home() / ".emscripten"
    if em_config.exists():
        updated.setdefault("EM_CONFIG", str(em_config))
    return updated


def capture_emsdk_environment(emsdk_root: Path, env: dict[str, str]) -> tuple[dict[str, str] | None, str | None]:
    env_script = emsdk_env_script_path(emsdk_root)
    if not env_script.exists():
        return None, f"{display_path(env_script)} missing"

    activation_env = env.copy()
    activation_env["EMSDK_QUIET"] = "1"

    if is_windows():
        result = subprocess.run(
            f'call "{env_script}" >nul && set',
            cwd=str(emsdk_root),
            env=activation_env,
            text=True,
            capture_output=True,
            timeout=EMSDK_COMMAND_TIMEOUT,
            shell=True,
            executable=os.environ.get("COMSPEC", "cmd.exe"),
        )
    else:
        bash = find_command("bash", env) or "bash"
        result = run(
            [bash, "-lc", f'source "{env_script}" >/dev/null && env'],
            cwd=emsdk_root,
            env=activation_env,
            capture_output=True,
            timeout=EMSDK_COMMAND_TIMEOUT,
        )

    if result.returncode != 0:
        detail = (result.stderr or result.stdout).strip()
        return None, detail or "emsdk_env activation failed"

    activated_env: dict[str, str] = {}
    for line in result.stdout.splitlines():
        if "=" not in line or line.startswith("="):
            continue
        key, value = line.split("=", 1)
        if key:
            activated_env[key] = value

    if not activated_env:
        return None, "environment capture produced no output"
    return activated_env, None


def build_emsdk_runtime_env(emsdk_root: Path, env: dict[str, str]) -> dict[str, str]:
    activated_env, _ = capture_emsdk_environment(emsdk_root, env)
    if activated_env is not None:
        return activated_env
    return inject_emsdk_paths(env, emsdk_root)


def probe_emcc(root_dir: Path, env: dict[str, str]) -> tuple[bool, str]:
    emcc_path = find_command("emcc.bat", env) or find_command("emcc", env)
    if emcc_path:
        result = run([emcc_path, "--version"], env=env, capture_output=True)
        if result.returncode == 0:
            first_line = ((result.stdout or "") + (result.stderr or "")).splitlines()[0]
            return True, f"ready ({first_line})"
        return False, "broken (emcc found but failed to run)"
    for _, candidate in emsdk_candidates(root_dir, env):
        if candidate.exists():
            return False, f"missing (searched under {display_path(candidate, root_dir)})"
    return False, "missing"


def ensure_emsdk(root_dir: Path, env: dict[str, str], auto_install: bool, version: str) -> tuple[dict[str, str], bool]:
    log_header("Emscripten SDK")
    for source, candidate in emsdk_candidates(root_dir, env):
        if not candidate.exists():
            continue
        runtime_env = build_emsdk_runtime_env(candidate, env)
        ready, detail = probe_emcc(root_dir, runtime_env)
        if ready:
            print(f"[OK] EMSDK: {display_path(candidate, root_dir)} ({source})")
            print(f"[OK] emcc : {detail}")
            return runtime_env, True

    print("[!!] emcc not found in PATH, or the current emsdk is unusable.")
    if not auto_install:
        print("Rerun setup with --skip-emsdk if you do not need WASM builds.")
        return env, False

    local_emsdk = root_dir / "tools" / "emsdk"
    if not emsdk_script_path(local_emsdk).exists():
        git_path = find_command("git.exe", env) or find_command("git", env)
        if not git_path:
            print("[!!] git not found in PATH, cannot clone emsdk.")
            return env, False
        local_emsdk.parent.mkdir(parents=True, exist_ok=True)
        for url in EMSDK_GIT_URLS:
            print(f"Cloning emsdk repository from: {url}")
            result = run([git_path, "clone", "--depth", "1", url, str(local_emsdk)], cwd=root_dir, env=env, capture_output=True, timeout=EMSDK_COMMAND_TIMEOUT)
            if result.returncode == 0 and emsdk_script_path(local_emsdk).exists():
                break
            shutil.rmtree(local_emsdk, ignore_errors=True)
        if not emsdk_script_path(local_emsdk).exists():
            print("[!!] Unable to clone emsdk repository.")
            return env, False

    script = emsdk_script_path(local_emsdk)
    if is_windows():
        install_cmd = ["cmd.exe", "/c", str(script), "install", version]
        activate_cmd = ["cmd.exe", "/c", str(script), "activate", version]
    else:
        install_cmd = [str(script), "install", version]
        activate_cmd = [str(script), "activate", version]

    print(f"Installing Emscripten toolchain: {version}")
    if run(install_cmd, cwd=local_emsdk, env=env, capture_output=True, timeout=EMSDK_COMMAND_TIMEOUT).returncode != 0:
        print("[!!] Failed to install Emscripten toolchain.")
        return env, False
    print(f"Activating Emscripten toolchain: {version}")
    if run(activate_cmd, cwd=local_emsdk, env=env, capture_output=True, timeout=EMSDK_COMMAND_TIMEOUT).returncode != 0:
        print("[!!] Failed to activate Emscripten toolchain.")
        return env, False

    runtime_env = build_emsdk_runtime_env(local_emsdk, env)
    ready, detail = probe_emcc(root_dir, runtime_env)
    if ready:
        print(f"[OK] EMSDK: {display_path(local_emsdk, root_dir)}")
        print(f"[OK] emcc : {detail}")
        return runtime_env, True
    print("[!!] emsdk was installed, but emcc is still unavailable.")
    return env, False


def run_build_verification(root_dir: Path, env: dict[str, str]) -> bool:
    log_header("Build Verification")
    make_cmd = find_command("make.exe", env) or find_command("make", env) or "make"
    command = [make_cmd, "all", "APP=HelloCustomWidgets", "APP_SUB=input/xy_pad", "PORT=pc"]
    print("Running: " + " ".join(command))
    result = run(command, cwd=root_dir, env=env)
    if result.returncode == 0:
        print("[OK] Build verification passed.")
        return True
    print("[!!] Build verification failed.")
    return False


def print_summary(
    root_dir: Path,
    venv_dir: Path,
    python_mode: str,
    env: dict[str, str],
    build_status: str,
    *,
    toolchain_skipped: bool = False,
    emsdk_skipped: bool = False,
) -> None:
    log_header("Summary")
    venv_python = venv_python_path(venv_dir)
    if python_mode == "none" and not venv_python.exists():
        venv_status = "skipped"
    else:
        venv_status = "ready" if venv_python.exists() else "missing"
    python_dep_status = "skipped" if python_mode == "none" else probe_python_dependencies(venv_python)
    make_ready = bool(find_command("make.exe", env) or find_command("make", env))
    gcc_ready = bool(find_command("gcc.exe", env) or find_command("gcc", env))
    make_status = "skipped" if toolchain_skipped else ("ready" if make_ready else "missing")
    gcc_status = "skipped" if toolchain_skipped else ("ready" if gcc_ready else "missing")
    emcc_ready, emcc_status = probe_emcc(root_dir, env)
    emsdk_present = any(candidate.exists() for _, candidate in emsdk_candidates(root_dir, env))
    emsdk_status = "skipped" if emsdk_skipped else ("present" if emsdk_present else "missing")

    def status_line(label: str, value: str) -> None:
        mark = "INFO"
        lowered = value.lower()
        if lowered.startswith(("ready", "passed", "present")):
            mark = "OK"
        elif lowered.startswith("skipped"):
            mark = "SKIP"
        elif lowered.startswith(("missing", "broken", "failed", "not run")):
            mark = "FAIL"
        print(f"[{mark:<4}] {label:<14}: {value}")

    status_line("Project root", display_path(root_dir))
    status_line("Platform", f"{sys.platform} ({os.name})")
    status_line("Host Python", f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}")
    status_line("Python mode", python_mode)
    status_line("Virtual env", venv_status)
    status_line("Python deps", python_dep_status)
    status_line("make", make_status)
    status_line("gcc", gcc_status)
    if is_windows():
        if toolchain_skipped:
            toolchain_state = "skipped"
        elif (root_dir / "tools" / "w64devkit" / "bin" / "gcc.exe").exists():
            toolchain_state = "present"
        elif make_ready and gcc_ready:
            toolchain_state = "not requested"
        else:
            toolchain_state = "missing"
        status_line("w64devkit", toolchain_state)
    status_line("EMSDK", emsdk_status)
    status_line("emcc", "skipped" if emsdk_skipped else ("ready" if emcc_ready else emcc_status))
    status_line("Build check", build_status)
    print()
    print("Common commands:")
    print("  make all APP=HelloCustomWidgets APP_SUB=input/xy_pad PORT=pc")
    print("  make ci CATEGORY=input")
    print("  python scripts/release_check.py")
    if emcc_ready:
        print("  python scripts/web/wasm_build_demos.py")
    if venv_python.exists():
        if is_windows():
            print(f"  {display_path(venv_dir, root_dir)}\\Scripts\\activate.bat")
        else:
            print(f"  source {display_path(venv_dir, root_dir)}/bin/activate")


def normalize_python_mode(args: argparse.Namespace) -> str:
    if args.mode is None:
        return args.python_mode
    return {"0": "none", "1": "basic", "2": "full", "3": "none"}[args.mode]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="EmbeddedGUI Widgets environment setup")
    parser.add_argument("--python-mode", choices=["full", "basic", "none"], default="full", help="Python dependency profile to install (default: full).")
    parser.add_argument("--mode", choices=["0", "1", "2", "3"], help=argparse.SUPPRESS)
    parser.add_argument("--venv-dir", default=DEFAULT_VENV_DIR, help="Virtual environment directory (default: .venv).")
    parser.add_argument("--skip-toolchain", action="store_true", help="Do not auto-install or validate the native build toolchain.")
    parser.add_argument("--install-toolchain", action="store_true", help="Install the Windows w64devkit toolchain and exit.")
    parser.add_argument("--skip-emsdk", action="store_true", help="Skip Emscripten validation or installation for WASM workflows.")
    parser.add_argument("--install-emsdk", action="store_true", help="Install the Emscripten SDK bundle and exit.")
    parser.add_argument("--emsdk-version", default=DEFAULT_EMSDK_VERSION, help=f"Emscripten SDK version or alias to install/activate (default: {DEFAULT_EMSDK_VERSION}).")
    parser.add_argument("--skip-build-check", action="store_true", help="Skip the final HelloCustomWidgets build verification step.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root_dir = project_root()
    venv_dir = (root_dir / args.venv_dir).resolve()
    python_mode = normalize_python_mode(args)
    build_status = "not run"
    summary_env = os.environ.copy()

    os.chdir(root_dir)

    if args.install_toolchain and args.install_emsdk:
        print("[!!] Use only one install-only flag at a time.")
        return 1

    if args.install_toolchain:
        if not is_windows():
            print("[!!] --install-toolchain is only supported on Windows.")
            return 1
        summary_env, ready = ensure_windows_toolchain(root_dir, auto_install=True)
        print_summary(root_dir, venv_dir, python_mode, summary_env, "skipped (--install-toolchain)", toolchain_skipped=True, emsdk_skipped=True)
        return 0 if ready else 1

    if args.install_emsdk:
        summary_env, ready = ensure_emsdk(root_dir, os.environ.copy(), auto_install=is_windows(), version=args.emsdk_version)
        print_summary(root_dir, venv_dir, python_mode, summary_env, "skipped (--install-emsdk)", toolchain_skipped=True)
        return 0 if ready else 1

    if python_mode != "none":
        if not install_python_environment(root_dir, venv_dir):
            print_summary(root_dir, venv_dir, python_mode, summary_env, "skipped (python setup failed)")
            return 1
    else:
        print("Skipping Python dependency installation.")

    if args.skip_toolchain:
        toolchain_env = summary_env.copy()
        toolchain_ready = True
        print("Skipping toolchain setup by request.")
    elif is_windows():
        toolchain_env, toolchain_ready = ensure_windows_toolchain(root_dir, auto_install=True)
    else:
        toolchain_env, toolchain_ready = check_posix_toolchain()

    if args.skip_emsdk:
        emsdk_ready = True
        print("Skipping Emscripten setup by request.")
    else:
        toolchain_env, emsdk_ready = ensure_emsdk(root_dir, toolchain_env, auto_install=is_windows(), version=args.emsdk_version)

    if not toolchain_ready:
        build_status = "skipped (toolchain not ready)"
    elif args.skip_build_check:
        build_status = "skipped by request"
    elif run_build_verification(root_dir, toolchain_env):
        build_status = "passed"
    else:
        build_status = "failed"

    print_summary(
        root_dir,
        venv_dir,
        python_mode,
        toolchain_env,
        build_status,
        toolchain_skipped=args.skip_toolchain,
        emsdk_skipped=args.skip_emsdk,
    )
    if not args.skip_toolchain and not toolchain_ready:
        return 1
    if not args.skip_emsdk and not emsdk_ready:
        return 1
    return 0 if build_status != "failed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
