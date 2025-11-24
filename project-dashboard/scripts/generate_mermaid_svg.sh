#!/usr/bin/env bash
set -euo pipefail

# Generate SVG files from Mermaid (.mmd) diagram files
# Usage: ./generate_mermaid_svg.sh [file.mmd] or ./generate_mermaid_svg.sh (processes all .mmd files in doc/)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
DOC_DIR="${PROJECT_DIR}/doc"

# Check if mermaid-cli is available
if ! command -v npx >/dev/null 2>&1; then
    echo "Error: npx is not installed. Please install Node.js and npm first." >&2
    exit 1
fi

# Function to generate SVG from a single .mmd file
generate_svg() {
    local mmd_file="$1"
    local svg_file="${mmd_file%.mmd}.svg"
    
    if [[ ! -f "${mmd_file}" ]]; then
        echo "Error: File not found: ${mmd_file}" >&2
        return 1
    fi
    
    echo "Generating SVG from ${mmd_file}..."
    
    if npx -y @mermaid-js/mermaid-cli -i "${mmd_file}" -o "${svg_file}"; then
        echo "✅ Generated: ${svg_file}"
        return 0
    else
        echo "❌ Failed to generate SVG from ${mmd_file}" >&2
        return 1
    fi
}

# Main logic
if [[ $# -eq 0 ]]; then
    # No arguments: process all .mmd files in doc/
    echo "Processing all .mmd files in ${DOC_DIR}..."
    
    if [[ ! -d "${DOC_DIR}" ]]; then
        echo "Error: Documentation directory not found: ${DOC_DIR}" >&2
        exit 1
    fi
    
    found_files=false
    failed_count=0
    
    while IFS= read -r -d '' mmd_file; do
        found_files=true
        if ! generate_svg "${mmd_file}"; then
            ((failed_count++))
        fi
    done < <(find "${DOC_DIR}" -name "*.mmd" -type f -print0)
    
    if [[ "${found_files}" == false ]]; then
        echo "No .mmd files found in ${DOC_DIR}"
        exit 0
    fi
    
    if [[ ${failed_count} -gt 0 ]]; then
        echo "Warning: ${failed_count} file(s) failed to generate SVG" >&2
        exit 1
    else
        echo "✅ All SVG files generated successfully!"
    fi
else
    # Process specific file(s) provided as arguments
    for mmd_file in "$@"; do
        # Resolve relative paths
        if [[ "${mmd_file}" != /* ]]; then
            mmd_file="${PWD}/${mmd_file}"
        fi
        
        if ! generate_svg "${mmd_file}"; then
            exit 1
        fi
    done
fi

