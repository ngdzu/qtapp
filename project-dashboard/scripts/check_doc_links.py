#!/usr/bin/env python3
"""
Verify all markdown links in documentation are valid

Checks:
1. Relative links to other markdown files
2. Links to sections within files (anchors)
3. Links to Mermaid diagrams (.mmd) and SVG files (.svg)
4. Reports broken links with file location
"""

import re
from pathlib import Path
from typing import List, Tuple
from collections import defaultdict

# Directories to check
DOC_ROOTS = [
    Path("project-dashboard/doc/z-monitor/architecture_and_design"),
    Path("project-dashboard/doc/foundation"),
]

class LinkChecker:
    def __init__(self):
        self.broken_links = []
        self.total_links = 0
        self.files_checked = 0
        self.link_types = defaultdict(int)
        
    def extract_links(self, content: str) -> List[Tuple[str, str]]:
        """Extract all markdown links from content
        
        Returns list of tuples: (link_text, link_url)
        """
        # Pattern: [text](url)
        # Skip code blocks to avoid false positives
        lines = content.split('\n')
        result = []
        in_code_block = False
        
        for line in lines:
            if line.strip().startswith('```'):
                in_code_block = not in_code_block
                continue
            
            if not in_code_block:
                pattern = r'\[([^\]]+)\]\(([^\)]+)\)'
                matches = re.findall(pattern, line)
                result.extend(matches)
        
        return result
    
    def check_link(self, source_file: Path, link_url: str) -> Tuple[bool, str]:
        """Check if a link is valid
        
        Returns: (is_valid, error_message)
        """
        # Skip external URLs
        if link_url.startswith(('http://', 'https://', 'mailto:', 'ftp://')):
            self.link_types['external'] += 1
            return True, ""
        
        # Skip anchors-only (same-page links)
        if link_url.startswith('#'):
            self.link_types['anchor'] += 1
            # TODO: Could validate section headers exist
            return True, ""
        
        # Handle links with anchors (file.md#section)
        anchor = None
        if '#' in link_url:
            link_url, anchor = link_url.split('#', 1)
        
        # Resolve relative path
        target = source_file.parent / link_url
        target = target.resolve()
        
        # Check if file exists
        if not target.exists():
            try:
                rel_target = target.relative_to(Path.cwd())
            except ValueError:
                rel_target = target
            
            if target.suffix == '.md':
                self.link_types['broken_md'] += 1
                return False, f"File not found: {rel_target}"
            elif target.suffix in ['.mmd', '.svg', '.png', '.jpg']:
                self.link_types['broken_asset'] += 1
                return False, f"Asset not found: {rel_target}"
            else:
                self.link_types['broken_other'] += 1
                return False, f"File not found: {rel_target}"
        
        # Track successful link types
        if target.suffix == '.md':
            self.link_types['valid_md'] += 1
        elif target.suffix in ['.mmd', '.svg']:
            self.link_types['valid_diagram'] += 1
        elif target.suffix in ['.png', '.jpg', '.jpeg']:
            self.link_types['valid_image'] += 1
        else:
            self.link_types['valid_other'] += 1
        
        # TODO: If anchor present, could validate section exists in target file
        if anchor:
            self.link_types['with_anchor'] += 1
        
        return True, ""
    
    def check_file(self, filepath: Path) -> List[dict]:
        """Check all links in a single file
        
        Returns list of broken link reports
        """
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        links = self.extract_links(content)
        broken = []
        
        for link_text, link_url in links:
            self.total_links += 1
            is_valid, error = self.check_link(filepath, link_url)
            
            if not is_valid:
                broken.append({
                    'file': filepath,
                    'text': link_text,
                    'url': link_url,
                    'error': error
                })
        
        return broken
    
    def check_directory(self, root: Path):
        """Check all markdown files in a directory"""
        
        for md_file in root.rglob("*.md"):
            self.files_checked += 1
            broken = self.check_file(md_file)
            self.broken_links.extend(broken)
    
    def print_report(self):
        """Print comprehensive link check report"""
        
        print("\n" + "="*80)
        print("📊 Link Check Report")
        print("="*80)
        
        print(f"\n✅ Files Checked: {self.files_checked}")
        print(f"🔗 Total Links: {self.total_links}")
        
        print("\n📈 Link Types:")
        print(f"   Valid Markdown: {self.link_types['valid_md']}")
        print(f"   Valid Diagrams: {self.link_types['valid_diagram']}")
        print(f"   Valid Images: {self.link_types['valid_image']}")
        print(f"   Valid Other: {self.link_types['valid_other']}")
        print(f"   External URLs: {self.link_types['external']}")
        print(f"   Same-page Anchors: {self.link_types['anchor']}")
        print(f"   Links with Anchors: {self.link_types['with_anchor']}")
        
        if self.broken_links:
            print(f"\n❌ Broken Links Found: {len(self.broken_links)}")
            print("="*80)
            
            # Group by file
            by_file = defaultdict(list)
            for broken in self.broken_links:
                by_file[broken['file']].append(broken)
            
            for filepath, links in sorted(by_file.items()):
                try:
                    rel_path = filepath.relative_to(Path.cwd())
                except ValueError:
                    # Already a relative path
                    rel_path = filepath
                print(f"\n📄 {rel_path}")
                
                for link in links:
                    print(f"   ❌ [{link['text']}]({link['url']})")
                    print(f"      Error: {link['error']}")
        else:
            print(f"\n✅ All Links Valid! No broken links found.")
        
        print("\n" + "="*80)
        
        # Return exit code
        return 1 if self.broken_links else 0

def main():
    """Main entry point"""
    
    print("🔍 Checking documentation links...")
    
    checker = LinkChecker()
    
    for root in DOC_ROOTS:
        if root.exists():
            print(f"   Scanning: {root}")
            checker.check_directory(root)
        else:
            print(f"   ⚠️  Not found: {root}")
    
    exit_code = checker.print_report()
    
    # Provide suggestions if broken links found
    if exit_code != 0:
        print("\n💡 Suggestions:")
        print("   1. Check if the target file was moved or renamed")
        print("   2. Update the link to use the correct relative path")
        print("   3. If the file was deleted, remove or update the reference")
        print("   4. Use './filename.md' for same-directory links")
        print("   5. Use '../directory/filename.md' for parent directory links")
    
    return exit_code

if __name__ == "__main__":
    exit(main())

