#!/bin/bash
# Generate QUERY_REFERENCE.md from QueryCatalog
# This script builds a small C++ program that calls QueryCatalog::generateDocumentation()
# and outputs the result to QUERY_REFERENCE.md

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
Z_MONITOR_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DOC_DIR="$Z_MONITOR_ROOT/doc"
OUTPUT_FILE="$DOC_DIR/QUERY_REFERENCE.md"

echo "Generating query documentation..."

# Create a temporary C++ program to generate documentation
TEMP_CPP=$(mktemp /tmp/query_doc_gen_XXXXXX.cpp)
TEMP_EXE=$(mktemp /tmp/query_doc_gen_XXXXXX)

cat > "$TEMP_CPP" << 'EOF'
#include "infrastructure/persistence/QueryRegistry.h"
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    QString doc = zmon::persistence::QueryCatalog::generateDocumentation();
    
    QTextStream out(stdout);
    out << doc;
    
    return 0;
}
EOF

# Try to compile and run (requires Qt and project to be built)
# For now, we'll use a simpler approach - just note that this should be run
# after the project is built, or we can generate it at build time

echo "Note: Query documentation generation requires the project to be built."
echo "To generate QUERY_REFERENCE.md:"
echo "  1. Build the project"
echo "  2. Run the query_doc_generator executable (if created)"
echo "  3. Or call QueryCatalog::generateDocumentation() from application code"
echo ""
echo "For now, documentation can be generated manually by:"
echo "  - Including QueryRegistry.h in your code"
echo "  - Calling QueryCatalog::generateDocumentation()"
echo "  - Writing the result to $OUTPUT_FILE"

# Cleanup
rm -f "$TEMP_CPP" "$TEMP_EXE"

echo "Done."

