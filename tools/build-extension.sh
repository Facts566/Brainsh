#!/usr/bin/env bash
# Build the distributable VS Code extension (.vsix) into dist/.
#
# vsce 4 requires Node 20+, so we pin vsce 3 and preload tools/polyfill.js,
# which supplies the global File object that undici (a vsce dependency) expects.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TOOLS="$ROOT/.tools"
EXT="$ROOT/brainsh-vscode"

VERSION="$(node -p "require('$EXT/package.json').version")"
OUT="$ROOT/dist/brainsh-$VERSION.vsix"

if [ ! -f "$TOOLS/node_modules/@vscode/vsce/vsce" ]; then
    echo "Installing @vscode/vsce@3 into .tools/ ..."
    mkdir -p "$TOOLS"
    npm install --prefix "$TOOLS" --no-save @vscode/vsce@3 >/dev/null
fi

mkdir -p "$ROOT/dist"
rm -f "$OUT"

cd "$EXT"
node -r "$ROOT/tools/polyfill.js" "$TOOLS/node_modules/@vscode/vsce/vsce" \
    package --out "$OUT"

echo "Built: $OUT"