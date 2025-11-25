# Doxygen Documentation Workflow

## Overview

API documentation is automatically generated from source code comments using Doxygen. This document describes the available workflows for documentation generation and validation.

## Workflows

### 1. Local Generation (Manual)

Generate documentation locally:

```bash
cd project-dashboard

# Using CMake (recommended)
cmake --build build --target docs

# Or directly with Doxygen
doxygen Doxyfile
```

Documentation will be available in `docs/api/html/`.

### 2. Pre-commit Hook (Optional)

A lightweight pre-commit hook checks for Doxygen comments:

**Setup:**
```bash
# Install pre-commit
pip install pre-commit

# Install hooks
cd project-dashboard
pre-commit install

# Run manually
pre-commit run --all-files
```

**Behavior:**
- **Warns** if public APIs may be missing Doxygen comments
- **Does not fail** the commit (warning only)
- **Skips** if Doxygen is not installed

**Skip hook:**
```bash
git commit --no-verify
```

**Note:** The pre-commit hook is optional and configured to warn only. It does not block commits to avoid developer friction.

### 3. GitHub Actions (Automatic)

A GitHub Actions workflow automatically generates documentation:

**Triggers:**
- **Nightly:** Runs at 2 AM UTC daily
- **On Push:** Runs when code changes are pushed to main/master
- **On PR:** Runs on pull requests (can be disabled if too slow)
- **Manual:** Can be triggered manually via workflow_dispatch

**Features:**
- Generates complete API documentation
- Checks documentation coverage
- Uploads documentation as artifacts
- Comments on PRs with documentation status
- Fails if too many undocumented items (>10 threshold)

**View workflow:**
- File: `.github/workflows/doxygen-docs.yml`
- Status: Check GitHub Actions tab

**Disable PR checks:**
Edit `.github/workflows/doxygen-docs.yml` and remove or comment out the `pull_request` trigger.

## Configuration

- **Doxyfile:** `project-dashboard/Doxyfile`
- **Output:** `project-dashboard/docs/api/html/`
- **Workflow:** `.github/workflows/doxygen-docs.yml`
- **Pre-commit:** `.pre-commit-config.yaml`

## Documentation Requirements

See `.cursor/rules/api_documentation.mdc` for complete documentation requirements:
- All public classes must have Doxygen comments
- All public methods must be documented
- All parameters and return values must be documented
- Examples should be provided for complex APIs

## Troubleshooting

### Pre-commit hook not running

```bash
# Reinstall hooks
pre-commit uninstall
pre-commit install
```

### Doxygen not found

```bash
# Ubuntu/Debian
sudo apt-get install doxygen graphviz

# macOS
brew install doxygen graphviz
```

### Documentation generation fails

1. Check Doxyfile configuration
2. Verify Doxygen is installed: `doxygen --version`
3. Check for syntax errors in source code comments
4. Review Doxygen warnings: `docs/api/doxygen_warnings.log`

