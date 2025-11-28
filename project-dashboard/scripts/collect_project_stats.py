#!/usr/bin/env python3
"""
Project Statistics Collector

Collects statistics about the Z Monitor project including:
- Lines of code
- Number of files
- Number of classes
- Average lines of code per file
- Average lines of code per class
- Average lines of code per function
- Classes per file (for files with classes)
- Functions per file (for files with functions)

Usage:
    # Basic usage (text output to stdout)
    python3 scripts/collect_project_stats.py --root z-monitor/src
    
    # Output as JSON
    python3 scripts/collect_project_stats.py --root z-monitor/src --output json
    
    # Output as Markdown to file
    python3 scripts/collect_project_stats.py --root z-monitor/src --output markdown --output-file doc/z-monitor/PROJECT_STATS.md
    
    # Include test files
    python3 scripts/collect_project_stats.py --root z-monitor/src --include-tests

Examples:
    # Analyze z-monitor source code
    python3 scripts/collect_project_stats.py --root z-monitor/src
    
    # Analyze entire z-monitor project (including tests)
    python3 scripts/collect_project_stats.py --root z-monitor --include-tests
    
    # Generate markdown report
    python3 scripts/collect_project_stats.py --root z-monitor/src --output markdown --output-file PROJECT_STATS.md

For detailed documentation, see: scripts/README_COLLECT_PROJECT_STATS.md
"""

import os
import re
import json
import argparse
from pathlib import Path
from typing import Dict, List, Tuple
from collections import defaultdict

