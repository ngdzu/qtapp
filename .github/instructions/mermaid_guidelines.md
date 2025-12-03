---
appliesTo:
  - "**/*.mmd"
  - "doc/**/*.mmd"
---
# Mermaid Diagram Guidelines

## Naming Convention

- **Diagram files MUST follow the order numbering convention** used by their parent document.
- If a document is named `NN_DOCUMENT_NAME.md`, its diagrams should be named `NN_DIAGRAM_DESCRIPTION.mmd`.
- **Example:**
  - Document: `30_DATABASE_ACCESS_STRATEGY.md`
  - Diagrams: `30_DATABASE_ACCESS_ARCHITECTURE.mmd`, `30_DATABASE_CONNECTION_STRATEGY.mmd`
- **Example:**
  - Document: `36_DATA_CACHING_STRATEGY.md`
  - Diagrams: `36_DATA_CACHING_COMPONENT.mmd`, `36_DATA_CACHING_PRIORITY.mmd`

## SVG Generation (Required)

- **Every `.mmd` file MUST have a corresponding `.svg` file.**
- After editing any Mermaid (`*.mmd`) file, **immediately regenerate the corresponding SVG** so reviewers can see the rendered output.
- Use the project's standard command (run from repo root):
  ```bash
  npx @mermaid-js/mermaid-cli -i <path/to/diagram>.mmd -o <path/to/diagram>.svg
  ```
- Replace `<path/to/diagram>` with the actual file you changed and check the CLI output for parsing errors.
- **Commit both the `.mmd` and updated `.svg` together** - they should always be in sync.
- **Successfully generating the SVG confirms that the Mermaid syntax is correct** - if the command completes without errors, the diagram syntax is valid.

## Documentation Synchronization (CRITICAL)

**When creating or editing any `.md` file that has a corresponding `.mmd` diagram file:**

1. **Check if diagram needs updates** - Review the `.md` file changes to see if they affect the diagram
2. **Update the `.mmd` file** - Revise the Mermaid diagram to reflect changes in the documentation
3. **Regenerate the `.svg` file** - Always regenerate the SVG after updating the Mermaid file
4. **Verify both files are in sync** - Ensure the diagram accurately represents the current documentation

**Examples:**
- Adding new classes to `09_CLASS_DESIGNS.md` → Update `09_CLASS_DESIGNS.mmd` → Regenerate `09_CLASS_DESIGNS.svg`
- Updating workflow in `38_AUTHENTICATION_WORKFLOW.md` → Update `38_AUTHENTICATION_WORKFLOW.mmd` → Regenerate `38_AUTHENTICATION_WORKFLOW.svg`
- Adding new components to `29_SYSTEM_COMPONENTS.md` → Update `29_SYSTEM_COMPONENTS.mmd` → Regenerate `29_SYSTEM_COMPONENTS.svg`

**Rule:** If you edit a `.md` file, you MUST also check and update its corresponding `.mmd` file (if it exists) and regenerate the `.svg` file.

## Quality Checklist

Before committing:
- [ ] `.mmd` file follows order numbering convention (e.g., `30_*`, `36_*`)
- [ ] `.svg` file exists with the same base name
- [ ] `.svg` file renders correctly (no syntax errors)
- [ ] Both `.mmd` and `.svg` are committed together
- [ ] Parent document references the diagram with correct relative paths
- [ ] **If `.md` file was edited, corresponding `.mmd` file was reviewed and updated if needed**
- [ ] **SVG file was regenerated after any `.mmd` file changes**
