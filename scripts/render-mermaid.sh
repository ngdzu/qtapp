#!/usr/bin/env bash
################################################################################
# render-mermaid.sh
#
# Renders Mermaid diagram (.mmd) to SVG
#
# Usage:
#   ./scripts/render-mermaid.sh <input.mmd> [output.svg]
#   ./scripts/render-mermaid.sh --all  # Render all .mmd files
#
# Requirements:
#   - mmdc (Mermaid CLI): npm install -g @mermaid-js/mermaid-cli
#
# Examples:
#   ./scripts/render-mermaid.sh doc/components/diagrams/DOC-COMP-020_diagram.mmd
#   ./scripts/render-mermaid.sh --all
################################################################################

set -e

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

# Check if mmdc is installed
if ! command -v mmdc &> /dev/null; then
    echo -e "${RED}Error: mmdc (Mermaid CLI) is not installed${NC}"
    echo "Install with: npm install -g @mermaid-js/mermaid-cli"
    exit 1
fi

render_diagram() {
    local input_file=$1
    local output_file=${2:-${input_file%.mmd}.svg}
    
    echo -e "${BLUE}Rendering: $(basename "$input_file")${NC}"
    
    if [ ! -f "$input_file" ]; then
        echo -e "${RED}Error: Input file not found: $input_file${NC}"
        return 1
    fi
    
    # Background: default to white to avoid transparent checkerboard SVGs.
    # Override by setting MMDC_BG (e.g., MMDC_BG=transparent) or pass -b manually if you fork this script.
    MMDC_BG="${MMDC_BG:-white}"

    # Render using mmdc with explicit background
    if mmdc -i "$input_file" -o "$output_file" -t default -b "$MMDC_BG" 2>/dev/null; then
        echo -e "${GREEN}✓ Created: $(basename "$output_file")${NC}"
        return 0
    else
        echo -e "${RED}✗ Failed to render: $input_file${NC}"
        return 1
    fi
}

# Parse arguments
if [ $# -eq 0 ]; then
    echo "Usage: $0 <input.mmd> [output.svg]"
    echo "       $0 --all"
    exit 1
fi

if [ "$1" == "--all" ]; then
    echo -e "${BLUE}=== Rendering All Mermaid Diagrams ===${NC}"
    echo ""
    
    success_count=0
    fail_count=0
    
    while IFS= read -r -d '' mmd_file; do
        if render_diagram "$mmd_file"; then
            ((success_count++))
        else
            ((fail_count++))
        fi
    done < <(find "$DOC_ROOT" -name "*.mmd" -type f -print0)
    
    echo ""
    echo -e "${BLUE}=== Summary ===${NC}"
    echo -e "${GREEN}Successful: $success_count${NC}"
    if [ $fail_count -gt 0 ]; then
        echo -e "${RED}Failed: $fail_count${NC}"
        exit 1
    else
        echo -e "${GREEN}✓ All diagrams rendered successfully!${NC}"
        exit 0
    fi
else
    render_diagram "$1" "$2"
fi
