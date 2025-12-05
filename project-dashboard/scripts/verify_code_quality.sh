#!/usr/bin/env bash

################################################################################
# verify_code_quality.sh
#
# Comprehensive code quality verification script for Z Monitor project.
# Performs lint checking and documentation validation across multiple languages.
#
# Usage:
#   ./scripts/verify_code_quality.sh [file1 file2 ...]
#   
# If no files provided, checks all uncommitted files in git.
#
# Exit codes:
#   0 - All checks passed
#   1 - Lint or documentation issues found
#   2 - Script error (missing tools, invalid arguments)
#
# @author Z Monitor Team
# @date 2025-12-05
################################################################################

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_FILES=0
PASSED_FILES=0
FAILED_FILES=0
TOTAL_ISSUES=0

# Output formatting
print_header() {
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

################################################################################
# Get list of files to check
################################################################################
get_files_to_check() {
    if [ $# -gt 0 ]; then
        # Use provided files (one per line)
        for file in "$@"; do
            echo "$file"
        done
    else
        # Get uncommitted files from git
        if ! git rev-parse --is-inside-work-tree > /dev/null 2>&1; then
            print_error "Not inside a git repository"
            exit 2
        fi
        
        # Get modified, added, and renamed files (exclude deleted)
        git diff --name-only --diff-filter=d HEAD
        git diff --cached --name-only --diff-filter=d
        git ls-files --others --exclude-standard
    fi
}

################################################################################
# C++ Lint Check (using clang-format and clang-tidy)
################################################################################
check_cpp_lint() {
    local file="$1"
    local issues=0
    
    # Check if clang-format is available
    if command -v clang-format &> /dev/null; then
        # Check formatting (dry-run)
        if ! clang-format --dry-run --Werror "$file" 2>&1 | grep -q "no modified files to format"; then
            local diff_output=$(clang-format --dry-run "$file" 2>&1)
            if [ -n "$diff_output" ]; then
                print_error "  Formatting issues in $file"
                echo "$diff_output" | sed 's/^/    /'
                ((issues++))
            fi
        fi
    else
        print_warning "  clang-format not found, skipping C++ formatting check"
    fi
    
    # Check with clang-tidy if available
    if command -v clang-tidy &> /dev/null; then
        # Only run if compile_commands.json exists
        if [ -f "compile_commands.json" ] || [ -f "build/compile_commands.json" ]; then
            local tidy_output=$(clang-tidy "$file" -- -std=c++17 2>&1 || true)
            if echo "$tidy_output" | grep -q "warning:\|error:"; then
                print_error "  clang-tidy issues in $file"
                echo "$tidy_output" | grep "warning:\|error:" | sed 's/^/    /'
                ((issues++))
            fi
        fi
    else
        print_warning "  clang-tidy not found, skipping C++ static analysis"
    fi
    
    return $issues
}

################################################################################
# QML Lint Check (using qmllint)
################################################################################
check_qml_lint() {
    local file="$1"
    local issues=0
    
    # Find qmllint (check common Qt installation paths)
    local qmllint=""
    if command -v qmllint &> /dev/null; then
        qmllint="qmllint"
    elif [ -f "$HOME/Qt/6.9.2/macos/bin/qmllint" ]; then
        qmllint="$HOME/Qt/6.9.2/macos/bin/qmllint"
    elif [ -f "/usr/local/Qt-6.9.2/bin/qmllint" ]; then
        qmllint="/usr/local/Qt-6.9.2/bin/qmllint"
    fi
    
    if [ -n "$qmllint" ]; then
        local lint_output=$("$qmllint" "$file" 2>&1 || true)
        if echo "$lint_output" | grep -q "Warning:\|Error:"; then
            print_error "  QML lint issues in $file"
            echo "$lint_output" | sed 's/^/    /'
            ((issues++))
        fi
    else
        print_warning "  qmllint not found, skipping QML lint check"
    fi
    
    # Check basic formatting (indentation consistency)
    if grep -q $'\t' "$file"; then
        print_error "  Tab characters found in $file (use spaces)"
        ((issues++))
    fi
    
    # Check for trailing whitespace
    if grep -q " $" "$file"; then
        print_error "  Trailing whitespace found in $file"
        ((issues++))
    fi
    
    return $issues
}

################################################################################
# CMake Lint Check (basic formatting)
################################################################################
check_cmake_lint() {
    local file="$1"
    local issues=0
    
    # Check for tab characters
    if grep -q $'\t' "$file"; then
        print_error "  Tab characters found in $file (use spaces)"
        ((issues++))
    fi
    
    # Check for trailing whitespace
    if grep -q " $" "$file"; then
        print_error "  Trailing whitespace found in $file"
        ((issues++))
    fi
    
    # Check indentation consistency (should be 4 spaces)
    if grep -qE "^  [^ ]" "$file"; then
        print_warning "  Possible inconsistent indentation in $file (expected 4 spaces)"
        ((issues++))
    fi
    
    return $issues
}

################################################################################
# Markdown Lint Check (basic formatting)
################################################################################
check_markdown_lint() {
    local file="$1"
    local issues=0
    
    # Check for trailing whitespace
    if grep -q " $" "$file"; then
        print_error "  Trailing whitespace found in $file"
        ((issues++))
    fi
    
    # Check if markdownlint is available
    if command -v markdownlint &> /dev/null; then
        local lint_output=$(markdownlint "$file" 2>&1 || true)
        if [ -n "$lint_output" ]; then
            print_error "  Markdown lint issues in $file"
            echo "$lint_output" | sed 's/^/    /'
            ((issues++))
        fi
    fi
    
    return $issues
}

################################################################################
# Shell Script Lint Check (using shellcheck)
################################################################################
check_shell_lint() {
    local file="$1"
    local issues=0
    
    if command -v shellcheck &> /dev/null; then
        local lint_output=$(shellcheck "$file" 2>&1 || true)
        if [ -n "$lint_output" ]; then
            print_error "  Shell script issues in $file"
            echo "$lint_output" | sed 's/^/    /'
            ((issues++))
        fi
    else
        print_warning "  shellcheck not found, skipping shell script check"
    fi
    
    return $issues
}

################################################################################
# Doxygen Documentation Check (C++ and QML)
################################################################################
check_documentation() {
    local file="$1"
    local issues=0
    
    case "$file" in
        *.h|*.cpp)
            # Check for class documentation
            if grep -qE "^(class|struct)\s+\w+" "$file"; then
                local classes=$(grep -oE "^(class|struct)\s+\w+" "$file" | awk '{print $2}')
                for class in $classes; do
                    # Look for Doxygen comment before class
                    if ! grep -B5 "^class $class" "$file" | grep -qE "/\*\*|///"; then
                        print_error "  Missing Doxygen comment for class '$class' in $file"
                        ((issues++))
                    fi
                done
            fi
            
            # Check for public function documentation (excluding constructors/destructors)
            if grep -qE "^\s*(public|protected):" "$file"; then
                # Find public methods
                local in_public=false
                while IFS= read -r line; do
                    if [[ "$line" =~ ^[[:space:]]*(public|protected): ]]; then
                        in_public=true
                    elif [[ "$line" =~ ^[[:space:]]*private: ]]; then
                        in_public=false
                    elif $in_public && [[ "$line" =~ [[:space:]]([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*\( ]]; then
                        local func_name="${BASH_REMATCH[1]}"
                        # Skip constructors, destructors, Qt slots/signals
                        if [[ ! "$func_name" =~ ^(~|Q_|signals|slots)$ ]]; then
                            # Simple heuristic: check if there's a comment within 3 lines before
                            # (more sophisticated check would use actual parsing)
                            print_warning "  Possible missing Doxygen comment for method '$func_name' in $file (verify manually)"
                        fi
                    fi
                done < "$file"
            fi
            ;;
            
        *.qml)
            # Check for property documentation
            if grep -qE "^\s*property\s+" "$file"; then
                local prop_count=$(grep -cE "^\s*property\s+" "$file")
                local doc_count=$(grep -cE "^\s*/\*\*.*@property" "$file")
                if [ "$doc_count" -lt "$prop_count" ]; then
                    print_warning "  Possible missing @property documentation in $file ($doc_count documented / $prop_count total)"
                    ((issues++))
                fi
            fi
            
            # Check for function documentation
            if grep -qE "^\s*function\s+\w+" "$file"; then
                local funcs=$(grep -oE "^\s*function\s+\w+" "$file" | awk '{print $2}')
                for func in $funcs; do
                    if ! grep -B3 "function $func" "$file" | grep -qE "/\*\*|///"; then
                        print_warning "  Possible missing documentation for function '$func' in $file"
                        ((issues++))
                    fi
                done
            fi
            ;;
    esac
    
    return $issues
}

################################################################################
# Process single file
################################################################################
process_file() {
    local file="$1"
    local file_issues=0
    
    # Skip if file doesn't exist
    if [ ! -f "$file" ]; then
        print_warning "File not found: $file"
        return 0
    fi
    
    # Determine file type and run appropriate checks
    case "$file" in
        *.h|*.cpp|*.hpp|*.cc|*.cxx)
            print_info "Checking C++ file: $file"
            check_cpp_lint "$file" || ((file_issues+=$?))
            check_documentation "$file" || ((file_issues+=$?))
            ;;
            
        *.qml)
            print_info "Checking QML file: $file"
            check_qml_lint "$file" || ((file_issues+=$?))
            check_documentation "$file" || ((file_issues+=$?))
            ;;
            
        CMakeLists.txt|*.cmake)
            print_info "Checking CMake file: $file"
            check_cmake_lint "$file" || ((file_issues+=$?))
            ;;
            
        *.md)
            print_info "Checking Markdown file: $file"
            check_markdown_lint "$file" || ((file_issues+=$?))
            ;;
            
        *.sh)
            print_info "Checking Shell script: $file"
            check_shell_lint "$file" || ((file_issues+=$?))
            ;;
            
        *)
            print_info "Skipping unsupported file type: $file"
            return 0
            ;;
    esac
    
    ((TOTAL_FILES++))
    ((TOTAL_ISSUES+=file_issues))
    
    if [ $file_issues -eq 0 ]; then
        print_success "  No issues found in $file"
        ((PASSED_FILES++))
    else
        print_error "  Found $file_issues issue(s) in $file"
        ((FAILED_FILES++))
    fi
    
    echo ""
    return $file_issues
}

################################################################################
# Main execution
################################################################################
main() {
    print_header "Code Quality Verification"
    echo ""
    
    # Get files to check
    local files=$(get_files_to_check "$@")
    
    if [ -z "$files" ]; then
        print_info "No files to check"
        exit 0
    fi
    
    # Process each file
    while IFS= read -r file; do
        [ -n "$file" ] && process_file "$file"
    done <<< "$files"
    
    # Print summary
    print_header "Summary"
    echo ""
    echo "  Total files checked: $TOTAL_FILES"
    echo "  Passed: $PASSED_FILES"
    echo "  Failed: $FAILED_FILES"
    echo "  Total issues: $TOTAL_ISSUES"
    echo ""
    
    if [ $FAILED_FILES -eq 0 ]; then
        print_success "All checks passed! ✨"
        exit 0
    else
        print_error "Found issues in $FAILED_FILES file(s)"
        exit 1
    fi
}

# Run main function
main "$@"
