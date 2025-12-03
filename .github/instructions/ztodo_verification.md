---
alwaysApply: true
---

# ZTODO Verification Workflow

## Overview

Every ZTODO action item must include verification steps to ensure work is complete and correct. This rule ensures that when working on ZTODO items, verification is always performed before marking tasks as complete.

## Critical Rule: Verify Before Completion

**NEVER mark a ZTODO item as complete without performing verification steps.**

Verification is a required part of every task completion, not optional.

## Verification Workflow

### 1. Before Starting Work

When beginning work on a ZTODO item:

1. **Read the full task description** - Understand all requirements
2. **Identify verification criteria** - Extract from "Acceptance" section
3. **Plan verification approach** - Determine how to verify each criterion
4. **Set up verification environment** - Ensure tools/tests are ready

### 2. During Implementation

While implementing:

1. **Write verification code** - Tests, checks, or manual verification steps
2. **Verify incrementally** - Don't wait until the end
3. **Document verification** - Note what was verified and how

### 3. Before Marking Complete

Before marking a ZTODO item as complete, perform ALL verification steps:

1. **Functional Verification** - Does it work as specified?
2. **Code Quality Verification** - Does it meet standards?
3. **Documentation Verification** - Is documentation updated?
4. **Integration Verification** - Does it integrate correctly?
5. **Test Verification** - Do all tests pass?
6. **Performance Verification** - Does it meet performance requirements?
7. **QML Verification** - (If applicable) Does QML code meet standards?

### 4. Verification Status Tracking

As you complete each verification step, update the task in ZTODO.md:

```markdown
- Verification Steps:
  1. Functional: ✅ Verified - All requirements met, edge cases handled
  2. Code Quality: ✅ Verified - No warnings, Doxygen complete, builds clean
  3. Documentation: ✅ Verified - Design docs updated, cross-refs validated
  4. Integration: ✅ Verified - All tests pass, no breaking changes
  5. Tests: ✅ Verified - 85% coverage, regression tests pass
  6. Performance: ✅ Verified - < 50ms latency, no regressions
```

This provides visibility into what has been verified and what remains.

### 5. Rollback Plan

Before implementing, prepare a rollback plan:

1. **Commit baseline** - Ensure clean git state before starting
2. **Document changes** - List all files modified/created
3. **If verification fails:**
   - Revert changes: `git checkout -- <files>` or `git reset --hard HEAD`
   - Document failure reason
   - Create issue for future fix
   - Do NOT mark task complete
4. **If partial completion:**
   - Keep working changes in separate branch
   - Document what works vs. what doesn't
   - Update task status to reflect partial completion

## Verification Categories

### Functional Verification

**What to Verify:**
- ✅ Feature works as specified in "What" section
- ✅ All requirements from "Why" section are met
- ✅ Edge cases are handled
- ✅ Error conditions are handled
- ✅ User-facing behavior matches specifications

**How to Verify:**
- Manual testing
- Automated tests
- Integration tests
- User acceptance testing
- Code review

**Example:**
```markdown
- [ ] Functional Verification:
  - [ ] Feature works as specified
  - [ ] All requirements met
  - [ ] Edge cases handled
  - [ ] Error conditions handled
```

### Code Quality Verification

**What to Verify:**
- ✅ Code follows style guidelines
- ✅ No linter errors (`clang-format`, `clang-tidy`, `qmllint` for QML)
- ✅ No compiler warnings (-Wall -Wextra -Werror)
- ✅ Code is properly documented (Doxygen comments)
- ✅ Follows architecture patterns (DDD layers respected)
- ✅ No memory leaks (AddressSanitizer, Valgrind)
- ✅ No security vulnerabilities (static analysis)
- ✅ Thread safety verified (ThreadSanitizer if concurrent)
- ✅ Error messages are clear and actionable
- ✅ CMakeLists.txt updated if new files/dependencies added
- ✅ **Affected targets build successfully**
- ✅ **Dependent targets build successfully**

**How to Verify:**
- Linter checks (`clang-format`, `clang-tidy`)
- Static analysis tools
- Code review
- Documentation generation
- Memory leak detection
- **Build verification script** (`./scripts/verify-build.sh <files...>`)

**Build Verification (REQUIRED):**

For every task that creates, modifies, or deletes files, you MUST verify that affected CMake targets build successfully:

1. **Identify Affected Files:** List all files created, modified, or deleted during the task
2. **Run Build Verification Script:** Execute `./scripts/verify-build.sh <file1> <file2> ...`
3. **Script Automatically:**
   - Finds the closest `CMakeLists.txt` for each file
   - Identifies which CMake targets include those files
   - Builds each affected target
   - Identifies all targets that depend on affected targets (transitive dependencies)
   - Builds all dependent targets
4. **Verify Success:** All builds must succeed with no errors

**Example Usage:**
```bash
# After implementing LocalPatientService
./scripts/verify-build.sh \
  src/infrastructure/patient/LocalPatientService.cpp \
  src/infrastructure/patient/LocalPatientService.h

# Output:
# ✓ Found CMakeLists.txt: src/infrastructure/CMakeLists.txt
# ✓ Affected target: z_monitor_infrastructure
# ✓ Building z_monitor_infrastructure... SUCCESS
# ✓ Dependent targets: z-monitor, z_monitor_tests
# ✓ Building z-monitor... SUCCESS
# ✓ Building z_monitor_tests... SUCCESS
# ✅ All affected targets built successfully
```

**When to Run:**
- After implementing new classes/files
- After modifying existing source files
- After updating CMakeLists.txt
- Before marking a task as complete
- As part of verification checklist

**Example:**
```markdown
- [ ] Code Quality Verification:
  - [ ] No linter errors
  - [ ] No compiler warnings
  - [ ] Doxygen comments added
  - [ ] Code follows style guidelines
  - [ ] No memory leaks detected
  - [ ] Affected targets build successfully (verify-build.sh)
  - [ ] Dependent targets build successfully (verify-build.sh)
```

### Documentation Verification

**What to Verify:**
- ✅ All relevant documentation updated
- ✅ API documentation generated (if applicable)
- ✅ Design documents updated
- ✅ README updated (if applicable)
- ✅ Code comments are clear and complete
- ✅ Cross-references are valid (no broken links)
- ✅ Examples are tested and correct
- ✅ Diagrams updated (Mermaid diagrams re-rendered)
- ✅ Version numbers updated (if applicable)

**How to Verify:**
- Review documentation files for completeness
- Generate and review API docs (Doxygen)
- Check all cross-references and internal links
- Run and verify all code examples
- Re-render Mermaid diagrams (`./scripts/render-mermaid.sh`)
- Spell-check documentation
- Verify markdown formatting

**Example:**
```markdown
- [ ] Documentation Verification:
  - [ ] Design docs updated (19_ADT_WORKFLOW.md, 02_ARCHITECTURE.md)
  - [ ] API docs generated and reviewed
  - [ ] README updated (if applicable)
  - [ ] Code comments complete (Doxygen)
  - [ ] Cross-references validated (no broken links)
  - [ ] Examples tested and correct
  - [ ] Mermaid diagrams re-rendered
```

### Integration Verification

**What to Verify:**
- ✅ Integrates with existing code
- ✅ No breaking changes (unless intentional)
- ✅ Dependencies are correct
- ✅ Build system updated
- ✅ Tests pass in CI/CD

**How to Verify:**
- Build and test locally
- Run CI/CD pipeline
- Integration tests
- Dependency checks
- Build system verification

**Example:**
```markdown
- [ ] Integration Verification:
  - [ ] Code builds successfully
  - [ ] All tests pass
  - [ ] CI/CD pipeline passes
  - [ ] No breaking changes
```

### Test Verification

**What to Verify:**
- ✅ Unit tests written and pass
- ✅ Integration tests written and pass
- ✅ Test coverage meets requirements
- ✅ Tests are meaningful and comprehensive
- ✅ Edge cases are tested
- ✅ Error conditions are tested
- ✅ Thread safety tested (if multi-threaded)
- ✅ Regression tests pass (existing features still work)

**How to Verify:**
- Run test suite (`./scripts/run_tests.sh all`)
- Check test coverage (`./scripts/run_tests.sh coverage`)
- Review test quality
- Verify test documentation
- Run regression test suite
- Run thread sanitizer for concurrent code

**Example:**
```markdown
- [ ] Test Verification:
  - [ ] Unit tests written and pass
  - [ ] Integration tests written and pass
  - [ ] Test coverage meets threshold (≥80% for new code)
  - [ ] Edge cases tested
  - [ ] Error conditions tested
  - [ ] Regression tests pass
  - [ ] Thread safety verified (if applicable)
```

### Performance Verification

**What to Verify:**
- ✅ Meets performance requirements (latency, throughput)
- ✅ No performance regressions
- ✅ Memory usage within acceptable limits
- ✅ CPU usage acceptable
- ✅ No blocking operations in critical paths
- ✅ Benchmarks pass (if applicable)

