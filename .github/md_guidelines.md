---
appliesTo:
  - "**/*.md"
  - "**/*.mdx"
---
# Markdown Guidelines

## Overview

This document defines the markdown coding standards and best practices for documentation in the Z Monitor project. These guidelines ensure consistent, readable, and maintainable documentation that renders correctly across different markdown processors.

**Key Principles:**
- **Valid Syntax** - All markdown must be valid and render correctly
- **Readability First** - Format for human readability, not just rendering
- **Consistency** - Use consistent formatting patterns throughout
- **Accessibility** - Make documents navigable with proper links and structure
- **Maintainability** - Write markdown that's easy to update and maintain

---

## 1. Syntax Validation

### Valid Markdown Syntax

- **Verify headers, lists, code fences, and tables render correctly**
- **Test complex sections** - Preview files (e.g., VS Code Markdown preview) before committing
- **Check for syntax errors** - Ensure all markdown elements are properly closed

### Common Syntax Issues

- **Unclosed code fences**: Every ` ``` ` must have a matching closing ` ``` `
- **Mismatched headers**: Use consistent header levels (don't skip levels)
- **Broken lists**: Ensure proper indentation and list markers
- **Invalid links**: Verify all links resolve correctly

---

## 2. Headers

### Header Levels

- **Use hierarchical structure**: Don't skip header levels (e.g., don't go from `##` to `####`)
- **Consistent formatting**: Use ATX-style headers (`# Header`) not Setext-style (`===`)
- **Descriptive titles**: Headers should clearly describe the section content

```markdown
# ✅ Good: Hierarchical structure
# Main Title
## Section
### Subsection
## Another Section

# ❌ Bad: Skipping levels
# Main Title
#### Subsection (skipped ## and ###)
```

### Header Best Practices

- **One H1 per document**: Use `#` for document title only
- **Use H2 for major sections**: `##` for main sections
- **Use H3+ for subsections**: `###` for subsections within sections
- **Table of contents**: Consider adding TOC for long documents

---

## 3. Code Blocks

### Fenced Code Blocks

- **Always use fenced code blocks** with language identifiers:
  ```markdown
  ```cpp
  // Code here
  ```
  
  ```bash
  # Commands here
  ```
  ```

- **Language identifiers**: Use appropriate language tags (`cpp`, `bash`, `python`, `cmake`, `sql`, `qml`, `yaml`, `json`, etc.)

### Code Block Best Practices

- **No comments in shell/bash code blocks** - Comments make it difficult to copy and paste commands
  ```markdown
  # ❌ Bad: Comments in code block
  ```bash
  docker build -t myapp .  # Build the image
  docker run myapp  # Run the container
  ```
  
  # ✅ Good: Explanatory text outside code block
  Build the Docker image:
  
  ```bash
  docker build -t myapp .
  ```
  
  Then run the container:
  
  ```bash
  docker run myapp
  ```
  ```

- **Ensure code samples compile or type-check**: Verify code examples are correct
- **Include context**: Add brief explanation before code blocks when needed
- **Keep code blocks focused**: Show only relevant code, not entire files

### Inline Code

- **Use backticks for inline code**: `` `variableName` ``, `` `function()` ``
- **Use for**: File names, function names, variable names, commands, technical terms
- **Don't use for**: Emphasis (use `*italic*` or `**bold**` instead)

---

## 4. Tables

### Table Syntax

- **Ensure each row has the same number of `|` separators**
- **Include alignment row**: Use `| --- |` or `| :--- |` or `| :---: |` or `| ---: |`
- **Leave blank lines** before and after tables for readability

```markdown
# ✅ Good: Properly formatted table
| Column 1 | Column 2 | Column 3 |
|----------|----------|-----------|
| Value 1  | Value 2  | Value 3   |
| Value 4  | Value 5  | Value 6   |

# ❌ Bad: Missing alignment row
| Column 1 | Column 2 |
| Value 1  | Value 2  |

# ❌ Bad: Inconsistent separators
| Column 1 | Column 2 | Column 3 |
| Value 1  | Value 2  |  (missing separator)
```

### Table Best Practices

- **Avoid trailing whitespace** inside table cells
- **Align columns consistently**: Use alignment markers when helpful
- **Keep tables simple**: Complex tables may not render well
- **Use tables for structured data**: Not for layout (use HTML/CSS if needed)

### Table Alignment

```markdown
# Left-aligned (default)
| Column | Column |
|--------|--------|
| Value  | Value  |

# Center-aligned
| Column | Column |
|:------:|:------:|
| Value  | Value  |

# Right-aligned
| Column | Column |
|-------:|------:|
| Value  | Value  |

# Mixed alignment
| Left | Center | Right |
|:-----|:------:|------:|
| A    | B      | C     |
```

---

## 5. Links

### Markdown Link Syntax

- **Always use markdown links for file references** - Makes documents navigable
  ```markdown
  # ✅ Good: Markdown link
  See [Sensor Integration Summary](../z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md) for details.
  
  # ❌ Bad: Plain text path
  See doc/z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md for details.
  ```

