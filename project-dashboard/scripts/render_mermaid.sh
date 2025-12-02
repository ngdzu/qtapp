#!/usr/bin/env zsh
set -euo pipefail

DIAGRAM_DIR="$(cd "$(dirname "$0")/../doc/processes/diagrams" && pwd)"

if [[ ! -d "$DIAGRAM_DIR" ]]; then
  echo "Diagrams directory not found: $DIAGRAM_DIR" >&2
  exit 1
fi

echo "Rendering Mermaid diagrams in: $DIAGRAM_DIR"

# Ensure required tool (npx) is available
if ! command -v npx >/dev/null 2>&1; then
  echo "Error: 'npx' not found. Install Node.js (which includes npm/npx)." >&2
  echo "On macOS: brew install node" >&2
  exit 127
fi

# Find all .mmd files and render to .svg
for mmd in "$DIAGRAM_DIR"/**/*.mmd(N); do
  svg="${mmd:r}.svg"
  echo "Rendering: ${mmd:t} -> ${svg:t}"
  # Use npx to run mermaid-cli without global install
  npx -y @mermaid-js/mermaid-cli@10.9.1 -i "$mmd" -o "$svg" --scale 1
done

echo "Done."
