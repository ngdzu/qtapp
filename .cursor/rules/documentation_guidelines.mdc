---
alwaysApply: true
appliesTo:
  - "project-dashboard/doc/**/*.md"
  - "project-dashboard/doc/**/*.mmd"
  - "project-dashboard/ZTODO.md"
  - "**/*requirements*.md"
  - "**/*design*.md"
  - "**/*architecture*.md"
  - "**/*database*.md"
---
# Documentation Maintenance Guidelines

When making changes to requirements, design, database schema, architecture, or any system components, you **must** update the related documentation to keep it synchronized with the codebase.

## Required Updates Checklist

When modifying any of the following, update the corresponding documentation:

### Requirements & Design Changes
- [ ] **Architecture Documents** (`project-dashboard/doc/02_ARCHITECTURE.md`)
  - Update component descriptions
  - Update data flow descriptions
  - Update system interactions

- [ ] **Class Design Documents** (`project-dashboard/doc/09_CLASS_DESIGNS.md`)
  - Update class responsibilities
  - Update method signatures
  - Update properties and signals
  - Update dependencies

- [ ] **System Components Reference** (`project-dashboard/doc/29_SYSTEM_COMPONENTS.md`)
  - Update component inventory when adding/removing components
  - Update component interaction diagram (`29_SYSTEM_COMPONENTS.mmd`)
  - Regenerate SVG: `npx @mermaid-js/mermaid-cli -i doc/29_SYSTEM_COMPONENTS.mmd -o doc/29_SYSTEM_COMPONENTS.svg`
  - Update component count summary
  - Ensure alignment with architecture and class design docs

- [ ] **Thread Model** (`project-dashboard/doc/12_THREAD_MODEL.md`)
  - Update service-to-thread mapping when adding/removing components
  - Update thread topology diagram (`12_THREAD_MODEL.mmd`)
  - Regenerate SVG: `npx @mermaid-js/mermaid-cli -i doc/12_THREAD_MODEL.mmd -o doc/12_THREAD_MODEL.svg`
  - Update component count per thread
  - Ensure all 98 components are accounted for

- [ ] **Database Design** (`project-dashboard/doc/10_DATABASE_DESIGN.md`)
  - Update table schemas (DDL)
  - Update indices
  - Update relationships
  - Update retention/archival policies

- [ ] **Interface Documentation** (`project-dashboard/doc/interfaces/*.md`)
  - Update interface methods
  - Update responsibilities
  - Update error semantics
  - Update example code paths

- [ ] **Definitions** (`project-dashboard/doc/08_DEFINITIONS.md`)
  - Add new terms
  - Update existing definitions
  - Ensure alphabetical organization

### Diagram Updates
- [ ] **Mermaid Diagrams** (`project-dashboard/doc/**/*.mmd`)
  - Update sequence diagrams for workflow changes
  - Update class diagrams for new classes/methods
  - Update architecture diagrams for component changes
  - **Regenerate SVG files** after editing MMD files (see `mermaid_guidelines.mdc`)

- [ ] **Architecture Diagrams** (`project-dashboard/doc/02_ARCHITECTURE.mmd`)
  - Update component relationships
  - Add new components
  - Update data flow

### Task Tracking
- [ ] **ZTODO.md**
  - Add new tasks for new features
  - Update existing tasks with implementation details
  - Mark completed tasks
  - Add notes about design decisions
  - Reference related documentation

### Security & Configuration
- [ ] **Security Documentation** (`project-dashboard/doc/06_SECURITY.md`)
  - Update security architecture
  - Update authentication mechanisms
  - Update certificate management
  - Update encryption details

- [ ] **Setup Guides** (`project-dashboard/doc/07_SETUP_GUIDE.md`)
  - Update build instructions
  - Update configuration steps
  - Update prerequisites

