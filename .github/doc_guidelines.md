---
alwaysApply: true
---

# Documentation Guidelines

## Overview

This document provides comprehensive guidelines for creating, maintaining, and organizing documentation in the Z Monitor project. Following these guidelines ensures documentation is discoverable, maintainable, and optimized for both AI agents and human readers.

**Related Documents:**
- `DOC_REORGANIZATION_PLAN.md` - Complete documentation reorganization strategy
- `.github/ztodo_task_guidelines.md` - Task creation guidelines
- `.github/ztodo_verification.md` - Verification workflow

---

## Critical Rules

1. **Every document MUST have a DOC-ID** - Use format: `DOC-{CATEGORY}-{NUMBER}`
2. **Every document MUST have metadata** - Complete YAML frontmatter required
3. **Every document MUST follow template** - Use category-specific template
4. **Every document MUST have owner** - Designated maintainer identified
5. **Every document MUST link diagrams** - Co-locate diagrams in `diagrams/` subdirectory
6. **Every document MUST cross-reference** - Link to related docs, tasks, requirements
7. **Every document MUST be versioned** - Semantic versioning (vMAJOR.MINOR)
8. **Every document MUST be reviewed** - Follow approval workflow

---

## Document ID Format

**Format:** `DOC-{CATEGORY}-{NUMBER}`

**Categories:**
- `ARCH` - Architecture & System Design
- `REQ` - Requirements & Specifications  
- `API` - API Documentation & Interface Contracts
- `COMP` - Component/Module Specifications
- `PROC` - Processes & Workflows
- `GUIDE` - Guidelines & Best Practices
- `REF` - Reference Materials & Lookups
- `TRAIN` - Training & Onboarding
- `REG` - Regulatory & Compliance

**Number:** Zero-padded 3-digit sequential number within category (001, 002, ..., 999)

**Examples:**
- `DOC-ARCH-001` - System Architecture Overview
- `DOC-COMP-020` - Database Manager Component Specification
- `DOC-API-003` - IPatientRepository Interface Contract
- `DOC-PROC-020` - Certificate Provisioning Workflow
- `DOC-GUIDE-001` - C++ Coding Guidelines

**Version Format:** `vMAJOR.MINOR`
- Full ID with version: `DOC-COMP-020-v2.1`
- Major version (X.0): Breaking changes, major rewrites
- Minor version (X.Y): Additive changes, clarifications

---

## Metadata Standard (YAML Frontmatter)

**Every document MUST start with YAML frontmatter:**

```yaml
---
doc_id: DOC-COMP-020
title: Database Manager Component Specification
version: v2.1
category: Component
subcategory: Infrastructure/Database
status: Approved
owner: Database Team
reviewers: 
  - Architecture Team
  - Security Team
last_reviewed: 2025-12-01
next_review: 2026-03-01
related_docs:
  - DOC-API-001  # IDatabaseManager Interface
  - DOC-COMP-021 # Schema Management
  - DOC-COMP-022 # Query Registry
  - DOC-ARCH-005 # Data Flow Architecture
related_tasks:
  - TASK-DB-001  # SQLCipher Integration
  - TASK-INFRA-017 # Qt Plugin Configuration
related_requirements:
  - REQ-DATA-001 # Encryption at Rest
  - REQ-PERF-005 # Database Performance
tags:
  - database
  - infrastructure
  - persistence
  - sqlite
  - encryption
diagram_files:
  - DOC-COMP-020_database_er_diagram.mmd
  - DOC-COMP-020_database_er_diagram.svg
---
```

### Metadata Fields

**Required Fields:**
- `doc_id` - Document identifier (DOC-{CATEGORY}-{NUMBER})
- `title` - Full document title
- `version` - Semantic version (vMAJOR.MINOR)
- `category` - Document category (Architecture, Component, API, etc.)
- `status` - Approval status (Draft, In Review, Approved, Published, Deprecated)
- `owner` - Document owner/maintainer

**Recommended Fields:**
- `subcategory` - Further categorization (e.g., Infrastructure/Database)
- `reviewers` - List of designated reviewers
- `last_reviewed` - Date of last review (YYYY-MM-DD)
- `next_review` - Scheduled next review date
- `related_docs` - List of related DOC-IDs with brief comments
- `related_tasks` - List of TASK-IDs that implement this doc
- `related_requirements` - List of REQ-IDs this doc satisfies
- `tags` - Searchable keywords
- `diagram_files` - List of diagram files (source and rendered)

**Status Values:**
- `Draft` - Work in progress, not reviewed
- `In Review` - Submitted for review, awaiting feedback
- `Approved` - Reviewed and approved by reviewers
- `Published` - Approved and officially published
- `Deprecated` - No longer current, superseded

---

## See Complete Templates

