#!/usr/bin/env python3
"""
Update all Z Monitor document references in foundation index to use new path structure
"""

import re
from pathlib import Path

FOUNDATION_INDEX = Path("project-dashboard/doc/foundation/00_FOUNDATIONAL_KNOWLEDGE_INDEX.md")

def update_references():
    """Update all document references to use new z-monitor/architecture_and_design/ path"""
    
    with open(FOUNDATION_INDEX, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Pattern 1: References in "Document:" lines that are just filenames
    # Match pattern like: - **Document:** `12_THREAD_MODEL.md`
    content = re.sub(
        r'(\*\*Document:\*\*.*?)`(\d+_[A-Z_]+\.md)`',
        r'\1`../z-monitor/architecture_and_design/\2`',
        content
    )
    
    # Pattern 2: References with sections
    # Match: `10_DATABASE_DESIGN.md` (telemetry_metrics table)
    content = re.sub(
        r'`(\d+_[A-Z_]+\.md)`(\s*\([^)]+\))',
        r'`../z-monitor/architecture_and_design/\1`\2',
        content
    )
    
    # Pattern 3: Scattered across references
    # Match: Scattered across `02_ARCHITECTURE.md`, `09_CLASS_DESIGNS.md`
    content = re.sub(
        r'(Scattered across\s+)`(\d+_[A-Z_]+\.md)`',
        r'\1`../z-monitor/architecture_and_design/\2`',
        content
    )
    
    # Pattern 4: "Mentioned in" references
    content = re.sub(
        r'(Mentioned in\s+)`(\d+_[A-Z_]+\.md)`',
        r'\1`../z-monitor/architecture_and_design/\2`',
        content
    )
    
    # Write back
    with open(FOUNDATION_INDEX, 'w') as f:
        f.write(content)
    
    if content != original_content:
        print("✅ Updated Z Monitor document references in foundation index")
        print(f"📄 File: {FOUNDATION_INDEX}")
    else:
        print("ℹ️  No changes needed - all references already updated")

if __name__ == "__main__":
    update_references()

