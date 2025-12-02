#!/usr/bin/env bash
################################################################################
# validate-docs.sh
#
# Validates documentation for completeness and correctness
#
# Checks:
#   - All docs have metadata
#   - DOC-IDs are unique
#   - Cross-references resolve
#   - Diagrams exist
#   - No orphaned files
#   - Metadata schema is valid
#
# Usage:
#   ./scripts/validate-docs.sh [--fix] [--verbose]
#
# Options:
#   --fix      Attempt to auto-fix issues
#   --verbose  Show detailed output
################################################################################

# Note: set -e disabled to allow accumulating errors/warnings

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Config
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DOC_ROOT="$PROJECT_ROOT/project-dashboard/doc"

FIX_MODE=false
VERBOSE=false
ERROR_COUNT=0
WARNING_COUNT=0

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --fix)
            FIX_MODE=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            head -n 23 "$0" | tail -n +2 | sed 's/^# //'
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

log_error() {
    echo -e "${RED}✗ ERROR: $1${NC}"
    ((ERROR_COUNT++))
}

log_warning() {
    echo -e "${YELLOW}⚠ WARNING: $1${NC}"
    ((WARNING_COUNT++))
}

log_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

log_info() {
    if [ "$VERBOSE" = true ]; then
        echo -e "${BLUE}ℹ $1${NC}"
    fi
}