### UI/UX Changes
- [ ] **UI/UX Guide** (`project-dashboard/doc/03_UI_UX_GUIDE.md`)
  - Update component specifications
  - Update interaction patterns
  - Update layout descriptions

## Common Change Scenarios

### Adding a New Feature
1. Update architecture document with new component
2. Update class designs with new classes
3. Update database design if new tables/columns needed
4. Update interface documentation if new interfaces
5. Update definitions with new terms
6. Add tasks to ZTODO.md
7. Update/create diagrams (MMD + SVG)
8. Update UI/UX guide if UI changes

### Modifying Database Schema
1. Update `10_DATABASE_DESIGN.md` with new DDL
2. Update class designs if DatabaseManager changes
3. Add migration notes
4. Update ZTODO.md with migration tasks
5. Update architecture if schema affects data flow

### Adding Configuration Options
1. Update `SettingsManager` in class designs
2. Update `SettingsController` in class designs
3. Update database design (settings table)
4. Update UI/UX guide (Settings View section)
5. Update definitions with new setting terms
6. Update ZTODO.md with implementation tasks

### Changing Security Architecture
1. Update `06_SECURITY.md` comprehensively
2. Update interface documentation (ITelemetryServer, etc.)
3. Update class designs (NetworkManager, CertificateManager)
4. Update database design (certificates, security_audit_log tables)
5. Update certificate provisioning guide if needed
6. Update diagrams for security workflows
7. Update ZTODO.md with security implementation tasks

### Adding New Interfaces
1. Create interface documentation in `doc/interfaces/`
2. Update architecture document
3. Update class designs (implementations)
4. Update ZTODO.md interface definition tasks
5. Add to interface list in relevant docs

## Diagram Maintenance

When updating Mermaid diagrams:
1. Edit the `.mmd` file
2. Regenerate the `.svg` file using:
   ```bash
   npx @mermaid-js/mermaid-cli -i <path/to/diagram>.mmd -o <path/to/diagram>.svg
   ```
3. Commit both `.mmd` and `.svg` files
4. Verify SVG renders correctly (no syntax errors)

See `mermaid_guidelines.mdc` for detailed Mermaid diagram guidelines.

## Documentation Cross-References

Ensure documentation cross-references are updated:
- Links between related documents
- References to diagrams (both MMD and SVG)
- References to interface documentation
- References to ZTODO.md tasks
- References to code files/paths

## Review Checklist

Before marking documentation updates complete:
- [ ] All related documents updated
- [ ] Diagrams regenerated (MMD → SVG)
- [ ] ZTODO.md tasks added/updated
- [ ] Cross-references verified
- [ ] Definitions updated
- [ ] No broken links
- [ ] Consistent terminology across all docs

## Best Practices

1. **Update documentation as you code** - Don't leave it for later
2. **Keep diagrams synchronized** - Update MMD files and regenerate SVGs
3. **Use consistent terminology** - Check definitions document
4. **Reference related docs** - Link to related sections
5. **Update ZTODO.md proactively** - Add tasks before implementation
6. **Verify diagram syntax** - Always regenerate SVG to catch errors
7. **Review cross-references** - Ensure links still work after changes

## Related Guidelines

- `mermaid_guidelines.mdc` - Mermaid diagram creation and maintenance
- `quality_checklist.mdc` - Quality standards for documentation
- `maintenance_notes.mdc` - General maintenance procedures

# Maintenance Notes

- Keep `table_of_contents.md` in sync with lesson changes.
- Rebuild base images when Qt dependencies update:
  - `docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .`
  - `docker build --target qt-runtime -t qtapp-qt-runtime:latest .`
- Regularly test lesson builds to ensure compatibility.
- Document deviations from standard patterns in the lesson README.
- When editing Mermaid diagrams (`*.mmd`), render to SVG and verify there are no parser errors:
  ```bash
  npx @mermaid-js/mermaid-cli -i doc/12_THREAD_MODEL.mmd -o doc/12_THREAD_MODEL.svg
  ```