For complete document templates and all remaining guidelines, see:
- **Component Template:** `doc/templates/component_template.md`
- **API Template:** `doc/templates/api_template.md`
- **Process Template:** `doc/templates/process_template.md`
- **Guideline Template:** `doc/templates/guideline_template.md`

---

## Diagram Guidelines

### Naming Convention

**Format:** `{DOC-ID}_{diagram_purpose}.{ext}`

**Extensions:**
- `.mmd` - Mermaid source file (version controlled)
- `.svg` - Rendered SVG (version controlled)

**Examples:**
- `DOC-COMP-020_database_er_diagram.mmd`
- `DOC-COMP-020_database_er_diagram.svg`

### Storage Location

**Rule:** Always co-locate diagrams with documentation in `diagrams/` subdirectory

**Structure:**
```
components/infrastructure/database/
├── DOC-COMP-020_database_manager.md
├── DOC-COMP-021_schema_management.md
└── diagrams/
    ├── DOC-COMP-020_database_er_diagram.mmd
    ├── DOC-COMP-020_database_er_diagram.svg
    ├── DOC-COMP-021_migration_flow.mmd
    └── DOC-COMP-021_migration_flow.svg
```

### Diagram Workflow

1. **Create/Edit:** Edit `.mmd` (Mermaid) source file
2. **Render:** Run `scripts/render-mermaid.sh {file.mmd}` to generate SVG
3. **Commit:** Commit both `.mmd` (source) and `.svg` (rendered)
4. **Reference:** Link in markdown: `![Diagram](diagrams/{DOC-ID}_{name}.svg)`
5. **Update:** Always update `.mmd` first, then regenerate `.svg`

---

## Cross-Referencing

### Linking to Other Documents

**Always use relative paths and include DOC-ID in link text:**

```markdown
See [DOC-ARCH-001: System Overview](../architecture/DOC-ARCH-001_system_overview.md) for context.

Related components:
- [DOC-COMP-020: Database Manager](../components/infrastructure/database/DOC-COMP-020_database_manager.md)
```

### Linking to Tasks

```markdown
**Implementation Status:**
- TASK-DB-001: SQLCipher Integration - ✅ Complete
- TASK-DB-002: Schema Migration - 🔄 In Progress

See [TASK-DB-001](../../project-dashboard/ZTODO.md#task-db-001) in ZTODO.md.
```

---

## Document Approval Workflow

### Approval States

1. **Draft** - Work in progress
2. **In Review** - Awaiting reviewer feedback
3. **Approved** - Reviewed and approved
4. **Published** - Officially published
5. **Deprecated** - Superseded by newer version

### Reviewer Requirements

**DOC-ARCH (Architecture):**
- Architecture Team (mandatory)
- Security Team (if security-related)

**DOC-COMP (Components):**
- Component Owner (mandatory)
- Architecture Team (recommended)

**DOC-API (API Contracts):**
- API Owner (mandatory)
- All Client Teams (mandatory)

**DOC-PROC (Processes):**
- Process Owner (mandatory)
- Affected Teams (mandatory)

**DOC-GUIDE (Guidelines):**
- Tech Lead (mandatory)

**DOC-REG (Regulatory):**
- Compliance Officer (mandatory)
- Legal Team (mandatory)

---

## Best Practices

### Writing Style

**Be Clear and Concise:**
- ✅ "DatabaseManager handles all database operations"
- ❌ "The DatabaseManager class is the component that is responsible for managing..."

**Use Active Voice:**
- ✅ "The service validates input before processing"
- ❌ "Input is validated before it is processed"

**Be Specific:**
- ✅ "Latency must be < 50ms (p99)"
- ❌ "Latency should be fast"

**Use Examples:**
- Always include code examples for APIs
- Show both success and error cases

---

## Tools

### Document Creation

```bash
./scripts/create-doc.sh --category COMP --title "Database Manager"
```

### Index Generation

```bash
./scripts/generate-doc-index.py
```

### Validation

```bash
./scripts/validate-docs.sh
```

### Diagram Rendering

```bash
./scripts/render-mermaid.sh doc/components/diagrams/DOC-COMP-020_diagram.mmd
```

---

## Checklist for New Documents

- [ ] Document has unique DOC-ID
- [ ] Metadata frontmatter is complete
- [ ] Title is clear and specific
- [ ] Owner is designated
- [ ] Related docs/tasks/requirements are linked
- [ ] Tags are comprehensive
- [ ] Template sections are filled
- [ ] Code examples compile and run
- [ ] Diagrams are created and rendered
- [ ] Cross-references use relative paths
- [ ] Changelog has initial entry
- [ ] File is in correct directory

---

**Document ID:** DOC-GUIDE-004  
**Version:** v1.0  
**Status:** Published  
**Owner:** Documentation Team  
**Last Updated:** 2025-12-01