- **Use descriptive link text**: Link text should describe the destination
  ```markdown
  # ✅ Good: Descriptive link text
  [Database Design Documentation](10_DATABASE_DESIGN.md)
  
  # ❌ Bad: Generic link text
  [Click here](10_DATABASE_DESIGN.md)
  ```

### Link Types

- **Internal links**: Use relative paths for files in the same repository
  ```markdown
  [Architecture Overview](./02_ARCHITECTURE.md)
  [Database Design](../architecture_and_design/10_DATABASE_DESIGN.md)
  ```

- **External links**: Use absolute URLs with descriptive text
  ```markdown
  [Qt Documentation](https://doc.qt.io/qt-6/)
  [C++ Reference](https://en.cppreference.com/)
  ```

- **Anchor links**: Link to sections within documents
  ```markdown
  [Section Name](#section-name)
  [Database Schema](#database-schema)
  ```

### Link Best Practices

- **Verify links work**: Check that all links resolve correctly
- **Use relative paths**: Prefer relative paths for internal links
- **Update links when moving files**: Keep links synchronized with file structure
- **Link to specific sections**: When referencing long documents, link to relevant sections

---

## 6. Lists

### Unordered Lists

- **Use consistent markers**: Use `-` (hyphen) consistently
- **Proper indentation**: Use 2 or 4 spaces for nested lists
- **Blank lines**: Leave blank lines before and after lists when appropriate

```markdown
# ✅ Good: Consistent formatting
- Item 1
- Item 2
  - Nested item 1
  - Nested item 2
- Item 3

# ❌ Bad: Inconsistent markers
- Item 1
* Item 2
+ Item 3
```

### Ordered Lists

- **Use numbers**: `1.`, `2.`, `3.`, etc.
- **Numbers don't matter**: Markdown will auto-number, but use sequential numbers for readability
- **Nested lists**: Indent properly for nested ordered lists

```markdown
# ✅ Good: Properly formatted ordered list
1. First step
2. Second step
   1. Sub-step 1
   2. Sub-step 2
3. Third step
```

### Task Lists

- **Use for checklists**: `- [ ]` for unchecked, `- [x]` for checked
- **Common in ZTODO.md**: Use for tracking task completion

```markdown
# ✅ Good: Task list
- [ ] Task 1
- [x] Completed task
- [ ] Task 3
```

---

## 7. Emphasis and Formatting

### Bold and Italic

- **Bold**: Use `**text**` or `__text__` (prefer `**text**`)
- **Italic**: Use `*text*` or `_text_` (prefer `*text*`)
- **Bold italic**: Use `***text***`

```markdown
# ✅ Good: Emphasis
This is **important** text.
This is *emphasized* text.
This is ***very important*** text.
```

### Strikethrough

- **Use `~~text~~`** for strikethrough (when showing deprecated content)

```markdown
~~Deprecated feature~~ (use new feature instead)
```

### Blockquotes

- **Use `>` for blockquotes**: Indicate quotes, notes, or callouts

```markdown
> **Note:** This is an important note.
> 
> It can span multiple lines.
```

---

## 8. Horizontal Rules

- **Use `---` or `***`** for horizontal rules (section separators)
- **Leave blank lines** before and after horizontal rules

```markdown
Section 1 content

---

Section 2 content
```

---

## 9. Images

### Image Syntax

- **Use markdown image syntax**: `![alt text](path/to/image.png)`
- **Always include alt text**: For accessibility
- **Use relative paths**: For images in the repository

```markdown
# ✅ Good: Image with alt text
![Architecture Diagram](diagrams/architecture.svg)

# ❌ Bad: Missing alt text
![](diagrams/architecture.svg)
```

### Image Best Practices

- **Use SVG for diagrams**: Scalable, text-based format
- **Optimize images**: Keep file sizes reasonable
- **Descriptive alt text**: Alt text should describe the image content
- **Reference images**: Link to images in documentation directories

---

## 10. Special Sections

### Callouts and Admonitions

- **Use blockquotes for callouts**: Format important notes, warnings, tips

```markdown
> **⚠️ Warning:** This operation cannot be undone.

> **💡 Tip:** Use this pattern for better performance.

> **📝 Note:** This feature requires Qt 6.0 or later.
```

### Code References

- **Use code references for existing code**: When referencing code in the codebase
  ```markdown
  See `DatabaseManager::open()` method in [DatabaseManager.h](../src/infrastructure/persistence/DatabaseManager.h)
  ```

---

## 11. File Structure

### Document Headers

- **Include metadata** (if using frontmatter):
  ```markdown
  ---
  title: Document Title
  version: 1.0
  last_updated: 2025-01-15
  ---
  ```

### Table of Contents

- **Add TOC for long documents**: Help readers navigate
  ```markdown
  ## Table of Contents
  
  - [Section 1](#section-1)
  - [Section 2](#section-2)
    - [Subsection 2.1](#subsection-21)
  ```