# Check 1: All markdown files have metadata
check_metadata() {
    echo ""
    echo "Checking metadata..."
    
    local missing_metadata=()
    
    while IFS= read -r -d '' file; do
        # Skip index files, master index, templates, and foundation docs
        if [[ $(basename "$file") == _* ]] || \
           [[ $(basename "$file") == "00_INDEX.md" ]] || \
           [[ "$file" == */templates/* ]] || \
           [[ "$file" == */foundation/* ]] || \
           [[ "$file" == */z-monitor/* ]]; then
            continue
        fi
        
        # Check for YAML frontmatter
        if ! grep -q '^---$' "$file" 2>/dev/null; then
            missing_metadata+=("$file")
            log_error "Missing metadata: $file"
        else
            # Check for required fields
            if ! grep -q '^doc_id:' "$file"; then
                log_error "Missing doc_id in: $file"
            fi
            if ! grep -q '^title:' "$file"; then
                log_error "Missing title in: $file"
            fi
            if ! grep -q '^version:' "$file"; then
                log_error "Missing version in: $file"
            fi
            if ! grep -q '^status:' "$file"; then
                log_error "Missing status in: $file"
            fi
        fi
    done < <(find "$DOC_ROOT" -name "*.md" -type f -print0)
    
    if [ ${#missing_metadata[@]} -eq 0 ]; then
        log_success "All documents have metadata"
    fi
}

# Check 2: DOC-IDs are unique
check_unique_doc_ids() {
    echo ""
    echo "Checking DOC-ID uniqueness..."
    
    local doc_ids=()
    local duplicates=()
    
    while IFS= read -r -d '' file; do
        if [[ $(basename "$file") == _* ]] || [[ "$file" == */templates/* ]]; then
            continue
        fi
        
        local doc_id=$(grep -m 1 '^doc_id:' "$file" 2>/dev/null | sed 's/doc_id: *//' | tr -d ' ')
        if [ -n "$doc_id" ]; then
            if [[ " ${doc_ids[@]} " =~ " ${doc_id} " ]]; then
                duplicates+=("$doc_id in $file")
                log_error "Duplicate DOC-ID: $doc_id in $file"
            else
                doc_ids+=("$doc_id")
            fi
        fi
    done < <(find "$DOC_ROOT" -name "*.md" -type f -print0)
    
    if [ ${#duplicates[@]} -eq 0 ]; then
        log_success "All DOC-IDs are unique (${#doc_ids[@]} total)"
    fi
}

# Check 3: Cross-references resolve
check_cross_references() {
    echo ""
    echo "Checking cross-references..."
    
    local broken_refs=0
    
    while IFS= read -r -d '' file; do
        # Skip index files, templates, foundation, and z-monitor docs
        if [[ $(basename "$file") == _* ]] || \
           [[ "$file" == */templates/* ]] || \
           [[ "$file" == */foundation/* ]] || \
           [[ "$file" == */z-monitor/* ]]; then
            continue
        fi
        
        # Extract markdown links (skip code blocks)
        local in_code_block=false
        while IFS= read -r line; do
            # Track code fences
            if [[ $line =~ ^\`\`\` ]]; then
                if [ "$in_code_block" = true ]; then
                    in_code_block=false
                else
                    in_code_block=true
                fi
                continue
            fi
            
            # Skip lines in code blocks
            if [ "$in_code_block" = true ]; then
                continue
            fi
            
            # Match [text](path) patterns
            if [[ $line =~ \[.*\]\(([^\)]+)\) ]]; then
                local link="${BASH_REMATCH[1]}"
                
                # Skip external links, anchors, template placeholders (XXX), and diagram references
                if [[ $link =~ ^https?:// ]] || [[ $link =~ ^\# ]] || [[ $link =~ XXX ]] || [[ $link =~ diagrams/ ]]; then
                    continue
                fi
                
                # Resolve relative path
                local file_dir=$(dirname "$file")
                local target_path="$file_dir/$link"
                
                # Check if target exists
                if [ ! -f "$target_path" ] && [ ! -d "$target_path" ]; then
                    log_warning "Broken link in $file: $link"
                    ((broken_refs++))
                fi
            fi
        done < "$file"
    done < <(find "$DOC_ROOT" -name "*.md" -type f -print0)
    
    if [ $broken_refs -eq 0 ]; then
        log_success "All cross-references resolve"
    else
        log_info "Cross-references check completed with $broken_refs warnings"
    fi
}

# Check 4: Diagrams exist
check_diagrams() {
    echo ""
    echo "Checking diagram files..."
    
    local missing_diagrams=0
    
    while IFS= read -r -d '' file; do
        # Skip index files, templates, foundation, and z-monitor docs
        if [[ $(basename "$file") == _* ]] || \
           [[ "$file" == */templates/* ]] || \
           [[ "$file" == */foundation/* ]] || \
           [[ "$file" == */z-monitor/* ]]; then
            continue
        fi
        
        # Check diagram_files in metadata
        local in_metadata=false
        while IFS= read -r line; do
            if [[ $line == "diagram_files:" ]]; then
                in_metadata=true
                continue
            fi
            
            if [ "$in_metadata" = true ]; then
                if [[ $line =~ ^[[:space:]]*- ]]; then
                    local diagram_file=$(echo "$line" | sed 's/^[[:space:]]*- *//' | tr -d ' ')
                    
                    # Find diagram file
                    local file_dir=$(dirname "$file")
                    local diagram_path="$file_dir/diagrams/$diagram_file"
                    
                    if [ ! -f "$diagram_path" ]; then
                        log_warning "Missing diagram: $diagram_file (referenced in $file)"
                        ((missing_diagrams++))
                    fi
                else
                    break
                fi
            fi
        done < "$file"
    done < <(find "$DOC_ROOT" -name "*.md" -type f -print0)
    
    if [ $missing_diagrams -eq 0 ]; then
        log_success "All referenced diagrams exist"
    fi
}

# Check 5: No orphaned files
check_orphaned_files() {
    echo ""
    echo "Checking for orphaned files..."
    
    local orphaned=0
    
    # Check for .mmd files without corresponding .svg
    while IFS= read -r -d '' mmd_file; do
        local svg_file="${mmd_file%.mmd}.svg"
        if [ ! -f "$svg_file" ]; then
            log_warning "Orphaned Mermaid file (no SVG): $mmd_file"
            ((orphaned++))
        fi
    done < <(find "$DOC_ROOT" -name "*.mmd" -type f -print0)
    
    # Check for .svg files without corresponding .mmd
    while IFS= read -r -d '' svg_file; do
        local mmd_file="${svg_file%.svg}.mmd"
        if [ ! -f "$mmd_file" ]; then
            log_warning "Orphaned SVG file (no Mermaid source): $svg_file"
            ((orphaned++))
        fi
    done < <(find "$DOC_ROOT" -name "*.svg" -type f -print0)
    
    if [ $orphaned -eq 0 ]; then
        log_success "No orphaned diagram files"
    fi
}

# Check 6: Status values are valid
check_status_values() {
    echo ""
    echo "Checking status values..."
    
    local invalid_status=0
    local valid_statuses=("Draft" "In Review" "Approved" "Published" "Deprecated")
    
    while IFS= read -r -d '' file; do
        # Skip index files, templates, foundation, and z-monitor docs
        if [[ $(basename "$file") == _* ]] || \
           [[ "$file" == */templates/* ]] || \
           [[ "$file" == */foundation/* ]] || \
           [[ "$file" == */z-monitor/* ]]; then
            continue
        fi
        
        local status=$(grep -m 1 '^status:' "$file" 2>/dev/null | sed 's/status: *//' | tr -d ' ')
        if [ -n "$status" ]; then
            local valid=false
            for valid_status in "${valid_statuses[@]}"; do
                if [ "$status" == "$valid_status" ]; then
                    valid=true
                    break
                fi
            done
            
            if [ "$valid" = false ]; then
                log_error "Invalid status '$status' in: $file"
                ((invalid_status++))
            fi
        fi
    done < <(find "$DOC_ROOT" -name "*.md" -type f -print0)
    
    if [ $invalid_status -eq 0 ]; then
        log_success "All status values are valid"
    fi
}

# Main execution
echo -e "${BLUE}=== Documentation Validation ===${NC}"
echo "Doc root: $DOC_ROOT"

check_metadata
check_unique_doc_ids
check_cross_references
check_diagrams
check_orphaned_files
check_status_values

# Summary
echo ""
echo -e "${BLUE}=== Validation Summary ===${NC}"
if [ $ERROR_COUNT -eq 0 ] && [ $WARNING_COUNT -eq 0 ]; then
    echo -e "${GREEN}✓ All checks passed!${NC}"
    exit 0
elif [ $ERROR_COUNT -eq 0 ]; then
    echo -e "${YELLOW}⚠ Validation passed with warnings${NC}"
    echo -e "${YELLOW}Warnings: $WARNING_COUNT${NC}"
    exit 0
else
    echo -e "${RED}✗ Validation failed${NC}"
    echo -e "${RED}Errors: $ERROR_COUNT${NC}"
    echo -e "${YELLOW}Warnings: $WARNING_COUNT${NC}"
    exit 1
fi