class ProjectStatsCollector:
    """Collects statistics from C++ source files."""
    
    def __init__(self, root_dir: str):
        self.root_dir = Path(root_dir)
        self.stats = {
            'total_lines': 0,
            'total_files': 0,
            'total_classes': 0,
            'total_functions': 0,
            'files_by_type': defaultdict(int),
            'classes_by_file': [],
            'functions_by_file': [],
            'files': []
        }
        
    def collect(self, include_tests: bool = False) -> Dict:
        """Collect statistics from all C++ files."""
        cpp_files = list(self.root_dir.rglob('*.cpp')) + list(self.root_dir.rglob('*.h'))
        
        for file_path in cpp_files:
            # Skip generated files
            if 'generated' in str(file_path):
                continue
            
            # Optionally skip test files
            if not include_tests and 'test' in str(file_path).lower():
                continue
                
            file_stats = self.analyze_file(file_path)
            if file_stats:
                self.stats['files'].append(file_stats)
                self.stats['total_files'] += 1
                self.stats['total_lines'] += file_stats['lines']
                self.stats['total_classes'] += file_stats['classes']
                self.stats['total_functions'] += file_stats['functions']
                self.stats['files_by_type'][file_path.suffix] += 1
                self.stats['classes_by_file'].append(file_stats['classes'])
                self.stats['functions_by_file'].append(file_stats['functions'])
        
        # Calculate averages
        if self.stats['total_files'] > 0:
            self.stats['avg_lines_per_file'] = self.stats['total_lines'] / self.stats['total_files']
        else:
            self.stats['avg_lines_per_file'] = 0
            
        if self.stats['total_classes'] > 0:
            self.stats['avg_lines_per_class'] = self.stats['total_lines'] / self.stats['total_classes']
        else:
            self.stats['avg_lines_per_class'] = 0
            
        if self.stats['total_functions'] > 0:
            self.stats['avg_lines_per_function'] = self.stats['total_lines'] / self.stats['total_functions']
        else:
            self.stats['avg_lines_per_function'] = 0
        
        # Additional statistics
        if self.stats['classes_by_file']:
            files_with_classes = [c for c in self.stats['classes_by_file'] if c > 0]
            if files_with_classes:
                self.stats['avg_classes_per_file'] = sum(files_with_classes) / len(files_with_classes)
            else:
                self.stats['avg_classes_per_file'] = 0
        else:
            self.stats['avg_classes_per_file'] = 0
            
        if self.stats['functions_by_file']:
            files_with_functions = [f for f in self.stats['functions_by_file'] if f > 0]
            if files_with_functions:
                self.stats['avg_functions_per_file'] = sum(files_with_functions) / len(files_with_functions)
            else:
                self.stats['avg_functions_per_file'] = 0
        else:
            self.stats['avg_functions_per_file'] = 0
        
        return self.stats
    
    def analyze_file(self, file_path: Path) -> Dict:
        """Analyze a single C++ file."""
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                lines = content.split('\n')
        except Exception as e:
            print(f"Warning: Could not read {file_path}: {e}")
            return None
        
        # Count non-empty lines (excluding comments and blank lines)
        code_lines = 0
        for line in lines:
            stripped = line.strip()
            if stripped and not stripped.startswith('//') and not stripped.startswith('/*') and not stripped.startswith('*'):
                code_lines += 1
        
        # Count classes
        classes = self.count_classes(content)
        
        # Count functions
        functions = self.count_functions(content)
        
        return {
            'path': str(file_path.relative_to(self.root_dir)),
            'lines': len(lines),
            'code_lines': code_lines,
            'classes': classes,
            'functions': functions
        }
    
    def count_classes(self, content: str) -> int:
        """Count class declarations in C++ code."""
        count = 0
        
        # Remove comments to avoid false matches
        content_no_comments = self.remove_comments(content)
        
        # Match class declarations (including template classes)
        # Pattern: class/struct/enum class followed by name and opening brace
        class_pattern = r'\b(?:class|struct|enum\s+class)\s+\w+\s*(?:<[^>]*>)?\s*(?::[^{]*)?\{'
        matches = re.finditer(class_pattern, content_no_comments, re.MULTILINE)
        
        for match in matches:
            # Verify it's not a forward declaration by checking for opening brace
            pos = match.end()
            # Look ahead for opening brace (might be on next line)
            ahead = content_no_comments[pos:pos+200].strip()
            if '{' in ahead[:100] or ahead.startswith('{'):
                count += 1
        
        return count
    
    def count_functions(self, content: str) -> int:
        """Count function definitions in C++ code."""
        count = 0
        
        # Remove comments and strings to avoid false matches
        content_no_comments = self.remove_comments(content)
        
        # Pattern 1: Method definitions (Class::method)
        # Matches: return_type Class::method(...) { or Class::method(...) {
        method_pattern = r'\w+\s*::\s*\w+\s*\([^)]*\)\s*(?:const\s*)?\s*(?:override\s*)?\s*\{'
        matches = re.finditer(method_pattern, content_no_comments, re.MULTILINE)
        for match in matches:
            count += 1
        
        # Pattern 2: Standalone function definitions
        # Matches: return_type function_name(...) {
        # Excludes: declarations ending with ;, includes definitions with {
        function_pattern = r'(?:\w+\s+)+[~]?\w+\s*\([^)]*\)\s*(?:const\s*)?\s*(?:override\s*)?\s*\{'
        matches = re.finditer(function_pattern, content_no_comments, re.MULTILINE)
        for match in matches:
            # Exclude if it's a method (has ::)
            if '::' not in match.group(0):
                count += 1
        
        # Pattern 3: Constructor/destructor initializer lists
        # Matches: Class::Class(...) : member(...) {
        init_list_pattern = r'\w+\s*::\s*[~]?\w+\s*\([^)]*\)\s*:\s*\w+'
        matches = re.finditer(init_list_pattern, content_no_comments, re.MULTILINE)
        for match in matches:
            # Check if followed by { (definition, not just declaration)
            pos = match.end()
            ahead = content_no_comments[pos:pos+100].strip()
            if '{' in ahead[:50]:
                # Check we haven't already counted this
                if not any(m.start() == match.start() for m in re.finditer(method_pattern, content_no_comments)):
                    count += 1
        
        return count
    
    def remove_comments(self, content: str) -> str:
        """Remove C++ comments from content."""
        # Remove single-line comments
        content = re.sub(r'//.*?$', '', content, flags=re.MULTILINE)
        # Remove multi-line comments
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        return content
    
    def format_text(self) -> str:
        """Format statistics as plain text."""
        stats = self.stats
        output = []
        output.append("=" * 60)
        output.append("Z Monitor Project Statistics")
        output.append("=" * 60)
        output.append("")
        output.append(f"Total Lines of Code: {stats['total_lines']:,}")
        output.append(f"Total Files: {stats['total_files']}")
        output.append(f"  - Header files (.h): {stats['files_by_type'].get('.h', 0)}")
        output.append(f"  - Source files (.cpp): {stats['files_by_type'].get('.cpp', 0)}")
        output.append(f"Total Classes: {stats['total_classes']}")
        output.append(f"Total Functions: {stats['total_functions']}")
        output.append("")
        output.append("Averages:")
        output.append(f"  - Lines per file: {stats['avg_lines_per_file']:.1f}")
        output.append(f"  - Lines per class: {stats['avg_lines_per_class']:.1f}")
        output.append(f"  - Lines per function: {stats['avg_lines_per_function']:.1f}")
        output.append(f"  - Classes per file (files with classes): {stats.get('avg_classes_per_file', 0):.1f}")
        output.append(f"  - Functions per file (files with functions): {stats.get('avg_functions_per_file', 0):.1f}")
        output.append("")
        output.append("=" * 60)
        return "\n".join(output)
    
    def format_json(self) -> str:
        """Format statistics as JSON."""
        return json.dumps(self.stats, indent=2)
    
    def format_markdown(self) -> str:
        """Format statistics as Markdown."""
        stats = self.stats
        output = []
        output.append("# Z Monitor Project Statistics")
        output.append("")
        output.append("## Overview")
        output.append("")
        output.append(f"- **Total Lines of Code:** {stats['total_lines']:,}")
        output.append(f"- **Total Files:** {stats['total_files']}")
        output.append(f"  - Header files (.h): {stats['files_by_type'].get('.h', 0)}")
        output.append(f"  - Source files (.cpp): {stats['files_by_type'].get('.cpp', 0)}")
        output.append(f"- **Total Classes:** {stats['total_classes']}")
        output.append(f"- **Total Functions:** {stats['total_functions']}")
        output.append("")
        output.append("## Averages")
        output.append("")
        output.append(f"- **Lines per file:** {stats['avg_lines_per_file']:.1f}")
        output.append(f"- **Lines per class:** {stats['avg_lines_per_class']:.1f}")
        output.append(f"- **Lines per function:** {stats['avg_lines_per_function']:.1f}")
        output.append(f"- **Classes per file (files with classes):** {stats.get('avg_classes_per_file', 0):.1f}")
        output.append(f"- **Functions per file (files with functions):** {stats.get('avg_functions_per_file', 0):.1f}")
        output.append("")
        return "\n".join(output)


