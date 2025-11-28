# Project Statistics Collector

**Script:** `collect_project_stats.py`  
**Purpose:** Collect and analyze code statistics for the Z Monitor project

## Overview

The Project Statistics Collector analyzes C++ source files and generates comprehensive statistics about the codebase, including lines of code, file counts, class counts, function counts, and various averages.

## Features

The script collects the following statistics:

- **Total Lines of Code:** Total number of lines across all analyzed files
- **Total Files:** Number of header (.h) and source (.cpp) files
- **Total Classes:** Number of class, struct, and enum class declarations
- **Total Functions:** Number of function and method definitions
- **Averages:**
  - Lines per file
  - Lines per class
  - Lines per function
  - Classes per file (for files that contain classes)
  - Functions per file (for files that contain functions)

## Installation

No installation required. The script uses only Python 3 standard library modules:
- `os`
- `re`
- `json`
- `argparse`
- `pathlib`
- `collections`

## Usage

### Basic Usage

Analyze the z-monitor source code directory:

```bash
cd project-dashboard
python3 scripts/collect_project_stats.py --root z-monitor/src
```

### Command-Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--root` | Root directory to analyze | `z-monitor/src` |
| `--output` | Output format: `text`, `json`, or `markdown` | `text` |
| `--output-file` | Write output to file instead of stdout | (stdout) |
| `--include-tests` | Include test files in statistics | (excluded) |

### Output Formats

#### Text Output (Default)

Human-readable text format printed to stdout:

```
============================================================
Z Monitor Project Statistics
============================================================

Total Lines of Code: 9,630
Total Files: 56
  - Header files (.h): 39
  - Source files (.cpp): 17
Total Classes: 13
Total Functions: 217

Averages:
  - Lines per file: 172.0
  - Lines per class: 740.8
  - Lines per function: 44.4
  - Classes per file (files with classes): 1.2
  - Functions per file (files with functions): 8.0

============================================================
```

#### JSON Output

Machine-readable JSON format:

```bash
python3 scripts/collect_project_stats.py --root z-monitor/src --output json
```

Example output:
```json
{
  "total_lines": 9630,
  "total_files": 56,
  "total_classes": 13,
  "total_functions": 217,
  "files_by_type": {
    ".cpp": 17,
    ".h": 39
  },
  "avg_lines_per_file": 172.0,
  "avg_lines_per_class": 740.8,
  "avg_lines_per_function": 44.4,
  ...
}
```

#### Markdown Output

Markdown format suitable for documentation:

```bash
python3 scripts/collect_project_stats.py --root z-monitor/src --output markdown
```

Example output:
```markdown
# Z Monitor Project Statistics

## Overview

- **Total Lines of Code:** 9,630
- **Total Files:** 56
  - Header files (.h): 39
  - Source files (.cpp): 17
- **Total Classes:** 13
- **Total Functions:** 217

## Averages

- **Lines per file:** 172.0
- **Lines per class:** 740.8
- **Lines per function:** 44.4
...
```

### Writing to File

Save statistics to a file:

```bash
python3 scripts/collect_project_stats.py --root z-monitor/src --output markdown --output-file doc/z-monitor/PROJECT_STATS.md
```

### Including Test Files

By default, test files are excluded. To include them:

```bash
python3 scripts/collect_project_stats.py --root z-monitor --include-tests
```

## Examples

### Example 1: Analyze Source Code Only

```bash
cd project-dashboard
python3 scripts/collect_project_stats.py --root z-monitor/src
```

**Output:** Statistics for source code files only (excludes tests and generated files)

### Example 2: Generate Markdown Report

```bash
python3 scripts/collect_project_stats.py \
    --root z-monitor/src \
    --output markdown \
    --output-file doc/z-monitor/PROJECT_STATS.md
```

**Output:** Creates `doc/z-monitor/PROJECT_STATS.md` with formatted statistics

### Example 3: Analyze Entire Project (Including Tests)

```bash
python3 scripts/collect_project_stats.py \
    --root z-monitor \
    --include-tests
```

**Output:** Statistics for all files including test files

### Example 4: JSON Output for CI/CD

```bash
python3 scripts/collect_project_stats.py \
    --root z-monitor/src \
    --output json \
    --output-file build/stats.json
```