**How to Verify:**
- Run performance benchmarks
- Profile code (CPU, memory)
- Check latency measurements
- Compare with baseline performance
- Verify critical path performance (e.g., alarm detection < 50ms)

**Performance Targets (Z Monitor Specific):**
- Alarm detection: < 50ms from sensor to alarm raised
- UI update: < 250ms from data to screen
- Waveform rendering: 60 FPS (< 16ms per frame)
- Database queries: < 100ms for common queries
- Network transmission: < 1s for telemetry batch

**Example:**
```markdown
- [ ] Performance Verification:
  - [ ] Meets latency requirements
  - [ ] No performance regressions (benchmark comparison)
  - [ ] Memory usage acceptable
  - [ ] No blocking operations in critical paths
  - [ ] Benchmarks pass (if applicable)
```

### QML Verification (for UI tasks)

**What to Verify:**
- ✅ QML code follows style guidelines
- ✅ No qmllint errors or warnings
- ✅ No implicit types or unqualified access
- ✅ Property bindings are efficient (no binding loops)
- ✅ Component hierarchy is clean
- ✅ Accessibility labels present
- ✅ Responsive layout (adapts to screen sizes)

**How to Verify:**
- Run `qmllint` on all QML files
- Check for binding loops in debug output
- Test on different screen sizes
- Verify keyboard navigation
- Check accessibility with screen reader

**Example:**
```markdown
- [ ] QML Verification:
  - [ ] No qmllint errors
  - [ ] No implicit types
  - [ ] No binding loops
  - [ ] Accessibility labels present
  - [ ] Layout tested on different sizes
```

## Verification Checklist Template

When working on a ZTODO item, use this checklist:

```markdown
## Verification Checklist

### Functional Verification
- [ ] Feature works as specified in "What" section
- [ ] All requirements from "Why" section are met
- [ ] Edge cases are handled correctly
- [ ] Error conditions are handled gracefully
- [ ] User-facing behavior matches specifications

### Code Quality Verification
- [ ] Code follows style guidelines (see `.cursor/rules/code_style.mdc`)
- [ ] No linter errors (`clang-format`, `clang-tidy`)
- [ ] No compiler warnings
- [ ] All public APIs have Doxygen comments (see `.cursor/rules/api_documentation.mdc`)
- [ ] Code follows architecture patterns
- [ ] No memory leaks (verified with Valgrind/AddressSanitizer)
- [ ] No security vulnerabilities (static analysis)
- [ ] **Affected targets build successfully** (`./scripts/verify-build.sh <files...>`)
- [ ] **Dependent targets build successfully** (automatically checked by verify-build.sh)

### Documentation Verification
- [ ] All relevant design documents updated (see `.cursor/rules/documentation_maintenance.mdc`)
- [ ] API documentation generated and reviewed (if applicable)
- [ ] README updated (if applicable)
- [ ] Code comments are clear and complete
- [ ] Examples are correct and tested

### Integration Verification
- [ ] Code builds successfully (`cmake --build build`)
- [ ] All tests pass (`./scripts/run_tests.sh all`)
- [ ] CI/CD pipeline passes (GitHub Actions)
- [ ] No breaking changes (unless intentional and documented)
- [ ] Dependencies are correct and documented

### Test Verification
- [ ] Unit tests written and pass
- [ ] Integration tests written and pass
- [ ] Test coverage meets requirements (≥80% for new code, see `doc/18_TESTING_WORKFLOW.md`)
- [ ] Tests are meaningful and comprehensive
- [ ] Edge cases are tested
- [ ] Error conditions are tested
- [ ] Thread safety tested (if concurrent code)
- [ ] Regression tests pass (existing features work)

### Performance Verification
- [ ] Meets performance requirements (latency/throughput)
- [ ] No performance regressions (benchmark comparison)
- [ ] Memory usage acceptable
- [ ] CPU usage acceptable
- [ ] No blocking operations in critical paths
- [ ] Benchmarks pass (if applicable)

### QML Verification (if applicable)
- [ ] No qmllint errors or warnings
- [ ] No implicit types or unqualified access
- [ ] No binding loops
- [ ] Accessibility labels present
- [ ] Layout tested on different screen sizes
- [ ] Keyboard navigation works

### Final Checks
- [ ] All verification steps completed and tracked in ZTODO.md
- [ ] Verification status updated in task (✅ for each category)
- [ ] Code reviewed (self-review or peer review)
- [ ] Rollback plan documented (if verification failed)
- [ ] Ready for merge/commit
```