def main():
    parser = argparse.ArgumentParser(description='Collect project statistics')
    parser.add_argument('--root', default='z-monitor/src', help='Root directory to analyze (default: z-monitor/src)')
    parser.add_argument('--output', choices=['text', 'json', 'markdown'], default='text',
                       help='Output format (default: text)')
    parser.add_argument('--output-file', help='Output file path (default: stdout)')
    parser.add_argument('--include-tests', action='store_true', help='Include test files in statistics')
    
    args = parser.parse_args()
    
    root_path = Path(args.root)
    if not root_path.exists():
        # Try relative to project-dashboard
        root_path = Path('project-dashboard') / args.root
        if not root_path.exists():
            print(f"Error: Directory not found: {args.root}")
            return 1
    
    collector = ProjectStatsCollector(root_path)
    stats = collector.collect(include_tests=args.include_tests)
    
    # Format output
    if args.output == 'json':
        output = collector.format_json()
    elif args.output == 'markdown':
        output = collector.format_markdown()
    else:
        output = collector.format_text()
    
    # Write output
    if args.output_file:
        with open(args.output_file, 'w') as f:
            f.write(output)
        print(f"Statistics written to {args.output_file}")
    else:
        print(output)
    
    return 0


if __name__ == '__main__':
    exit(main())

