# GitHub Copilot Instructions for Z Monitor Project

This document consolidates all coding standards, guidelines, and workflows for the Z Monitor project. Follow these instructions when working with this codebase.

---

# ZTODO Task Execution Workflow

## When User Says "Do the Next Task" or "Perform the Next Task"

Follow this exact workflow:

### Step 1: Find the Next Uncompleted Task

1. **Read `project-dashboard/ZTODO.md`** and find the first task marked with `- [ ]` (unchecked checkbox)
2. **Skip tasks marked with `- [x]`** (completed tasks)
3. **Respect task ordering:**
   - "Sequential Tasks" must be done in order
   - "Parallel Tasks" can be done concurrently
   - Check for dependencies mentioned in task descriptions

### Step 2: Understand the Task

1. **Read the complete task description**, including:
   - **What:** What needs to be implemented
   - **Why:** Rationale and requirements
   - **Files:** All files that need to be created/modified
   - **Acceptance:** Acceptance criteria
   - **Verification Steps:** All verification requirements (5 categories: Functional, Code Quality, Documentation, Integration, Tests)
   - **Dependencies:** Any prerequisites
   - **Documentation:** Related documentation files to read

2. **Read related documentation** mentioned in the task:
   - Design documents (e.g., `doc/33_SCHEMA_MANAGEMENT.md`)
   - Architecture documents (e.g., `doc/02_ARCHITECTURE.md`)
   - Interface documentation (e.g., `doc/interfaces/*.md`)
   - Any other referenced docs

3. **Read related code files** to understand context:
   - Existing implementations mentioned in the task
   - Similar patterns in the codebase
   - Dependencies and interfaces

### Step 3: Execute the Task

1. **Implement all requirements** from the "What" section
2. **Follow all coding guidelines:**
   - Doxygen comments for all public APIs (see `api_documentation.mdc`)
   - No hardcoded values (see `cpp_guidelines.mdc` - Constants and Magic Numbers section)
   - Error handling with `Result<T, Error>` (see `cpp_guidelines.mdc`)
   - DDD layer separation (domain, application, infrastructure, interface)
   - Code style guidelines (see `cpp_guidelines.mdc`)

3. **Complete all subtasks** if the task has sub-items:
   - Check off each subtask as you complete it: `- [x]`
   - Don't mark the main task complete until ALL subtasks are done

4. **Update related files** mentioned in the "Files" section:
   - Create new files if needed
   - Modify existing files
   - Update CMakeLists.txt if adding new sources
   - Update documentation if required

### Step 4: Perform Verification (CRITICAL)

**⚠️ NEVER mark a task complete without performing ALL verification steps.**

Follow the verification workflow from `ztodo_verification.mdc`. For each verification step:

1. **Functional Verification:**
   - Test that the feature works as specified
   - Verify all requirements from "Why" section are met
   - Handle edge cases and error conditions
   - Update status: `**Status:** ✅ Verified - [details]`

2. **Code Quality Verification:**
   - Run linter checks (`clang-format`, `clang-tidy`)
   - Check for compiler warnings
   - Verify Doxygen comments are present
   - Ensure no hardcoded values (grep verification)
   - Update status: `**Status:** ✅ Verified - [details]`

3. **Documentation Verification:**
   - Update all related design documents
   - Update architecture diagrams if needed
   - Regenerate Mermaid diagrams (MMD → SVG) if modified
   - Update cross-references
   - Update status: `**Status:** ✅ Verified - [details]`

4. **Integration Verification:**
   - Verify code builds successfully
   - Check that all tests pass
   - Verify CI/CD integration (if applicable)
   - Ensure no breaking changes (unless intentional)
   - Update status: `**Status:** ✅ Verified - [details]`

5. **Test Verification:**
   - Write unit tests if required
   - Write integration tests if required
   - Verify test coverage meets requirements
   - Update status: `**Status:** ✅ Verified - [details]`

### Step 5: Update ZTODO.md

1. **Update verification step statuses** in the task:
   - Change `**Status:** ⏳` to `**Status:** ✅ Verified - [details]`
   - Add specific verification results for each step

2. **Mark subtasks complete** as you finish them:
   - Change `- [ ]` to `- [x]` for completed subtasks

3. **Mark main task complete** ONLY when:
   - All subtasks are complete (`- [x]`)
   - All 5 verification steps are verified (`**Status:** ✅ Verified`)
   - All acceptance criteria are met
   - All related documentation is updated

4. **Update task status** in ZTODO.md:
   - Change `- [ ]` to `- [x]` for the main task
   - Add completion note if needed

### Step 6: Report Completion

Provide a summary of:
- What was implemented
- What was verified
- Any issues encountered
- Next steps or follow-up tasks

## Example Workflow

```
User: "Do the next task"

1. Read ZTODO.md → Find first `- [ ]` task
2. Read task description → Understand requirements
3. Read related docs → Understand context
4. Implement → Create/modify files
5. Verify → Run all 5 verification steps
6. Update ZTODO.md → Mark complete with verification status
7. Report → Summarize completion
```

## Important Rules

