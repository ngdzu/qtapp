#!/usr/bin/env python3
"""
generate-doc-index-lite.py

Generates master index and category indexes for documentation without PyYAML.
Targets docs under project-dashboard/doc.
"""
import re
import json
from pathlib import Path
from datetime import datetime
from collections import defaultdict
from typing import Dict, Any, List

SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent
DOC_ROOT = PROJECT_ROOT / "project-dashboard" / "doc"

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


def fallback_yaml_load(yaml_text: str) -> Dict[str, Any]:
    data: Dict[str, Any] = {}
    lines = yaml_text.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i].rstrip()
        if not line or line.strip().startswith('#'):
            i += 1
            continue
        m = re.match(r'^([A-Za-z0-9_\-]+):\s*(.*)$', line)
        if not m:
            i += 1
            continue
        key, val = m.group(1), m.group(2)
        if val == '':
            lst: List[str] = []
            j = i + 1
            while j < len(lines):
                nxt = lines[j]
                if re.match(r'^\s*-\s*(.+)$', nxt):
                    item = re.sub(r'^\s*-\s*', '', nxt).strip()
                    lst.append(item)
                    j += 1
                else:
                    break
            if lst:
                data[key] = lst
                i = j
                continue
            else:
                data[key] = ''
        else:
            if val.startswith('[') and val.endswith(']'):
                inner = val[1:-1].strip()
                items = [x.strip() for x in inner.split(',') if x.strip()]
                data[key] = items
            else:
                data[key] = val.strip()
        i += 1
    return data


def extract_metadata(file_path: Path) -> Dict[str, Any]:
    try:
        content = file_path.read_text(encoding='utf-8')
        match = re.match(r'^---\s*\n(.*?)\n---\s*\n', content, re.DOTALL)
        if not match:
            return {}
        meta = fallback_yaml_load(match.group(1))
        meta['file_path'] = str(file_path.relative_to(DOC_ROOT))
        meta['absolute_path'] = str(file_path)
        return meta
    except Exception:
        return {}


def scan_documents() -> Dict[str, List[Dict[str, Any]]]:
    docs = defaultdict(list)
    for code, info in CATEGORIES.items():
        cat_dir = DOC_ROOT / info['dir']
        if not cat_dir.exists():
            continue
        for md in cat_dir.rglob('*.md'):
            if md.name.startswith('_') or md.name == '00_INDEX.md':
                continue
            meta = extract_metadata(md)
            if meta.get('doc_id'):
                docs[code].append(meta)
        docs[code].sort(key=lambda d: d.get('doc_id', ''))
    return docs


def generate_master_index(docs_by_cat: Dict[str, List[Dict[str, Any]]]) -> str:
    lines = [
        '# Documentation Index',
        '',
        f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        '',
        '## Quick Navigation',
        ''
    ]
    for code, info in CATEGORIES.items():
        count = len(docs_by_cat.get(code, []))
        lines.append(f"- [{info['name']}](#{code.lower()}) ({count} documents)")
    lines.extend(['', '---', ''])
    for code, info in CATEGORIES.items():
        docs = docs_by_cat.get(code, [])
        if not docs:
            continue
        lines.extend([
            f"## {info['name']} {{#{code.lower()}}}",
            '',
            '| DOC-ID | Title | Version | Status |',
            '|--------|-------|---------|--------|'
        ])
        for d in docs:
            link = f"[{d.get('doc_id','N/A')}]({d.get('file_path','')})"
            lines.append(f"| {link} | {d.get('title','Untitled')} | {d.get('version','N/A')} | {d.get('status','N/A')} |")
        lines.extend(['', '---', ''])
    return '\n'.join(lines)


def generate_category_index(code: str, docs: List[Dict[str, Any]]) -> str:
    info = CATEGORIES[code]
    lines = [
        f"# {info['name']} Documentation",
        '',
        f"**Category:** {code}",
        f"**Total Documents:** {len(docs)}",
        f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        '',
        '## Documents',
        ''
    ]
    groups = defaultdict(list)
    for d in docs:
        groups[d.get('subcategory','General')].append(d)
    for sub in sorted(groups.keys()):
        lines.extend([f"### {sub}", ''])
        for d in groups[sub]:
            rel = Path(d['file_path']).relative_to(info['dir'])
            lines.extend([
                f"#### [{d.get('doc_id')}: {d.get('title','Untitled')}]({rel})",
                '',
                f"- **Version:** {d.get('version','N/A')}",
                f"- **Status:** {d.get('status','N/A')}",
                f"- **Owner:** {d.get('owner','N/A')}",
                ''
            ])
    lines.extend(['---', '', '[Back to Master Index](../00_INDEX.md)', ''])
    return '\n'.join(lines)


def create_tag_index(docs: List[Dict[str, Any]]) -> Dict[str, List[str]]:
    idx = defaultdict(list)
    for d in docs:
        did = d.get('doc_id')
        tags = d.get('tags', [])
        for t in tags:
            if isinstance(t, str) and did:
                idx[t].append(did)
    return dict(idx)


def create_status_index(docs: List[Dict[str, Any]]) -> Dict[str, List[str]]:
    idx = defaultdict(list)
    for d in docs:
        did = d.get('doc_id')
        status = d.get('status','Unknown')
        if did:
            idx[status].append(did)
    return dict(idx)


def main():
    print('📚 Generating documentation indexes (lite)...')
    docs_by_cat = scan_documents()
    total = sum(len(v) for v in docs_by_cat.values())
    print(f'  Found {total} documents')

    # Master index
    master = generate_master_index(docs_by_cat)
    (DOC_ROOT / '00_INDEX.md').write_text(master, encoding='utf-8')
    print(f'  ✓ Created {DOC_ROOT / "00_INDEX.md"}')

    # Category indexes
    for code, docs in docs_by_cat.items():
        if not docs:
            # Ensure category dir exists and a minimal index
            cat_dir = DOC_ROOT / CATEGORIES[code]['dir']
            cat_dir.mkdir(parents=True, exist_ok=True)
        idx_md = generate_category_index(code, docs)
        (DOC_ROOT / CATEGORIES[code]['dir'] / '_index.md').write_text(idx_md, encoding='utf-8')
        print(f'  ✓ Created {DOC_ROOT / CATEGORIES[code]["dir"] / "_index.md"}')

    # Metadata JSON
    all_docs: List[Dict[str, Any]] = []
    for docs in docs_by_cat.values():
        all_docs.extend(docs)
    meta = {
        'generated': datetime.now().isoformat(),
        'total_documents': len(all_docs),
        'categories': {code: {"name": info['name'], "directory": info['dir'], "count": len(docs_by_cat.get(code, []))} for code, info in CATEGORIES.items()},
        'documents': all_docs,
        'indexes': {
            'by_doc_id': {d['doc_id']: d for d in all_docs if 'doc_id' in d},
            'by_tag': create_tag_index(all_docs),
            'by_status': create_status_index(all_docs)
        }
    }
    (DOC_ROOT / '_metadata.json').write_text(json.dumps(meta, indent=2), encoding='utf-8')
    print(f'  ✓ Created {DOC_ROOT / "_metadata.json"}')

    print('\n✅ Index generation complete (lite).')


if __name__ == '__main__':
    main()