**Output:** JSON file that can be parsed by CI/CD pipelines

### Example 5: Analyze Specific Directory

```bash
python3 scripts/collect_project_stats.py --root z-monitor/src/infrastructure
```

**Output:** Statistics for infrastructure layer only

## What Gets Analyzed

### Included Files

- All `.cpp` files (source files)
- All `.h` files (header files)

### Excluded Files (by default)

- Files in `generated/` directories
- Test files (files with `test` in the path, unless `--include-tests` is used)

### What Counts as a Class?

The script counts:
- `class` declarations
- `struct` declarations
- `enum class` declarations

**Note:** Forward declarations are excluded (only definitions with `{` are counted).

### What Counts as a Function?

The script counts:
- Method definitions: `ReturnType Class::method(...) {`
- Standalone function definitions: `ReturnType function(...) {`
- Constructor/destructor definitions with initializer lists

**Note:** Function declarations (ending with `;`) are excluded. Only definitions (with `{`) are counted.

## Statistics Explained

### Lines of Code

Total number of lines in all analyzed files, including:
- Code lines
- Blank lines
- Comments

**Note:** For more accurate code-only statistics, the script also tracks `code_lines` (excluding comments and blank lines) in the JSON output.

### Average Calculations

- **Lines per file:** `total_lines / total_files`
- **Lines per class:** `total_lines / total_classes`
- **Lines per function:** `total_lines / total_functions`
- **Classes per file:** Average number of classes in files that contain at least one class
- **Functions per file:** Average number of functions in files that contain at least one function

## Integration with CI/CD

### GitHub Actions Example

```yaml
- name: Collect Project Statistics
  run: |
    python3 scripts/collect_project_stats.py \
      --root z-monitor/src \
      --output json \
      --output-file build/stats.json
    
    # Upload as artifact
    echo "Statistics collected"
```

### Pre-commit Hook

Add to `.pre-commit-config.yaml`:

```yaml
- repo: local
  hooks:
    - id: project-stats
      name: Update Project Statistics
      entry: python3 scripts/collect_project_stats.py --root z-monitor/src --output markdown --output-file doc/z-monitor/PROJECT_STATS.md
      language: system
      files: ^z-monitor/src/.*\.(cpp|h)$
```

## Limitations

1. **Function Detection:** The script uses pattern matching to detect functions. Complex template functions or macros may not be detected accurately.

2. **Class Detection:** Forward declarations and template specializations may affect class counts.

3. **Comment Detection:** Multi-line comments spanning multiple lines may affect line counts.

4. **Qt Macros:** Qt-specific macros (e.g., `Q_OBJECT`, `Q_PROPERTY`) are counted as code lines.

## Troubleshooting

### "Directory not found" Error

If you get a "Directory not found" error:

1. **Check the path:** Ensure the `--root` path is correct relative to `project-dashboard/`
2. **Use absolute path:** You can use an absolute path if needed
3. **Check permissions:** Ensure you have read permissions for the directory

### Low Class/Function Counts

If class or function counts seem low:

1. **Check file encoding:** Ensure files are UTF-8 encoded
2. **Check file format:** The script expects standard C++ syntax
3. **Use `--include-tests`:** Some classes/functions may be in test files

### JSON Output Issues

If JSON output is malformed:

1. **Check Python version:** Requires Python 3.6+
2. **Check file encoding:** Ensure output file can be written
3. **Check disk space:** Ensure sufficient disk space for output

## Related Tools

- **cloc:** Alternative tool for counting lines of code (supports many languages)
- **tokei:** Fast code statistics tool (Rust-based)
- **scc:** Sloc, Cloc, and Code (Go-based)

## Contributing

To improve the script:

1. **Better Detection:** Improve regex patterns for class/function detection
2. **More Statistics:** Add additional metrics (cyclomatic complexity, etc.)
3. **Language Support:** Extend to support QML, CMake, etc.
4. **Visualization:** Add chart generation capabilities

## See Also

- [Project Structure Documentation](../doc/z-monitor/architecture_and_design/27_PROJECT_STRUCTURE.md)
- [Code Organization Guidelines](../doc/z-monitor/architecture_and_design/22_CODE_ORGANIZATION.md)
- [API Documentation](../doc/z-monitor/architecture_and_design/26_API_DOCUMENTATION.md)

