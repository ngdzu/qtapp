#!/usr/bin/env bash
################################################################################
# create-doc.sh
# 
# Interactive script to create a new documentation file from template
#
# Usage:
#   ./scripts/create-doc.sh [--category CATEGORY] [--title "Title"]
#
# Options:
#   --category    Document category (ARCH, COMP, API, PROC, GUIDE, REF, TRAIN, REG)
#   --title       Document title
#   --owner       Document owner/team
#   --interactive Run in interactive mode (default if no args)
#
# Examples:
#   ./scripts/create-doc.sh --category COMP --title "Database Manager"
#   ./scripts/create-doc.sh  # Interactive mode
################################################################################

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Base paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DOC_ROOT="$PROJECT_ROOT/project-dashboard/doc"
TEMPLATE_DIR="$DOC_ROOT/templates"

# Category mappings using simple functions (bash 3.2 compatible)
get_category_name() {
    case $1 in
        ARCH) echo "Architecture" ;;
        REQ) echo "Requirements" ;;
        API) echo "API" ;;
        COMP) echo "Component" ;;
        PROC) echo "Process" ;;
        GUIDE) echo "Guideline" ;;
        REF) echo "Reference" ;;
        TRAIN) echo "Training" ;;
        REG) echo "Regulatory" ;;
        *) echo "" ;;
    esac
}

get_category_dir() {
    case $1 in
        ARCH) echo "architecture" ;;
        REQ) echo "requirements" ;;
        API) echo "api" ;;
        COMP) echo "components" ;;
        PROC) echo "processes" ;;
        GUIDE) echo "guidelines" ;;
        REF) echo "reference" ;;
        TRAIN) echo "training" ;;
        REG) echo "regulatory" ;;
        *) echo "" ;;
    esac
}

get_category_template() {
    case $1 in
        ARCH) echo "component_template.md" ;;
        REQ) echo "guideline_template.md" ;;
        API) echo "api_template.md" ;;
        COMP) echo "component_template.md" ;;
        PROC) echo "process_template.md" ;;
        GUIDE) echo "guideline_template.md" ;;
        REF) echo "guideline_template.md" ;;
        TRAIN) echo "guideline_template.md" ;;
        REG) echo "guideline_template.md" ;;
        *) echo "" ;;
    esac
}

# Parse command line arguments
INTERACTIVE=true
CATEGORY=""
TITLE=""
OWNER=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --category)
            CATEGORY="$2"
            INTERACTIVE=false
            shift 2
            ;;
        --title)
            TITLE="$2"
            INTERACTIVE=false
            shift 2
            ;;
        --owner)
            OWNER="$2"
            shift 2
            ;;
        --interactive)
            INTERACTIVE=true
            shift
            ;;
        -h|--help)
            head -n 20 "$0" | tail -n +2 | sed 's/^# //'
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Function to get next available DOC-ID number
get_next_doc_id() {
    local category=$1
    local doc_dir=$(get_category_dir "$category")
    local max_num=0
    
    # Find all DOC-{CATEGORY}-XXX patterns in the directory (bash 3.2 compatible)
    if [ -d "$DOC_ROOT/$doc_dir" ]; then
        # Use find instead of ** globstar to support bash 3.2
        while IFS= read -r file; do
            # Extract DOC-ID from metadata
            local doc_id=$(grep -m 1 "^doc_id:" "$file" 2>/dev/null | sed 's/doc_id: *//' | tr -d ' ')
            if [[ $doc_id =~ DOC-${category}-([0-9]+) ]]; then
                local num=${BASH_REMATCH[1]}
                num=$((10#$num)) # Convert from string to decimal
                if [ $num -gt $max_num ]; then
                    max_num=$num
                fi
            fi
        done < <(find "$DOC_ROOT/$doc_dir" -type f -name '*.md' \( ! -name '_index.md' -a ! -name '00_INDEX.md' \))
    fi
    
    # Return next number (zero-padded to 3 digits)
    printf "%03d" $((max_num + 1))
}

# Function to prompt for category
prompt_category() {
    echo -e "${BLUE}Select document category:${NC}"
    echo "  1) ARCH - Architecture & System Design"
    echo "  2) REQ  - Requirements & Specifications"
    echo "  3) API  - API Documentation & Interface Contracts"
    echo "  4) COMP - Component/Module Specifications"
    echo "  5) PROC - Processes & Workflows"
    echo "  6) GUIDE - Guidelines & Best Practices"
    echo "  7) REF  - Reference Materials & Lookups"
    echo "  8) TRAIN - Training & Onboarding"
    echo "  9) REG  - Regulatory & Compliance"
    echo ""
    read -p "Enter choice (1-9): " choice
    
    case $choice in
        1) echo "ARCH" ;;
        2) echo "REQ" ;;
        3) echo "API" ;;
        4) echo "COMP" ;;
        5) echo "PROC" ;;
        6) echo "GUIDE" ;;
        7) echo "REF" ;;
        8) echo "TRAIN" ;;
        9) echo "REG" ;;
        *) echo -e "${RED}Invalid choice${NC}"; exit 1 ;;
    esac
}

