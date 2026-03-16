#!/usr/bin/env bash
set -euo pipefail

REQUIRED_PYTHON_PKGS=(numpy pytest pytest-forked scipy)

detect_os() {
  if [[ "$(uname)" == "Darwin" ]]; then echo "macos"
  elif [[ -f /etc/debian_version ]]; then echo "debian"
  else echo "unknown"; fi
}

check_python() {
  if ! command -v python3 >/dev/null 2>&1; then
    echo "ERROR: python3 not found. Install Python 3.8+ and retry." >&2
    exit 1
  fi
  local version
  version=$(python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
  local major minor
  major=${version%%.*}; minor=${version##*.}
  if [[ $major -lt 3 || ($major -eq 3 && $minor -lt 8) ]]; then
    echo "ERROR: Python 3.8+ required (found $version)." >&2
    exit 1
  fi
  echo "Python $version found."
  if ! python3 -m venv --help >/dev/null 2>&1; then
    echo "ERROR: python3-venv module not available."
    echo "On Ubuntu/Debian: sudo apt-get install python3-venv" >&2
    exit 1
  fi
}

setup_venv() {
  local venv_dir="${GEODE_VENV:-.venv}"
  if [[ ! -f "$venv_dir/bin/python3" ]]; then
    echo "Creating venv at $venv_dir ..."
    python3 -m venv "$venv_dir"
  else
    echo "Venv already exists at $venv_dir."
  fi
  # shellcheck disable=SC1090
  source "$venv_dir/bin/activate"
  pip install --upgrade pip --quiet
  local missing=()
  for pkg in "${REQUIRED_PYTHON_PKGS[@]}"; do
    if ! python3 -c "import ${pkg//-/_}" >/dev/null 2>&1; then
      missing+=("$pkg")
    fi
  done
  if [[ ${#missing[@]} -gt 0 ]]; then
    echo "Installing missing Python packages: ${missing[*]}"
    pip install "${missing[@]}"
  else
    echo "All required Python packages present."
  fi
  echo "Venv ready. Activate with: source $venv_dir/bin/activate"
}

install_macos() {
  command -v brew >/dev/null 2>&1 || { echo "Homebrew required: https://brew.sh"; exit 1; }
  brew install cmake gmp open-mesh libpng libjpeg
}

install_debian() {
  sudo apt-get update
  sudo apt-get install -y cmake libgmp-dev libopenmesh-dev libpng-dev libjpeg-dev python3-pip python3-venv
}

OS=$(detect_os)
check_python

case "$OS" in
  macos)   install_macos ;;
  debian)  install_debian ;;
  *)       echo "Unsupported OS. Install manually: cmake, gmp, open-mesh, libpng, libjpeg, numpy, pytest, scipy"; exit 1 ;;
esac

setup_venv

echo ""
echo "Dependencies installed. Build with:"
echo "  source \${GEODE_VENV:-.venv}/bin/activate"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make -j\$(nproc 2>/dev/null || sysctl -n hw.ncpu)"
