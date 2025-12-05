# Code Quality Verification Script

## Overview

`verify_code_quality.sh` is a comprehensive verification script that performs lint checking and documentation validation across multiple programming languages used in the Z Monitor project.

## Features

### Multi-Language Support

The script automatically detects file types and applies appropriate checks:

- **C++ (`.h`, `.cpp`, `.hpp`, `.cc`, `.cxx`)**
  - `clang-format` - Code formatting validation
  - `clang-tidy` - Static analysis (requires `compile_commands.json`)
  - Doxygen comment validation for classes and public methods

- **QML (`.qml`)**
  - `qmllint` - QML syntax and best practices
  - Tab character detection
  - Trailing whitespace detection
  - Property documentation validation
  - Function documentation validation

- **CMake (`CMakeLists.txt`, `*.cmake`)**
  - Tab character detection
  - Trailing whitespace detection
  - Indentation consistency checks (4-space standard)

- **Markdown (`.md`)**
  - `markdownlint` - Markdown formatting (if installed)
  - Trailing whitespace detection

- **Shell Scripts (`.sh`)**
  - `shellcheck` - Shell script static analysis (if installed)

### Documentation Validation

The script validates Doxygen-style documentation:

- **C++**: Checks for class documentation, public method documentation
- **QML**: Validates `@property` and function documentation
- **Coverage reporting**: Reports missing documentation with file/line context

### Code Quality Checks

- **Formatting**: Spacing, indentation, tab characters
- **Trailing whitespace**: Detects and reports trailing spaces
- **Best practices**: Language-specific linting rules

## Usage

### Auto-detect uncommitted files

When run without arguments, the script automatically checks all uncommitted files in the git repository:

```bash
./scripts/verify_code_quality.sh
```

This is the **recommended usage for ZTODO verification steps**.

### Check specific files

Pass file paths as arguments to check only those files:

```bash
./scripts/verify_code_quality.sh \
  src/interface/controllers/NotificationController.h \
  src/interface/controllers/NotificationController.cpp \
  resources/qml/components/NotificationToast.qml
```

### Usage in ZTODO Tasks

All ZTODO tasks must run this script as part of the **Code Quality Verification** step:

```markdown
- Verification Steps:
  1. Functional: ✅ Verified - [details]
  2. Code Quality: ✅ Verified - Run `./scripts/verify_code_quality.sh` (0 issues)
  3. Documentation: ✅ Verified - [details]
  4. Integration: ✅ Verified - [details]
  5. Tests: ✅ Verified - [details]
```

## Exit Codes

- **0**: All checks passed
- **1**: Lint or documentation issues found
- **2**: Script error (missing tools, invalid arguments, not in git repo)

## Output Format

The script provides color-coded output:

- 🟢 **Green ✓**: Check passed
- 🔴 **Red ✗**: Error or issue found
- 🟡 **Yellow ⚠**: Warning (non-critical issue)
- 🔵 **Blue ℹ**: Information

Example output:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Code Quality Verification
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ℹ Checking C++ file: src/NotificationController.h
✓   No issues found in src/NotificationController.h

ℹ Checking QML file: qml/NotificationToast.qml
✗   QML lint issues in qml/NotificationToast.qml
    Warning: Unqualified access [unqualified]
⚠   Possible missing @property documentation (0 documented / 3 total)
✗   Found 2 issue(s) in qml/NotificationToast.qml

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Summary
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  Total files checked: 2
  Passed: 1
  Failed: 1
  Total issues: 2

✗ Found issues in 1 file(s)
```

## Dependencies

### Required (Built-in)

- `bash` (version 4.0+)
- `git` (for auto-detection)
- `grep`, `sed`, `awk` (standard Unix tools)

### Optional (Enhanced Checking)

Install these tools for enhanced checking:

#### macOS (via Homebrew)

```bash
# C++ tools
brew install llvm  # Provides clang-format and clang-tidy

# Shell script linting
brew install shellcheck

# Markdown linting
npm install -g markdownlint-cli
```

#### Linux (Ubuntu/Debian)

```bash
# C++ tools
sudo apt install clang-format clang-tidy

# Shell script linting
sudo apt install shellcheck

# Markdown linting
npm install -g markdownlint-cli
```

### QML Tools

`qmllint` is provided by Qt installation. The script automatically detects Qt at:

- `$HOME/Qt/6.9.2/macos/bin/qmllint` (macOS)
- `/usr/local/Qt-6.9.2/bin/qmllint` (Linux)
- System PATH (if `qmllint` is in PATH)

## Customization

### Skipping Tools

If a tool is not installed, the script will display a warning but continue checking with available tools:

```
⚠ clang-format not found, skipping C++ formatting check
⚠ shellcheck not found, skipping shell script check
```

### Adding New Languages

To add support for a new language, modify the `process_file()` function and add a new `check_*_lint()` function following the existing patterns.

## Integration with CI/CD

Add to your CI pipeline to enforce code quality:

```yaml
# .github/workflows/code-quality.yml
- name: Verify Code Quality
  run: ./scripts/verify_code_quality.sh
```

## Troubleshooting

### "Not inside a git repository"

Ensure you're running the script from within the git repository:

```bash
cd /path/to/qtapp
./project-dashboard/scripts/verify_code_quality.sh
```

### "File not found" warnings

When auto-detecting files, the script may report files relative to git root. Ensure you're running from the correct directory or use absolute paths.

### False Positives

Some warnings may be false positives (e.g., QML `parent.hovered` on Button is valid). Use manual verification where needed. The script marks these as "⚠ verify manually".

## Related Documentation

- ZTODO Verification Guidelines: `.github/ztodo_verification.md`
- API Documentation Standards: `.cursor/rules/api_documentation.mdc`
- C++ Coding Guidelines: `.cursor/rules/cpp_guidelines.mdc`
- QML Coding Guidelines: `.cursor/rules/qml_guidelines.mdc`

## Support

For issues or questions about the verification script:

1. Check this README for common troubleshooting
2. Review the script source code comments
3. Consult the project's coding guidelines in `.github/instructions/`

---

**Note:** This script is part of the Z Monitor project's quality assurance workflow. All tasks in ZTODO.md must pass code quality verification before completion.