# Function to sanitize filename
sanitize_filename() {
    echo "$1" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9]/_/g' | sed 's/__*/_/g' | sed 's/^_//;s/_$//'
}

# Interactive mode
if [ "$INTERACTIVE" = true ]; then
    echo -e "${GREEN}=== Create New Documentation ===${NC}"
    echo ""
    
    # Get category
    if [ -z "$CATEGORY" ]; then
        CATEGORY=$(prompt_category)
    fi
    
    # Get title
    if [ -z "$TITLE" ]; then
        read -p "Enter document title: " TITLE
    fi
    
    # Get owner
    if [ -z "$OWNER" ]; then
        read -p "Enter document owner (team/person): " OWNER
    fi
    
    # Get subcategory
    read -p "Enter subcategory (optional, e.g., Infrastructure/Database): " SUBCATEGORY
    
    # Get tags
    read -p "Enter tags (comma-separated, e.g., database, infrastructure): " TAGS
fi

# Validate required fields
if [ -z "$CATEGORY" ] || [ -z "$TITLE" ]; then
    echo -e "${RED}Error: Category and title are required${NC}"
    exit 1
fi

# Validate category
CATEGORY_NAME=$(get_category_name "$CATEGORY")
if [ -z "$CATEGORY_NAME" ]; then
    echo -e "${RED}Error: Invalid category '$CATEGORY'${NC}"
    echo "Valid categories: ARCH REQ API COMP PROC GUIDE REF TRAIN REG"
    exit 1
fi

# Get next DOC-ID
DOC_NUMBER=$(get_next_doc_id "$CATEGORY")
DOC_ID="DOC-${CATEGORY}-${DOC_NUMBER}"

# Determine output directory
CATEGORY_DIR=$(get_category_dir "$CATEGORY")
DOC_DIR="$DOC_ROOT/$CATEGORY_DIR"
FILENAME="${DOC_ID}_$(sanitize_filename "$TITLE").md"
OUTPUT_PATH="$DOC_DIR/$FILENAME"
DIAGRAM_DIR="$DOC_DIR/diagrams"

# Create directories if they don't exist
mkdir -p "$DOC_DIR"
mkdir -p "$DIAGRAM_DIR"

# Get template
TEMPLATE_NAME=$(get_category_template "$CATEGORY")
TEMPLATE_FILE="$TEMPLATE_DIR/$TEMPLATE_NAME"
if [ ! -f "$TEMPLATE_FILE" ]; then
    echo -e "${RED}Error: Template not found: $TEMPLATE_FILE${NC}"
    exit 1
fi

# Get current date
CURRENT_DATE=$(date +%Y-%m-%d)

# Create document from template
echo -e "${BLUE}Creating document: $DOC_ID${NC}"
echo "  Title: $TITLE"
echo "  Category: $CATEGORY_NAME"
echo "  Path: $OUTPUT_PATH"
echo ""

# Copy template and replace placeholders
cp "$TEMPLATE_FILE" "$OUTPUT_PATH"

# Replace metadata placeholders
sed -i '' "s/doc_id: DOC-.*-XXX/doc_id: $DOC_ID/" "$OUTPUT_PATH"
sed -i '' "s/title: {.*}/title: $TITLE/" "$OUTPUT_PATH"
sed -i '' "s/category: .*/category: $CATEGORY_NAME/" "$OUTPUT_PATH"

if [ -n "$OWNER" ]; then
    sed -i '' "s/owner: {Team Name}/owner: $OWNER/" "$OUTPUT_PATH"
fi

if [ -n "$SUBCATEGORY" ]; then
    sed -i '' "s|subcategory: {.*}|subcategory: $SUBCATEGORY|" "$OUTPUT_PATH"
fi

if [ -n "$TAGS" ]; then
    # Convert comma-separated tags to YAML list
    TAG_LIST=$(echo "$TAGS" | sed 's/,/\n  - /g' | sed 's/^ *//')
    sed -i '' "s/tags:.*/tags:\n  - $TAG_LIST/" "$OUTPUT_PATH"
fi

# Update dates
sed -i '' "s/YYYY-MM-DD/$CURRENT_DATE/g" "$OUTPUT_PATH"

# Replace content placeholders
sed -i '' "s/{DOC-ID}/$DOC_ID/g" "$OUTPUT_PATH"
sed -i '' "s/{Component Name}/$TITLE/g" "$OUTPUT_PATH"
sed -i '' "s/{Interface Name}/$TITLE/g" "$OUTPUT_PATH"
sed -i '' "s/{Process Name}/$TITLE/g" "$OUTPUT_PATH"
sed -i '' "s/{Guideline Topic}/$TITLE/g" "$OUTPUT_PATH"

echo -e "${GREEN}✓ Document created successfully!${NC}"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "  1. Edit the document: $OUTPUT_PATH"
echo "  2. Create diagrams in: $DIAGRAM_DIR/"
echo "  3. Fill in all {placeholder} values"
echo "  4. Update metadata (related_docs, related_tasks, etc.)"
echo "  5. Run validation: ./scripts/validate-docs.sh"
echo "  6. Submit for review when ready"
echo ""
echo -e "${BLUE}Document ID: $DOC_ID${NC}"