- **Never skip verification** - It's required, not optional
- **Never mark complete without verification** - All 5 steps must be verified
- **Read related docs first** - Understanding context prevents mistakes
- **Follow DDD principles** - Respect layer boundaries
- **Document as you code** - Doxygen comments are required
- **No hardcoded values** - Use constants or configuration
- **Error handling** - Use `Result<T, Error>` for operations that can fail
- **DRY Principle** - Eliminate code duplication (see `instructions/DRY.md`)
- **Never stop working on the task until it is done** - Complete all subtasks and verification steps before moving on
- **⚠️ NEVER commit code** - Do not execute git commit commands or suggest git commit commands. The user must be the one who commits. Never run `git add` and `git commit`.
- **⚠️ Loop Detection and Safety Stop:** If you detect that you are repeatedly doing the same action or making the same attempt without progress, **stop after 10 repetitions** and provide a summary:
  - **What has been done:** List all completed steps, files created/modified, and progress made
  - **What is getting stuck:** Identify the specific issue, error, or blocker preventing progress
  - **Why it's stuck:** Explain the root cause (e.g., missing dependency, configuration issue, unclear requirement)
  - **Next steps needed:** Suggest what needs to happen to unblock progress (user input, clarification, dependency resolution, etc.)
- **Build Performance:** Always parallelize CMake builds using all available CPU cores to maximize build speed:
  - **Linux:** `cmake --build <build-dir> -j $(nproc)`
  - **macOS:** `cmake --build <build-dir> -j $(sysctl -n hw.physicalcpu)`
  - **Windows:** `cmake --build <build-dir> -j %NUMBER_OF_PROCESSORS%`
  - Replace `<build-dir>` with the actual build directory (typically `build` or `.` for current directory) 

## Related Rules

The detailed rules are split into topic-specific files in `./` to reduce token usage. **GitHub Copilot does NOT automatically load these files** - they must be explicitly referenced or attached when needed.

### Core Guidelines
- `./cpp_guidelines.md` - C++ coding standards, error handling, constants, and best practices
- `./api_documentation.md` - Doxygen documentation requirements for all public APIs
- `./qml_guidelines.md` - QML coding standards and best practices
- `./md_guidelines.md` - Markdown documentation standards and formatting
- `./naming_conventions.md` - Naming conventions for classes, mocks, and components
- `./namespace_guidelines.md` - Namespace organization rules and conventions
- `./DRY.md` - DRY (Don't Repeat Yourself) principle and code duplication elimination

### Workflow and Process
- `./ztodo_verification.md` - Complete verification workflow (CRITICAL) - Required for all ZTODO tasks
- `./documentation_guidelines.md` - Documentation creation and maintenance standards
- `./mermaid_guidelines.md` - Diagram creation guidelines (Mermaid syntax and best practices)

### Project-Specific
- `./lesson_guidelines.md` - Lesson file requirements, structure, and quality standards
- `./docker_guidelines.md` - Docker build patterns, multi-stage builds, and best practices

### Best Practices for Token Usage with Copilot

**To minimize token usage and get relevant guidance:**

1. **Attach specific rule files** when working on related tasks:
   - Writing C++ code → Attach `./cpp_guidelines.md` and `./api_documentation.md`
   - Writing QML → Attach `./qml_guidelines.md`
   - Creating lessons → Attach `./lesson_guidelines.md`
   - Working with Docker → Attach `./docker_guidelines.md`
   - Writing documentation → Attach `./md_guidelines.md` and `./mermaid_guidelines.md`
   - Doing ZTODO tasks → Attach `./ztodo_verification.md`
   - Refactoring or code cleanup → Attach `./DRY.md`
   - Organizing namespaces → Attach `./namespace_guidelines.md`

2. **Use @workspace or @file mentions** to bring in specific guidelines:
   - `@file ./cpp_guidelines.md` when asking C++ questions
   - `@file ./api_documentation.md` when documenting APIs

3. **Only load what you need** - Don't attach all rules at once, only the relevant ones for your current task.

### Quick Reference: When to Use Which Rules

- **C++ Implementation**: `cpp_guidelines.md` + `api_documentation.md` + `naming_conventions.md` + `namespace_guidelines.md` + `DRY.md`
- **QML UI Work**: `qml_guidelines.md`
- **Documentation**: `md_guidelines.md` + `mermaid_guidelines.md` + `documentation_guidelines.md`
- **ZTODO Tasks**: `ztodo_verification.md` + task-specific guidelines
- **Lesson Creation**: `lesson_guidelines.md`
- **Docker/Deployment**: `docker_guidelines.md`
- **Code Refactoring/Cleanup**: `DRY.md` + `cpp_guidelines.md` + `namespace_guidelines.md`

### Note on Constants and Hardcoded Values
The "No Hardcoded Values" rule is covered in `./cpp_guidelines.md` (Section 4: Constants and Magic Numbers). Always use constants, configuration, or constants files instead of hardcoded strings, numbers, or paths in production code.

### Note on Code Duplication
The DRY (Don't Repeat Yourself) principle is covered in `./DRY.md`. Always identify and eliminate duplicated code by extracting it into reusable functions, classes, or modules.

### Note on Database Issues
For common database issues during development and testing, refer to `.github/instructions/database_fix.md` for troubleshooting steps, schema fixes, and migration solutions.