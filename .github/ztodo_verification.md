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
- ✅ No linter errors
- ✅ No compiler warnings
- ✅ Code is properly documented (Doxygen comments)
- ✅ Follows architecture patterns
- ✅ No memory leaks
- ✅ No security vulnerabilities

**How to Verify:**
- Linter checks (`clang-format`, `clang-tidy`)
- Static analysis tools
- Code review
- Documentation generation
- Memory leak detection

**Example:**
```markdown
- [ ] Code Quality Verification:
  - [ ] No linter errors
  - [ ] No compiler warnings
  - [ ] Doxygen comments added
  - [ ] Code follows style guidelines
  - [ ] No memory leaks detected
```

### Documentation Verification

**What to Verify:**
- ✅ All relevant documentation updated
- ✅ API documentation generated (if applicable)
- ✅ Design documents updated
- ✅ README updated (if applicable)
- ✅ Code comments are clear and complete

**How to Verify:**
- Review documentation files
- Generate and review API docs
- Check cross-references
- Verify examples are correct

**Example:**
```markdown
- [ ] Documentation Verification:
  - [ ] Design docs updated
  - [ ] API docs generated and reviewed
  - [ ] README updated
  - [ ] Code comments complete
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

**How to Verify:**
- Run test suite
- Check test coverage
- Review test quality
- Verify test documentation

**Example:**
```markdown
- [ ] Test Verification:
  - [ ] Unit tests written and pass
  - [ ] Integration tests written and pass
  - [ ] Test coverage meets threshold
  - [ ] Edge cases tested
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
- [ ] Test coverage meets requirements (see `doc/18_TESTING_WORKFLOW.md`)
- [ ] Tests are meaningful and comprehensive
- [ ] Edge cases are tested
- [ ] Error conditions are tested

### Final Checks
- [ ] All verification steps completed
- [ ] Code reviewed (self-review or peer review)
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
3. Documentation: API docs generated, examples are correct
4. Integration: Doxygen workflow runs, CI passes
5. Tests: Documentation coverage check passes

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
