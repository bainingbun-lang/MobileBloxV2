#!/usr/bin/env bash
set -euo pipefail

LUAU_DIR="${1:-app/src/main/cpp/luau}"
LUAU_COMMIT="c87565b8b0db5d5d5bfd2b8b00ad6573e5d06d9f"

if [ -f "$LUAU_DIR/CMakeLists.txt" ]; then
    echo "Luau already present at $LUAU_DIR"
    exit 0
fi

rm -rf "$LUAU_DIR"
mkdir -p "$LUAU_DIR"
git -C "$LUAU_DIR" init -q
git -C "$LUAU_DIR" remote add origin https://github.com/luau-lang/luau.git
git -C "$LUAU_DIR" fetch -q --depth 1 origin "$LUAU_COMMIT"
git -C "$LUAU_DIR" checkout -q FETCH_HEAD
echo "Fetched Luau $LUAU_COMMIT into $LUAU_DIR"
