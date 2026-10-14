#!/usr/bin/env bash
# One-time setup of the GBA homebrew toolchain (devkitARM + libtonc) and the
# mGBA emulator, on Ubuntu/Debian. Uses sudo internally where needed - run
# it as your normal user (not with a leading `sudo`):
#
#   bash setup_toolchain.sh
#
set -euo pipefail

# Figure out whose ~/.bashrc to edit, in case this was invoked as
# `sudo bash setup_toolchain.sh` (which runs as root).
if [ -n "${SUDO_USER:-}" ] && [ "$SUDO_USER" != "root" ]; then
  TARGET_HOME=$(getent passwd "$SUDO_USER" | cut -d: -f6)
else
  TARGET_HOME="$HOME"
fi

echo "== Installing devkitPro pacman (package manager for devkitARM) =="
TMP_SCRIPT=$(mktemp)
# devkitPro's download host filters on this exact User-Agent string.
wget -U "dkp-apt" -O "$TMP_SCRIPT" https://apt.devkitpro.org/install-devkitpro-pacman
chmod +x "$TMP_SCRIPT"
sudo "$TMP_SCRIPT"
rm -f "$TMP_SCRIPT"

echo "== Syncing devkitPro package database =="
sudo dkp-pacman -Sy --noconfirm

echo "== Installing devkitARM, libgba, libtonc, GBA dev tools, and env setup =="
sudo dkp-pacman -S --noconfirm gba-dev devkit-env

echo "== Installing mGBA emulator =="
sudo apt install -y mgba-qt

echo "== Setting up environment variables ($TARGET_HOME/.bashrc) =="
PROFILE_SNIPPET='
# devkitPro / GBA dev (added by GBA Sudoku setup_toolchain.sh)
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export LIBTONC=$DEVKITPRO/libtonc
export PATH=$DEVKITARM/bin:$PATH
'
if ! grep -q "devkitPro / GBA dev" "$TARGET_HOME/.bashrc" 2>/dev/null; then
  echo "$PROFILE_SNIPPET" >> "$TARGET_HOME/.bashrc"
  echo "Added devkitPro environment variables to $TARGET_HOME/.bashrc"
else
  echo "$TARGET_HOME/.bashrc already has the devkitPro environment block, skipping"
fi

echo ""
echo "Done. The devkit-env package also sets these up on next login."
echo "Open a NEW terminal (or 'source ~/.bashrc') so DEVKITARM etc. are set,"
echo "then build the game with: make"
