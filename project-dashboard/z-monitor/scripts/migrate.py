#!/usr/bin/env python3
"""
Database Migration Runner

Applies numbered migration files in order.

Usage:
    python3 scripts/migrate.py --db /path/to/database.db
"""

import sqlite3
import os
import sys
import argparse
from pathlib import Path
from datetime import datetime

def get_current_version(conn: sqlite3.Connection) -> int:
    """Get current schema version from database."""
    cursor = conn.cursor()
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS schema_version (
            version INTEGER PRIMARY KEY,
            applied_at TEXT NOT NULL,
            description TEXT,
            migration_type TEXT DEFAULT 'schema'
        )
    """)
    
    cursor.execute("SELECT MAX(version) FROM schema_version")
    result = cursor.fetchone()
    return result[0] if result[0] is not None else 0

def apply_migration(conn: sqlite3.Connection, migration_file: Path, version: int) -> bool:
    """Apply a single migration file.
    
    Returns:
        True if successful, False if failed
    """
    print(f"📦 Applying migration {version}: {migration_file.name}")
    
    sql = None
    try:
        # Read migration file
        with open(migration_file, 'r') as f:
            sql = f.read()
        
        # Begin transaction
        conn.execute("BEGIN TRANSACTION")
        
        cursor = conn.cursor()
        cursor.executescript(sql)
        
        # Record migration in schema_version table
        cursor.execute("""
            INSERT INTO schema_version (version, applied_at, description, migration_type)
            VALUES (?, datetime('now'), ?, 'schema')
        """, (version, migration_file.stem))
        
        # Commit transaction
        conn.commit()
        print(f"✅ Migration {version} applied successfully")
        return True
        
    except sqlite3.Error as e:
        # ✅ Log database error (infrastructure failure)
        conn.rollback()
        print(f"❌ Migration {version} FAILED (database error): {e}")
        print(f"   File: {migration_file}")
        if sql:
            print(f"   SQL: {sql[:200]}...")  # First 200 chars for debugging
        return False  # ✅ Return error status
        
    except FileNotFoundError as e:
        # ✅ Log file error (infrastructure failure)
        print(f"❌ Migration {version} FAILED (file not found): {e}")
        return False
        
    except Exception as e:
        # ✅ Log unexpected error (infrastructure failure)
        conn.rollback()
        print(f"❌ Migration {version} FAILED (unexpected error): {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Database Migration Runner')
    parser.add_argument('--db', required=True, help='Path to database file')
    parser.add_argument('--migrations-dir', default='schema/migrations', 
                       help='Directory containing migration files (default: schema/migrations)')
    args = parser.parse_args()
    
    # Get script directory and project root
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    migrations_dir = project_root / args.migrations_dir
    
    if not migrations_dir.exists():
        print(f"❌ Error: Migrations directory not found: {migrations_dir}")
        sys.exit(1)
    
    db_path = Path(args.db)
    if not db_path.exists() and db_path.parent.exists():
        # Create database file if parent directory exists
        db_path.touch()
    
    conn = sqlite3.connect(str(db_path))
    
    current_version = get_current_version(conn)
    print(f"📊 Current schema version: {current_version}")
    
    # Get all migration files
    migration_files = sorted(migrations_dir.glob("*.sql"))
    
    if not migration_files:
        print(f"⚠️  No migration files found in {migrations_dir}")
        conn.close()
        sys.exit(0)
    
    applied_count = 0
    for migration_file in migration_files:
        # Extract version number from filename (e.g., 0001_initial_schema.sql)
        try:
            version_str = migration_file.stem.split('_')[0]
            version = int(version_str)
        except (ValueError, IndexError):
            print(f"⚠️  Skipping invalid migration file: {migration_file.name} (expected format: NNNN_description.sql)")
            continue
        
        if version > current_version:
            success = apply_migration(conn, migration_file, version)
            if not success:
                # ✅ Stop on first failure (don't continue with partial migration)
                print(f"❌ Migration process stopped at version {version}")
                conn.close()
                sys.exit(1)  # ✅ Return error status
            applied_count += 1
    
    final_version = get_current_version(conn)
    
    if applied_count > 0:
        print(f"✅ Applied {applied_count} migration(s)")
    else:
        print("✅ No migrations to apply")
    
    print(f"📊 Database is up to date (version {final_version})")
    
    conn.close()
    sys.exit(0)

if __name__ == "__main__":
    main()

