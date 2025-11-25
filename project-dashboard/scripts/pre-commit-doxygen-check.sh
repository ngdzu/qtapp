#!/bin/bash
# Pre-commit hook to check for Doxygen comments in public APIs
# This is a lightweight check that only verifies comments exist, doesn't generate docs
# Exit code 0 = pass, 1 = fail (but this hook is configured to warn only)

set -e

# Colors for output
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Check if Doxygen is installed (optional - hook can run without it)
if ! command -v doxygen &> /dev/null; then
    echo -e "${YELLOW}⚠️  Doxygen not installed. Skipping documentation check.${NC}"
    echo "   Install Doxygen to enable documentation validation: sudo apt-get install doxygen"
    exit 0  # Don't fail if Doxygen not installed
fi

# Get list of changed C++ files
CHANGED_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h)$' || true)

if [ -z "$CHANGED_FILES" ]; then
    echo -e "${GREEN}✓ No C++ files changed.${NC}"
    exit 0
fi

echo "Checking Doxygen comments in changed files..."

# Simple check: look for public classes/methods without Doxygen comments
MISSING_DOCS=0

for file in $CHANGED_FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi
    
    # Check for public class declarations without preceding /** comment
    # This is a simple heuristic - may have false positives
    if grep -q "^class.*public" "$file" 2>/dev/null || grep -q "^class [A-Z]" "$file" 2>/dev/null; then
        # Check if there's a Doxygen comment before the class
        # This is a basic check - not perfect but catches obvious cases
        if ! grep -B5 "^class" "$file" | grep -q "/\*\*"; then
            echo -e "${YELLOW}⚠️  $file: Public class may be missing Doxygen comment${NC}"
            MISSING_DOCS=1
        fi
    fi
done

if [ $MISSING_DOCS -eq 1 ]; then
    echo -e "${YELLOW}⚠️  Some files may be missing Doxygen comments.${NC}"
    echo "   See .cursor/rules/api_documentation.mdc for documentation requirements"
    echo "   This is a warning only - commit will proceed"
    exit 0  # Don't fail - just warn
else
    echo -e "${GREEN}✓ Doxygen comment check passed${NC}"
    exit 0
fi

