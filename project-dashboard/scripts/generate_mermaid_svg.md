# Generate Mermaid SVG

Generates SVG files from Mermaid diagram files (`.mmd`).

## Prerequisites

- Node.js and npm installed
- The script will automatically install `@mermaid-js/mermaid-cli` via `npx` if needed

## Usage

### Generate SVG for all Mermaid files

Process all `.mmd` files in `project-dashboard/doc/`:

```bash
cd project-dashboard
./scripts/generate_mermaid_svg.sh
```

This will find all `.mmd` files in `project-dashboard/doc/` and generate corresponding `.svg` files in the same directory.

### Generate SVG for a specific file

Process a single file:

```bash
cd project-dashboard
./scripts/generate_mermaid_svg.sh doc/02_ARCHITECTURE.mmd
```

### Generate SVG for multiple files

Process multiple specific files:

```bash
cd project-dashboard
./scripts/generate_mermaid_svg.sh doc/02_ARCHITECTURE.mmd doc/05_STATE_MACHINES.mmd
```

## Output

- SVG files are generated in the same directory as the source `.mmd` file
- The output filename matches the input filename but with `.svg` extension
- Example: `doc/02_ARCHITECTURE.mmd` → `doc/02_ARCHITECTURE.svg`

## Error Handling

- The script will exit with an error if any file fails to generate
- When processing all files, a summary is shown at the end
- Individual file errors are reported as they occur
- Common errors:
  - Missing `npx`: Install Node.js and npm
  - Invalid Mermaid syntax: Check the `.mmd` file for syntax errors
  - File not found: Verify the file path is correct

## Examples

Generate all diagrams:

```bash
./scripts/generate_mermaid_svg.sh
```

Generate a specific diagram:

```bash
./scripts/generate_mermaid_svg.sh doc/12_THREAD_MODEL.mmd
```

## Integration

This script is automatically referenced by the `.cursor/rules/mermaid_guidelines.mdc` rule file, which ensures SVG generation after editing Mermaid files.

