#!/usr/bin/env python3
"""
@file validate_orm_schema.py
@brief Validates that ORM mappings use schema constants correctly.

This script validates that all QxOrm mapping files use Schema:: constants
instead of hardcoded table/column names. It parses ORM mapping files and
checks against the schema YAML definition.

Usage:
    python3 scripts/validate_orm_schema.py

Exit codes:
    0 - All mappings valid
    1 - Validation errors found
"""

import yaml
import re
import sys
from pathlib import Path
from typing import Set, List, Tuple

def load_schema(yaml_path: Path) -> dict:
    """Load schema YAML file."""
    with open(yaml_path, 'r') as f:
        return yaml.safe_load(f)

def extract_schema_constants(schema: dict) -> Tuple[Set[str], Set[str]]:
    """Extract expected table and column constants from schema."""
    tables = set()
    columns = {}
    
    if 'tables' not in schema:
        return tables, columns
    
    for table_name, table_def in schema['tables'].items():
        # Table constant: PATIENTS, USERS, etc.
        table_const = table_name.upper()
        tables.add(table_const)
        
        # Column constants for this table
        if 'columns' in table_def:
            table_columns = set()
            for col_name in table_def['columns'].keys():
                # Column constant: MRN, NAME, etc.
                col_const = col_name.upper()
                table_columns.add(col_const)
            columns[table_name] = table_columns
    
    return tables, columns

def validate_orm_mapping_file(mapping_file: Path, schema: dict) -> List[str]:
    """Validate a single ORM mapping file."""
    errors = []
    
    # Extract expected constants
    expected_tables, expected_columns = extract_schema_constants(schema)
    
    try:
        with open(mapping_file, 'r') as f:
            content = f.read()
    except Exception as e:
        errors.append(f"Cannot read {mapping_file}: {e}")
        return errors
    
    # Check for hardcoded table names (t.setName("..."))
    hardcoded_table_pattern = r't\.setName\(["\']([^"\']+)["\']\)'
    for match in re.finditer(hardcoded_table_pattern, content):
        table_name = match.group(1)
        errors.append(f"{mapping_file.name}: Hardcoded table name '{table_name}' found. Use Schema::Tables:: constant instead.")
    
    # Check for hardcoded column names (t.data(..., "..."))
    hardcoded_column_pattern = r't\.(?:id|data)\([^,]+,\s*["\']([^"\']+)["\']\)'
    for match in re.finditer(hardcoded_column_pattern, content):
        column_name = match.group(1)
        errors.append(f"{mapping_file.name}: Hardcoded column name '{column_name}' found. Use Schema::Columns:: constant instead.")
    
    # Check that Schema constants are used
    # Accept either direct usage (Schema::Tables::PATIENTS) or using namespace (using namespace Schema::Tables; PATIENTS)
    has_table_namespace = 'using namespace Schema::Tables' in content or 'Schema::Tables::' in content
    has_column_namespace = 'using namespace Schema::Columns::' in content or 'Schema::Columns::' in content
    
    if not has_table_namespace and 't.setName(' in content:
        errors.append(f"{mapping_file.name}: Table name should use Schema::Tables:: constant or 'using namespace Schema::Tables'")
    
    if not has_column_namespace and ('t.data(' in content or 't.id(' in content):
        errors.append(f"{mapping_file.name}: Column names should use Schema::Columns:: constants or 'using namespace Schema::Columns::'")
    
    return errors

def main():
    """Main validation function."""
    # Get project root (assume script is in scripts/ directory)
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    schema_yaml = project_root / 'schema' / 'database.yaml'
    orm_dir = project_root / 'src' / 'infrastructure' / 'persistence' / 'orm'
    
    # Check if schema YAML exists
    if not schema_yaml.exists():
        print(f"❌ Schema YAML not found: {schema_yaml}")
        return 1
    
    # Check if ORM directory exists
    if not orm_dir.exists():
        print(f"⚠️  ORM directory not found: {orm_dir}")
        print("   (This is OK if ORM is not yet implemented)")
        return 0
    
    # Load schema
    try:
        schema = load_schema(schema_yaml)
    except Exception as e:
        print(f"❌ Cannot load schema YAML: {e}")
        return 1
    
    # Find all ORM mapping files
    mapping_files = list(orm_dir.glob('*Entity.h')) + list(orm_dir.glob('*Mapping.h'))
    
    if not mapping_files:
        print("⚠️  No ORM mapping files found")
        print("   (This is OK if ORM is not yet implemented)")
        return 0
    
    # Validate each mapping file
    all_errors = []
    for mapping_file in mapping_files:
        errors = validate_orm_mapping_file(mapping_file, schema)
        all_errors.extend(errors)
    
    # Report results
    if all_errors:
        print("❌ ORM mapping validation FAILED:")
        for error in all_errors:
            print(f"   {error}")
        return 1
    else:
        print("✅ All ORM mappings validated successfully")
        print(f"   Validated {len(mapping_files)} mapping file(s)")
        return 0

if __name__ == "__main__":
    sys.exit(main())

