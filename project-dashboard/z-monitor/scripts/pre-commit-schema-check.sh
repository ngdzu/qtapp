#!/bin/bash
# Pre-commit hook: Verify schema is synchronized
#
# This hook ensures that if schema/database.yaml is modified,
# the generated SchemaInfo.h and DDL files are also updated.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Check if schema YAML is in the staged changes
if git diff --cached --name-only | grep -q "^schema/database\.yaml$"; then
    echo "⚠️  schema/database.yaml modified - regenerating schema..."
    
    # Activate virtual environment if it exists
    if [ -d "$PROJECT_ROOT/.venv" ]; then
        source "$PROJECT_ROOT/.venv/bin/activate"
    fi
    
    # Regenerate schema
    cd "$PROJECT_ROOT"
    python3 scripts/generate_schema.py
    
    # Add generated files to commit
    git add src/infrastructure/persistence/generated/SchemaInfo.h
    git add schema/generated/ddl/*.sql 2>/dev/null || true
    
    echo "✅ Schema regenerated and staged"
fi

exit 0