### Document Organization

- **Clear structure**: Use consistent section organization
- **Logical flow**: Organize content in a logical order
- **Cross-references**: Link to related documents and sections

---

## 12. Best Practices

### Readability

- **Line length**: Keep lines under 100 characters when possible (not strict, but helpful)
- **Blank lines**: Use blank lines to separate sections and improve readability
- **Consistent formatting**: Use consistent patterns throughout documents

### Writing Style

- **Clear and concise**: Write clearly and concisely
- **Active voice**: Prefer active voice when possible
- **Technical accuracy**: Ensure technical content is accurate
- **Examples**: Include examples to illustrate concepts

### Maintenance

- **Update links**: Keep links synchronized with file structure
- **Review regularly**: Review and update documentation as code changes
- **Version information**: Include version/date information when relevant

---

## 13. Common Mistakes to Avoid

### ❌ Don't Include Comments in Code Blocks

```markdown
# ❌ Bad: Comments in shell code block
```bash
docker build -t myapp .  # Build the image
```

# ✅ Good: Explanatory text outside
Build the Docker image:

```bash
docker build -t myapp .
```
```

### ❌ Don't Use Plain Text Paths

```markdown
# ❌ Bad: Plain text path
See doc/10_DATABASE_DESIGN.md for details.

# ✅ Good: Markdown link
See [Database Design](doc/10_DATABASE_DESIGN.md) for details.
```

### ❌ Don't Skip Header Levels

```markdown
# ❌ Bad: Skipping levels
# Title
#### Subsection (skipped ## and ###)

# ✅ Good: Hierarchical structure
# Title
## Section
### Subsection
```

### ❌ Don't Create Invalid Tables

```markdown
# ❌ Bad: Missing alignment row, inconsistent separators
| Col1 | Col2 |
| Val1 | Val2 |

# ✅ Good: Proper table format
| Col1 | Col2 |
|------|------|
| Val1 | Val2 |
```

### ❌ Don't Use Code Blocks for Emphasis

```markdown
# ❌ Bad: Using code for emphasis
This is `very important` text.

# ✅ Good: Use bold for emphasis
This is **very important** text.
```

---

## 14. Validation and Tools

### Preview Before Committing

- **Use markdown preview**: Preview files in VS Code or other editors
- **Check rendering**: Verify tables, code blocks, and links render correctly
- **Test links**: Verify all links resolve correctly

### Linting Tools

- **Markdownlint**: Use markdownlint to check syntax
- **Link checkers**: Use tools to verify links are valid
- **Spell checkers**: Use spell checkers for documentation

### Common Validation Checks

- [ ] All code blocks have language identifiers
- [ ] All tables have alignment rows
- [ ] All links use markdown syntax (not plain text paths)
- [ ] No comments in shell/bash code blocks
- [ ] Headers follow hierarchical structure
- [ ] Images have alt text
- [ ] Lists use consistent formatting

---

## 15. Quick Reference

### Syntax Cheat Sheet

| Element | Syntax | Example |
|---------|--------|---------|
| **Header 1** | `# Header` | `# Main Title` |
| **Header 2** | `## Header` | `## Section` |
| **Bold** | `**text**` | `**important**` |
| **Italic** | `*text*` | `*emphasized*` |
| **Code** | `` `code` `` | `` `variableName` `` |
| **Code Block** | ` ```language` | ` ```cpp` |
| **Link** | `[text](url)` | `[Docs](doc.md)` |
| **Image** | `![alt](path)` | `![Diagram](img.svg)` |
| **List** | `- item` | `- Item 1` |
| **Table** | `\| Col \|` | `\| Col1 \| Col2 \|` |
| **Blockquote** | `> text` | `> Note: ...` |

### Common Patterns

| Pattern | ✅ Good | ❌ Bad |
|---------|---------|--------|
| **File Links** | `[Database Design](10_DATABASE_DESIGN.md)` | `doc/10_DATABASE_DESIGN.md` |
| **Code Blocks** | ` ```cpp` with language | ` ``` ` without language |
| **Shell Commands** | Explanatory text outside block | Comments inside block |
| **Tables** | Alignment row included | Missing alignment row |
| **Headers** | Hierarchical (`#`, `##`, `###`) | Skipping levels |

---

## 16. Related Guidelines

- **Documentation Guidelines**: See `.cursor/rules/documentation_guidelines.mdc` for documentation standards
- **API Documentation**: See `.cursor/rules/api_documentation.mdc` for API documentation requirements
- **Code Style**: See `.cursor/rules/cpp_guidelines.mdc` for C++ code examples in documentation

---

## Enforcement

- **Code Review**: All markdown files must follow these guidelines
- **Link Validation**: All links must resolve correctly
- **Syntax Validation**: All markdown must be valid
- **Preview Check**: Files should be previewed before committing

---

**Remember:** Markdown is meant to be readable in both source and rendered form. Write for humans first, rendering second. Keep formatting consistent, links navigable, and code blocks executable.
