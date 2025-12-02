#!/usr/bin/env python3
"""
generate-doc-index.py

Generates master index and category indexes for documentation.

Usage:
    python3 scripts/generate-doc-index.py

Generates:
    - doc/00_INDEX.md - Master index with all documents
    - doc/_metadata.json - Machine-readable metadata for AI agents
    - doc/{category}/_index.md - Category-specific indexes
"""

import os
import re
import json
import yaml
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Any
from collections import defaultdict

# Base paths
SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent
DOC_ROOT = PROJECT_ROOT / "project-dashboard" / "doc"

# Category metadata
CATEGORIES = {
    "ARCH": {"name": "Architecture", "dir": "architecture"},
    "REQ": {"name": "Requirements", "dir": "requirements"},
    "API": {"name": "API", "dir": "api"},
    "COMP": {"name": "Component", "dir": "components"},
    "PROC": {"name": "Process", "dir": "processes"},
    "GUIDE": {"name": "Guideline", "dir": "guidelines"},
    "REF": {"name": "Reference", "dir": "reference"},
    "TRAIN": {"name": "Training", "dir": "training"},
    "REG": {"name": "Regulatory", "dir": "regulatory"},
}


def extract_metadata(file_path: Path) -> Dict[str, Any]:
    """Extract YAML frontmatter metadata from markdown file."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Match YAML frontmatter
        match = re.match(r'^---\s*\n(.*?)\n---\s*\n', content, re.DOTALL)
        if not match:
            return {}
        
        yaml_content = match.group(1)
        metadata = yaml.safe_load(yaml_content)
        
        # Add file path and relative path
        metadata['file_path'] = str(file_path.relative_to(DOC_ROOT))
        metadata['absolute_path'] = str(file_path)
        
        return metadata
    except Exception as e:
        print(f"Warning: Failed to extract metadata from {file_path}: {e}")
        return {}


def scan_documents() -> Dict[str, List[Dict[str, Any]]]:
    """Scan all documentation files and extract metadata."""
    docs_by_category = defaultdict(list)
    
    for category_code, category_info in CATEGORIES.items():
        category_dir = DOC_ROOT / category_info["dir"]
        if not category_dir.exists():
            continue
        
        # Find all markdown files except indexes
        for md_file in category_dir.rglob("*.md"):
            if md_file.name.startswith("_"):
                continue  # Skip index files
            
            metadata = extract_metadata(md_file)
            if metadata and metadata.get("doc_id"):
                docs_by_category[category_code].append(metadata)
    
    # Sort documents by DOC-ID within each category
    for category_code in docs_by_category:
        docs_by_category[category_code].sort(key=lambda d: d.get("doc_id", ""))
    
    return docs_by_category


def generate_master_index(docs_by_category: Dict[str, List[Dict[str, Any]]]) -> str:
    """Generate master index markdown."""
    lines = [
        "# Documentation Index",
        "",
        f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        "",
        "This is the master index of all documentation in the Z Monitor project.",
        "",
        "## Quick Navigation",
        ""
    ]
    
    # Quick navigation by category
    for category_code, category_info in CATEGORIES.items():
        doc_count = len(docs_by_category.get(category_code, []))
        lines.append(f"- [{category_info['name']}](#{category_code.lower()}) ({doc_count} documents)")
    
    lines.extend([
        "",
        "## Statistics",
        "",
        f"- **Total Documents:** {sum(len(docs) for docs in docs_by_category.values())}",
        f"- **Categories:** {len([c for c in CATEGORIES if docs_by_category.get(c)])}",
        f"- **Last Updated:** {datetime.now().strftime('%Y-%m-%d')}",
        "",
        "---",
        ""
    ])
    
    # Documents by category
    for category_code, category_info in CATEGORIES.items():
        docs = docs_by_category.get(category_code, [])
        if not docs:
            continue
        
        lines.extend([
            f"## {category_info['name']} {{#{category_code.lower()}}}",
            "",
            f"**Directory:** `{category_info['dir']}/`",
            "",
            f"**Total:** {len(docs)} documents",
            "",
            "| DOC-ID | Title | Version | Status | Owner |",
            "|--------|-------|---------|--------|-------|"
        ])
        
        for doc in docs:
            doc_id = doc.get("doc_id", "N/A")
            title = doc.get("title", "Untitled")
            version = doc.get("version", "N/A")
            status = doc.get("status", "N/A")
            owner = doc.get("owner", "N/A")
            file_path = doc.get("file_path", "")
            
            # Create markdown link
            link = f"[{doc_id}]({file_path})"
            
            lines.append(f"| {link} | {title} | {version} | {status} | {owner} |")
        
        lines.extend(["", "---", ""])
    
    # Search tips
    lines.extend([
        "## Search Tips",
        "",
        "### By Category",
        "Use the quick navigation links above to jump to specific categories.",
        "",
        "### By DOC-ID",
        "Search for `DOC-{CATEGORY}-{NUMBER}` to find specific documents.",
        "",
        "### By Tag",
        "See `_metadata.json` for searchable tags and cross-references.",
        "",
        "### By Status",
        "- **Draft:** Work in progress",
        "- **In Review:** Awaiting feedback",
        "- **Approved:** Reviewed and approved",
        "- **Published:** Officially published",
        "- **Deprecated:** Superseded by newer version",
        "",
        "---",
        "",
        "## Related Documentation",
        "",
        "- [Documentation Guidelines](../.github/doc_guidelines.md)",
        "- [DOC Reorganization Plan](../project-dashboard/DOC_REORGANIZATION_PLAN.md)",
        "- [ZTODO Task Guidelines](../.github/ztodo_task_guidelines.md)",
        ""
    ])
    
    return "\n".join(lines)


def generate_category_index(category_code: str, docs: List[Dict[str, Any]]) -> str:
    """Generate category-specific index."""
    category_info = CATEGORIES[category_code]
    
    lines = [
        f"# {category_info['name']} Documentation",
        "",
        f"**Category:** {category_code}",
        f"**Total Documents:** {len(docs)}",
        f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        "",
        "## Documents",
        ""
    ]
    
    # Group by subcategory if available
    by_subcategory = defaultdict(list)
    for doc in docs:
        subcategory = doc.get("subcategory", "General")
        by_subcategory[subcategory].append(doc)
    
    for subcategory in sorted(by_subcategory.keys()):
        lines.extend([
            f"### {subcategory}",
            ""
        ])
        
        for doc in by_subcategory[subcategory]:
            doc_id = doc.get("doc_id", "N/A")
            title = doc.get("title", "Untitled")
            version = doc.get("version", "N/A")
            status = doc.get("status", "N/A")
            file_path = doc.get("file_path", "")
            
            # Relative path from category directory
            rel_path = Path(file_path).relative_to(category_info["dir"])
            
            lines.extend([
                f"#### [{doc_id}: {title}]({rel_path})",
                "",
                f"- **Version:** {version}",
                f"- **Status:** {status}",
                f"- **Owner:** {doc.get('owner', 'N/A')}",
            ])
            
            # Add tags if available
            tags = doc.get("tags", [])
            if tags:
                # Handle both string and dict/list items in tags
                tag_strs = [str(tag) if not isinstance(tag, (dict, list)) else "" for tag in tags]
                tag_strs = [t for t in tag_strs if t]  # Filter empty strings
                if tag_strs:
                    lines.append(f"- **Tags:** {', '.join(tag_strs)}")
            
            # Add related docs if available
            related_docs = doc.get("related_docs", [])
            if related_docs:
                lines.append(f"- **Related:** {', '.join(related_docs)}")
            
            lines.append("")
    
    lines.extend([
        "---",
        "",
        f"[Back to Master Index](../00_INDEX.md)",
        ""
    ])
    
    return "\n".join(lines)


def convert_to_json_serializable(obj):
    """Convert non-serializable objects to JSON-serializable types."""
    from datetime import date, datetime as dt
    
    if isinstance(obj, (date, dt)):
        return obj.isoformat()
    elif isinstance(obj, dict):
        return {k: convert_to_json_serializable(v) for k, v in obj.items()}
    elif isinstance(obj, list):
        return [convert_to_json_serializable(item) for item in obj]
    else:
        return obj

def generate_metadata_json(docs_by_category: Dict[str, List[Dict[str, Any]]]) -> str:
    """Generate machine-readable metadata JSON for AI agents."""
    # Flatten all documents
    all_docs = []
    for docs in docs_by_category.values():
        all_docs.extend(docs)
    
    # Convert all documents to JSON-serializable format
    all_docs = [convert_to_json_serializable(doc) for doc in all_docs]
    
    metadata = {
        "generated": datetime.now().isoformat(),
        "total_documents": len(all_docs),
        "categories": {
            code: {
                "name": info["name"],
                "directory": info["dir"],
                "count": len(docs_by_category.get(code, []))
            }
            for code, info in CATEGORIES.items()
        },
        "documents": all_docs,
        "indexes": {
            "by_doc_id": {doc["doc_id"]: doc for doc in all_docs if "doc_id" in doc},
            "by_tag": create_tag_index(all_docs),
            "by_status": create_status_index(all_docs),
        }
    }
    
    return json.dumps(metadata, indent=2, ensure_ascii=False)


def create_tag_index(docs: List[Dict[str, Any]]) -> Dict[str, List[str]]:
    """Create index of documents by tag."""
    tag_index = defaultdict(list)
    for doc in docs:
        doc_id = doc.get("doc_id")
        tags = doc.get("tags", [])
        for tag in tags:
            # Handle both string and dict/list items in tags
            if isinstance(tag, str) and doc_id:
                tag_index[tag].append(doc_id)
    return dict(tag_index)


def create_status_index(docs: List[Dict[str, Any]]) -> Dict[str, List[str]]:
    """Create index of documents by status."""
    status_index = defaultdict(list)
    for doc in docs:
        doc_id = doc.get("doc_id")
        status = doc.get("status", "Unknown")
        if doc_id:
            status_index[status].append(doc_id)
    return dict(status_index)


def main():
    """Main execution."""
    print("📚 Generating documentation indexes...")
    
    # Scan all documents
    print("  Scanning documents...")
    docs_by_category = scan_documents()
    total_docs = sum(len(docs) for docs in docs_by_category.values())
    print(f"  Found {total_docs} documents across {len(docs_by_category)} categories")
    
    # Generate master index
    print("  Generating master index...")
    master_index = generate_master_index(docs_by_category)
    master_index_path = DOC_ROOT / "00_INDEX.md"
    with open(master_index_path, 'w', encoding='utf-8') as f:
        f.write(master_index)
    print(f"  ✓ Created {master_index_path}")
    
    # Generate category indexes
    print("  Generating category indexes...")
    for category_code, docs in docs_by_category.items():
        if not docs:
            continue
        
        category_info = CATEGORIES[category_code]
        category_dir = DOC_ROOT / category_info["dir"]
        category_index = generate_category_index(category_code, docs)
        
        index_path = category_dir / "_index.md"
        with open(index_path, 'w', encoding='utf-8') as f:
            f.write(category_index)
        print(f"  ✓ Created {index_path}")
    
    # Generate metadata JSON
    print("  Generating metadata JSON...")
    metadata_json = generate_metadata_json(docs_by_category)
    metadata_path = DOC_ROOT / "_metadata.json"
    with open(metadata_path, 'w', encoding='utf-8') as f:
        f.write(metadata_json)
    print(f"  ✓ Created {metadata_path}")
    
    print("")
    print("✅ Index generation complete!")
    print(f"   Total documents: {total_docs}")
    print(f"   Master index: {master_index_path}")
    print(f"   Metadata JSON: {metadata_path}")
    print(f"   Category indexes: {len(docs_by_category)}")


if __name__ == "__main__":
    main()
