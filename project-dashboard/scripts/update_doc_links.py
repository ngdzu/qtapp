#!/usr/bin/env python3
"""
Update all document references to use proper markdown links instead of just filenames
"""

import re
from pathlib import Path

Z_MONITOR_DOCS = Path("project-dashboard/doc/z-monitor/architecture_and_design")

def update_to_markdown_links(content: str) -> str:
    """Convert document name references to markdown links"""
    
    # Pattern 1: (see `FILENAME.md`)  ->  (see [FILENAME.md](./FILENAME.md))
    content = re.sub(
        r'\(see `(\d+_[A-Z_]+\.md)`\)',
        r'(see [\1](./\1))',
        content
    )
    
    # Pattern 2: See `FILENAME.md` for  ->  See [FILENAME.md](./FILENAME.md) for
    content = re.sub(
        r'See `(\d+_[A-Z_]+\.md)` for',
        r'See [\1](./\1) for',
        content
    )
    
    # Pattern 3: see `doc/FILENAME.md`  ->  see [FILENAME.md](./FILENAME.md)
    content = re.sub(
        r'see `doc/(\d+_[A-Z_]+\.md)`',
        r'see [\1](./\1)',
        content
    )
    
    # Pattern 4: See `doc/FILENAME.md`  ->  See [FILENAME.md](./FILENAME.md)
    content = re.sub(
        r'See `doc/(\d+_[A-Z_]+\.md)`',
        r'See [\1](./\1)',
        content
    )
    
    # Pattern 5: `doc/FILENAME.md` - Description  ->  [FILENAME.md](./FILENAME.md) - Description
    content = re.sub(
        r'`doc/(\d+_[A-Z_]+\.md)` - ',
        r'[\1](./\1) - ',
        content
    )
    
    # Pattern 6: in `FILENAME.md` (context)  ->  in [FILENAME.md](./FILENAME.md) (context)
    content = re.sub(
        r'in `(\d+_[A-Z_]+\.md)`',
        r'in [\1](./\1)',
        content
    )
    
    # Pattern 7: `FILENAME.md` (Section X)  ->  [FILENAME.md](./FILENAME.md) (Section X)
    content = re.sub(
        r'`(\d+_[A-Z_]+\.md)` \(Section',
        r'[\1](./\1) (Section',
        content
    )
    
    return content

def process_file(filepath: Path) -> tuple[bool, int]:
    """Process a single file and return (changed, num_replacements)"""
    
    with open(filepath, 'r', encoding='utf-8') as f:
        original = f.read()
    
    updated = update_to_markdown_links(original)
    
    if updated != original:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(updated)
        
        # Count number of changes
        num_changes = updated.count('](./') - original.count('](./') 
        return True, num_changes
    
    return False, 0

def main():
    """Update all markdown files in Z Monitor docs"""
    
    print("📝 Updating document references to markdown links...\n")
    
    total_files = 0
    updated_files = 0
    total_changes = 0
    
    for md_file in Z_MONITOR_DOCS.glob("*.md"):
        total_files += 1
        changed, num_changes = process_file(md_file)
        
        if changed:
            updated_files += 1
            total_changes += num_changes
            print(f"✅ {md_file.name:<50} ({num_changes} links)")
    
    print(f"\n{'='*70}")
    print(f"📊 Summary:")
    print(f"   Total files: {total_files}")
    print(f"   Updated files: {updated_files}")
    print(f"   Total link conversions: {total_changes}")
    print(f"{'='*70}")

if __name__ == "__main__":
    main()

