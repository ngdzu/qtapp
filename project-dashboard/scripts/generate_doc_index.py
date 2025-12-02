#!/usr/bin/env python3
"""
Regenerate documentation master index (00_INDEX.md) and metadata file (_metadata.json).
Scans markdown files under project-dashboard/doc (excluding foundation/) and extracts YAML frontmatter.
Frontmatter requirements (minimum): doc_id, title, version, category, status.
Outputs:
 - 00_INDEX.md grouped by category with doc_id, version, title, relative path
 - _metadata.json listing documents with parsed fields
Exit code:
 - 0 on success
 - 1 if any required metadata missing or duplicate doc_id detected
"""
import sys, re, json, pathlib, datetime
from collections import defaultdict

DOC_ROOT = pathlib.Path(__file__).resolve().parent.parent / 'doc'
EXCLUDE_DIRS = {'foundation', 'z-monitor'}  # Skip generic knowledge and legacy unmigrated docs
REQUIRED_FIELDS = ['doc_id', 'title', 'version', 'category', 'status']
FRONTMATTER_PATTERN = re.compile(r'^---\n(.*?)\n---', re.DOTALL)
FIELD_PATTERN = re.compile(r'^(\w+):\s*(.*)$')

now = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S UTC')

def parse_frontmatter(text: str):
    m = FRONTMATTER_PATTERN.match(text)
    if not m:
        return {}
    block = m.group(1)
    data = {}
    current_key = None
    for line in block.split('\n'):
        if not line.strip():
            continue
        if re.match(r'^\s{2,}-', line) and current_key:
            # list item for current key - convert to list if currently scalar
            if current_key in data and not isinstance(data[current_key], list):
                data[current_key] = [data[current_key]]
            data.setdefault(current_key, []).append(line.strip().lstrip('-').strip())
            continue
        fm = FIELD_PATTERN.match(line)
        if fm:
            key, value = fm.group(1), fm.group(2).strip()
            current_key = key
            # simple scalar; lists handled above
            if value.startswith('[') and value.endswith(']'):
                # inline list
                items = [v.strip() for v in value[1:-1].split(',') if v.strip()]
                data[key] = items
            else:
                data[key] = value
        else:
            current_key = None
    return data

def scan_docs(root: pathlib.Path):
    docs = []
    for path in root.rglob('*.md'):
        # skip excluded dirs
        parts = set(p.name for p in path.relative_to(root).parents)
        if any(ex in parts for ex in EXCLUDE_DIRS):
            continue
        # skip index/README navigation files
        if path.name in ('00_INDEX.md', '01_README.md', '_index.md', 'README.md'):
            continue
        rel = path.relative_to(root)
        text = path.read_text(encoding='utf-8', errors='ignore')
        fm = parse_frontmatter(text)
        fm['file_path'] = str(rel)
        docs.append(fm)
    return docs

def validate_docs(docs):
    errors = []
    by_id = defaultdict(list)
    for d in docs:
        if not d.get('doc_id'):
            errors.append(f"Missing doc_id in {d.get('file_path')}")
        for f in REQUIRED_FIELDS:
            if not d.get(f):
                errors.append(f"Missing {f} in {d.get('file_path')} (doc_id={d.get('doc_id')})")
        if d.get('doc_id'):
            by_id[d['doc_id']].append(d['file_path'])
    for doc_id, paths in by_id.items():
        if len(paths) > 1:
            errors.append(f"Duplicate doc_id {doc_id}: {', '.join(paths)}")
    return errors

def build_index(docs):
    by_cat = defaultdict(list)
    for d in docs:
        if d.get('doc_id'):
            by_cat[d.get('category','Uncategorized')].append(d)
    for cat_docs in by_cat.values():
        cat_docs.sort(key=lambda x: x['doc_id'])
    lines = ["---", "# Auto-generated documentation index", f"# Generated: {now}", "---", "", "# Documentation Index", ""]
    for cat in sorted(by_cat.keys()):
        lines.append(f"## {cat}")
        for d in by_cat[cat]:
            lines.append(f"- [{d['doc_id']} {d.get('version','')}]({d['file_path']}) - {d.get('title','')}")
        lines.append("")
    return '\n'.join(lines)

def main():
    docs = scan_docs(DOC_ROOT)
    errors = validate_docs(docs)
    # Write metadata JSON regardless (may aid debugging)
    meta_path = DOC_ROOT / '_metadata.json'
    with meta_path.open('w', encoding='utf-8') as f:
        json.dump({'generated': now, 'documents': docs}, f, indent=2)
    index_content = build_index(docs)
    (DOC_ROOT / '00_INDEX.md').write_text(index_content, encoding='utf-8')
    if errors:
        sys.stderr.write('\n'.join(errors) + '\n')
        sys.stderr.write(f"Validation failed with {len(errors)} error(s).\n")
        sys.exit(1)
    print(f"Documentation index regenerated. {len(docs)} documents processed. No errors.")

if __name__ == '__main__':
    main()