## Verification in ZTODO Format

When updating ZTODO items, include verification steps in the task description:

```markdown
- [ ] Implement Feature X
  - What: Description of what to implement
  - Why: Reason for the feature
  - Files: Files to modify/create
  - Acceptance: Acceptance criteria
  - Verification Steps:
    1. Functional: Test feature works as specified
    2. Code Quality: Run linter, check documentation
    3. Documentation: Update design docs
    4. Integration: Build and run tests
    5. Tests: Write and run unit/integration tests
  - Tests: Test requirements
  - Prompt: Prompt file
```

## Automated Verification

Where possible, automate verification:

### Pre-commit Hooks
- Linter checks
- Format checks
- Documentation checks (Doxygen)

### CI/CD Pipeline
- Build verification
- Test execution
- Coverage checks
- Documentation generation

### Test Suites
- Unit tests
- Integration tests
- End-to-end tests

## Manual Verification

For items that cannot be automated:

1. **Create verification checklist** - Document what to check
2. **Perform systematic review** - Go through checklist methodically
3. **Document results** - Note what was verified and any issues found
4. **Fix issues** - Address any problems found
5. **Re-verify** - Confirm fixes work

## Verification Failure

If verification fails:

1. **Document the failure** - What failed and why
2. **Fix the issue** - Address the problem
3. **Re-run verification** - Verify the fix
4. **Don't mark complete** - Until all verification passes

## Verification Examples

### Example 1: Adding a New Class

**Verification Steps:**
1. Functional: Class works as specified, methods behave correctly
2. Code Quality: Follows style, has Doxygen comments, no warnings
3. Documentation: Class design doc updated, API docs generated
4. Integration: Builds, integrates with existing code, tests pass
5. Tests: Unit tests written, integration tests updated

### Example 2: Updating Database Schema

**Verification Steps:**
1. Functional: Schema changes work, migrations run successfully
2. Code Quality: SQL follows conventions, no syntax errors
3. Documentation: Database design doc updated, migration documented
4. Integration: App builds, database operations work, tests pass
5. Tests: Database tests updated, migration tests added

### Example 3: Adding API Documentation

**Verification Steps:**
1. Functional: Documentation generates successfully, all APIs documented
2. Code Quality: Doxygen comments follow style, no warnings
3. Documentation: API docs generated, examples are correct, cross-refs valid
4. Integration: Doxygen workflow runs, CI passes
5. Tests: Documentation coverage check passes

### Example 4: Implementing QML Component (AdmissionModal)

**Verification Steps:**
1. Functional: Modal opens, validates input, triggers controller methods
2. Code Quality: qmllint clean, no implicit types, CMakeLists.txt updated for resources
3. Documentation: Component usage documented in ADT workflow doc
4. Integration: Works with PatientController, no layout regressions
5. Tests: QML test harness verifies admission flow
6. QML: No binding loops, accessibility labels present, responsive layout

### Example 5: Optimizing Alarm Detection Path

**Verification Steps:**
1. Functional: Alarm detection works correctly, thresholds respected
2. Code Quality: No warnings, thread-safe, build verification passes
3. Documentation: Performance improvements documented
4. Integration: Builds, tests pass, no breaking changes
5. Tests: Unit tests + regression tests pass
6. Performance: Latency < 50ms (verified with benchmark), no memory leaks

## Integration with AI Assistant

When prompting with ZTODO items:

1. **Extract verification criteria** from "Acceptance" section
2. **Generate verification checklist** based on task type
3. **Perform verification** as part of implementation
4. **Report verification results** before marking complete
5. **Fix any issues** found during verification

## Best Practices

### Do's
- ✅ Always verify before marking complete
- ✅ Use automated verification where possible
- ✅ Document verification results
- ✅ Fix issues before completion
- ✅ Re-verify after fixes

### Don'ts
- ❌ Don't skip verification
- ❌ Don't mark complete with failing verification
- ❌ Don't assume it works without testing
- ❌ Don't ignore verification failures
- ❌ Don't verify only happy path

## Related Documents

- `project-dashboard/ZTODO.md` - Task list with verification requirements
- `.cursor/rules/code_style.mdc` - Code quality standards
- `.cursor/rules/api_documentation.mdc` - Documentation requirements
- `.cursor/rules/documentation_maintenance.mdc` - Documentation update process
- `project-dashboard/doc/18_TESTING_WORKFLOW.md` - Testing requirements
