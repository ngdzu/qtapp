---
description: When create/edit lessons at the root level of the repository. 
alwaysApply: false
---
# Lesson Structure Requirements and Guidelines

## Overview

This document defines the requirements and best practices for creating Qt/C++ lessons. Each lesson must be self-contained, buildable, runnable, and educational. Lessons follow a consistent structure to ensure quality and maintainability.

**Key Principles:**
- **Self-Contained** - Each lesson is a complete, runnable example
- **Educational** - Clear learning goals with progressive difficulty
- **Buildable** - Must build and run in Docker container
- **Documented** - Comprehensive documentation for learners
- **Tested** - Quiz and answers validate understanding

---

## 1. Required Files

Each lesson folder (e.g., `01-qt-setup/`, `05-signals-and-slots/`) must include exactly **7 required files**:

### 1.1. `lesson.md` (300-600 words)

**Purpose:** Main lesson content explaining the concept with examples.

**Required Sections:**
1. **Learning Goals** - What the learner will understand after this lesson
2. **Code Snippets** - 2-3 key code examples demonstrating the concept
3. **Example Walkthrough** - Step-by-step explanation of how the code works
4. **Expected Output** - What the learner should see when running the code
5. **Try It Exercise** - Hands-on exercise for the learner to practice
6. **Key Takeaways** - Summary of important concepts learned

**Format Guidelines:**
- Use clear headings and structure
- Include syntax-highlighted code blocks
- Explain the "why" not just the "how"
- Keep examples focused and concise
- Reference related lessons when appropriate

### 1.2. `quiz.md` (5-10 questions)

**Purpose:** Assess understanding with a mix of question types.

**Question Distribution:**
- **2-3 Conceptual Questions** - Test understanding of concepts, principles, and "why"
- **2-4 Practical Code Questions** - Test ability to write/fix code, identify errors
- **1-2 Reflection Questions** - Test deeper understanding and application

**Format Requirements:**
- Number all questions sequentially
- Include Qt/C++ code snippets when relevant
- Make questions progressively more challenging
- Ensure questions align with learning goals

**Example Structure:**
```markdown
## Quiz Questions

1. **Conceptual:** What is the difference between signals and slots in Qt?

2. **Practical:** What is wrong with this code snippet?
   ```cpp
   // Code snippet here
   ```

3. **Reflection:** When would you use signals/slots vs direct function calls?
```

### 1.3. `answers.md`

**Purpose:** Provide complete answers with explanations and corrected code.

**Required for Each Question:**
1. **Repeat the question** - Full question text
2. **Direct Answer** - One-sentence direct answer
3. **Explanation** - 1-3 sentences referencing the lesson content
4. **Corrected/Complete Code** - If applicable, show corrected code
5. **Pitfalls and Prevention** - Common mistakes and how to avoid them

**Format:**
```markdown
## Answers

### Question 1: [Question Text]

**Answer:** [Direct one-sentence answer]

**Explanation:** [1-3 sentences referencing lesson]

**Pitfalls:**
- [Common mistake 1]: [How to prevent]
- [Common mistake 2]: [How to prevent]
```

### 1.4. `main.cpp` (and headers if needed)

**Purpose:** Single executable demonstrating the lesson concept.

**Requirements:**
- **Single `main()` function** - One entry point
- **Concise demo** - Focused on the lesson concept, not extraneous features
- **Idiomatic Qt/C++** - Follows Qt and C++ best practices
- **Doxygen comments** - All public APIs documented (see `.cursor/rules/api_documentation.mdc`)
- **No hardcoded values** - Use constants or configuration (see `.cursor/rules/cpp_guidelines.mdc`)

**Code Quality:**
- Follow C++17 standard
- Use modern Qt 6 APIs
- Include proper error handling
- Use `Result<T, Error>` for operations that can fail
- Follow naming conventions (PascalCase classes, camelCase functions)

### 1.5. `CMakeLists.txt`

**Purpose:** Minimal cross-platform build configuration.

**Required Elements:**
```cmake
cmake_minimum_required(VERSION 3.16)
project(lesson-name VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

add_executable(executable-name main.cpp)
target_link_libraries(executable-name PRIVATE Qt6::Core Qt6::Widgets)

install(TARGETS executable-name RUNTIME DESTINATION /opt/lesson##)
```

**Guidelines:**
- Use minimal Qt components (only what's needed)
- Set `CMAKE_CXX_STANDARD 17` and `CMAKE_CXX_STANDARD_REQUIRED ON`
- Enable `CMAKE_AUTOMOC ON` for Qt MOC
- Install to `/opt/lesson##/` (e.g., `/opt/lesson01/`, `/opt/lesson05/`)
- Keep it simple - no unnecessary complexity

### 1.6. `Dockerfile`

**Purpose:** Multi-stage build for containerized execution.

**Required Pattern:**
```dockerfile
# Multi-stage build
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /build
COPY . .
RUN cmake -B build -S . && \
    cmake --build build && \
    cmake --install build

FROM qtapp-qt-runtime:latest
COPY --from=builder /opt/lesson##/executable-name /opt/lesson##/
WORKDIR /opt/lesson##
CMD ["./executable-name"]
```

**Guidelines:**
- Use `qtapp-qt-dev-env:latest` for builder stage
- Use `qtapp-qt-runtime:latest` for runtime stage
- Builder copies source, builds, and installs
- Runtime copies only the executable (minimal layers)
- Keep image size small (~40 KB incremental)

### 1.7. `README.md`

**Purpose:** Build and run instructions for learners.

**Required Sections:**
1. **Prerequisites** - System requirements (X11 for GUI apps, Docker, etc.)
2. **Build Instructions** - How to build the lesson
3. **Run Instructions** - How to run the lesson (with examples for macOS/Linux)
4. **Expected Behavior** - What the learner should see

**Format Guidelines:**
- **NO comments inside code blocks** - Keep code blocks clean
- Provide platform-specific examples (macOS, Linux)
- Include Docker commands for containerized execution
- Document X11 setup for GUI applications
- Keep instructions clear and concise

**Example Structure:**
```markdown
# Lesson ##: [Lesson Title]

## Prerequisites

- Docker installed
- X11 server (for GUI applications on macOS/Linux)

## Building

```bash
docker build -t lesson## .
```

## Running

### macOS
```bash
DISPLAY=host.docker.internal:0 docker run -e DISPLAY=host.docker.internal:0 lesson##
```

### Linux
```bash
docker run -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY lesson##
```

## Expected Behavior

[Description of what the application does]
```

---

## 2. Lesson Progression

Lessons are organized into three difficulty levels:

### 2.1. Beginner (Lessons 01-07)

**Focus:** Foundation and basic Qt concepts

**Topics:**
- Lesson 01: Qt setup and project structure
- Lesson 02: C++ prerequisites (smart pointers, RAII, modern C++)
- Lesson 03: Widgets basics (QPushButton, QLabel, QLineEdit)
- Lesson 04: Layouts and containers (QVBoxLayout, QHBoxLayout, QGridLayout)
- Lesson 05: Signals and slots (connect, emit, lambda connections)
- Lesson 06: Events and handling (mouse, keyboard, custom events)
- Lesson 07: Dialogs (QMessageBox, QFileDialog, custom dialogs)

**Learning Goals:**
- Understand Qt project structure
- Master basic widget usage
- Understand event-driven programming
- Create simple interactive applications

### 2.2. Intermediate (Lessons 08-21)

**Focus:** Advanced Qt features and patterns

**Topics:**
- Lesson 08: Model/View architecture (QAbstractItemModel, QListView)
- Lesson 09: Custom models (implementing QAbstractItemModel)
- Lesson 10: Item delegates (QStyledItemDelegate, custom rendering)
- Lesson 11: Resources and translations (QRC files, i18n)
- Lesson 12: Qt Quick/QML introduction (QML syntax, components)
- Lesson 13: Qt Quick Controls (Button, TextField, ListView in QML)
- Lesson 14: Graphics View (QGraphicsScene, QGraphicsItem)
- Lesson 15: Multimedia (QMediaPlayer, audio/video)
- Lesson 16: Networking (QNetworkAccessManager, HTTP requests)
- Lesson 17: Threading and concurrency (QThread, QMutex, async patterns)
- Lesson 18: SQL and models (QSqlDatabase, QSqlTableModel)
- Lesson 19: Serialization and settings (QSettings, JSON, XML)
- Lesson 20: Styles, themes, and palette (QStyle, QPalette, styling)
- Lesson 21: Animation and transitions (QPropertyAnimation, QML animations)

**Learning Goals:**
- Master Model/View architecture
- Understand QML and declarative UI
- Work with databases and networking
- Implement threading and concurrency
- Create polished, styled applications

### 2.3. Advanced (Lessons 22-28)

**Focus:** Advanced topics and production practices

**Topics:**
- Lesson 22: OpenGL and 3D (QOpenGLWidget, 3D graphics)
- Lesson 23: Plugins and extensibility (QPluginLoader, plugin architecture)
- Lesson 24: Testing and automation (Qt Test, unit testing, mocking)
- Lesson 25: Deployment and packaging (installers, cross-platform deployment)
- Lesson 26: CI/CD and builds (GitHub Actions, automated builds)
- Lesson 27: Performance and profiling (QProfiler, optimization)
- Lesson 28: Accessibility and i18n (a11y, full internationalization)

**Learning Goals:**
- Understand advanced graphics programming
- Implement plugin architectures
- Set up testing and CI/CD
- Optimize application performance
- Create accessible, internationalized applications

---

## 3. Code Quality Standards

### 3.1. C++ Standards

- **C++17** - Use C++17 features throughout
- **Modern C++** - Prefer `auto`, range-based for, smart pointers
- **Qt 6 APIs** - Use modern Qt 6 features, avoid deprecated Qt 5 APIs
- **Doxygen Comments** - All public APIs must have Doxygen comments

### 3.2. Qt Best Practices

- **Signals/Slots** - Use new-style connections (function pointers, not strings)
- **Memory Management** - Use Qt parent-child or smart pointers
- **Error Handling** - Use `Result<T, Error>` for operations that can fail
- **No Hardcoded Values** - Use constants or configuration

### 3.3. Code Organization

- **Single Focus** - Each lesson demonstrates one concept clearly
- **Minimal Dependencies** - Only include what's needed
- **Clear Naming** - Use descriptive names that explain intent
- **Comments** - Explain "why" not "what" (code should be self-documenting)

---

## 4. Documentation Standards

### 4.1. `lesson.md` Quality

- **Word Count:** 300-600 words (concise but complete)
- **Structure:** Clear sections with headings
- **Code Examples:** Syntax-highlighted, focused, explained
- **Progressive Learning:** Build on previous concepts
- **Cross-References:** Link to related lessons when helpful

### 4.2. `quiz.md` Quality

- **Question Count:** 5-10 questions (balanced coverage)
- **Question Types:** Mix of conceptual, practical, and reflection
- **Difficulty:** Progressive (easier to harder)
- **Code Snippets:** Include when testing code understanding
- **Alignment:** Questions must align with learning goals

### 4.3. `answers.md` Quality

- **Completeness:** Every question has a complete answer
- **Explanation:** Context and reasoning, not just the answer
- **Code Corrections:** Show before/after when applicable
- **Pitfalls:** Highlight common mistakes and prevention
- **Learning Focus:** Help learners understand, not just memorize

---

## 5. Build and Runtime Requirements

### 5.1. Docker Build

- **Must build successfully** in Docker container
- **Use multi-stage build** to minimize image size
- **Base images:** `qtapp-qt-dev-env:latest` and `qtapp-qt-runtime:latest`
- **Incremental size:** Keep under ~40 KB additional storage

### 5.2. Application Execution

- **Must run successfully** in container
- **GUI Applications:** Require X11 setup (documented in README)
- **Console Applications:** Should work without X11
- **Error Handling:** Graceful failure with clear error messages

### 5.3. Platform Support

- **Cross-platform:** Code should work on macOS, Linux, Windows
- **Docker-first:** Primary execution environment is Docker
- **X11 Setup:** Document X11 requirements for GUI apps
- **Environment Variables:** Use for configuration (e.g., `DISPLAY`)

---

## 6. Lesson Completion Checklist

Before marking a lesson as complete, verify all items:

### File Existence
- [ ] `lesson.md` exists and is 300-600 words
- [ ] `quiz.md` exists with 5-10 questions
- [ ] `answers.md` exists with complete answers
- [ ] `main.cpp` exists (and headers if needed)
- [ ] `CMakeLists.txt` exists with correct configuration
- [ ] `Dockerfile` exists with multi-stage build
- [ ] `README.md` exists with build/run instructions

### Code Quality
- [ ] Code builds successfully in Docker
- [ ] Code follows C++17 and Qt 6 best practices
- [ ] Doxygen comments on all public APIs
- [ ] No hardcoded values (use constants)
- [ ] Error handling uses `Result<T, Error>` where appropriate
- [ ] Single executable with one `main()` function

### Documentation Quality
- [ ] `lesson.md` has all required sections (goals, snippets, walkthrough, output, exercise, takeaways)
- [ ] `quiz.md` has balanced question types (conceptual, practical, reflection)
- [ ] `answers.md` provides complete answers with explanations and pitfalls
- [ ] `README.md` has no comments inside code blocks
- [ ] All documentation is clear and educational

### Build and Runtime
- [ ] Application builds inside Docker
- [ ] Application runs successfully in container
- [ ] Dockerfile follows multi-stage pattern
- [ ] Incremental storage impact is ~40 KB
- [ ] README provides correct build/run instructions

### Educational Value
- [ ] Lesson demonstrates the concept clearly
- [ ] Code examples are focused and relevant
- [ ] Quiz questions test understanding
- [ ] Answers help learners understand concepts
- [ ] Lesson builds on previous lessons appropriately

---

## 7. Common Patterns

### 7.1. GUI Applications

**CMakeLists.txt:**
```cmake
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
# ... rest of configuration
```

**README.md:**
- Document X11 prerequisites
- Provide macOS example: `DISPLAY=host.docker.internal:0`
- Provide Linux example: `-v /tmp/.X11-unix:/tmp/.X11-unix`
- Suppress GL noise: `-e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false"`

**main.cpp:**
```cpp
#include <QApplication>
// Use QApplication for GUI apps
```

### 7.2. Console Applications

**CMakeLists.txt:**
```cmake
find_package(Qt6 COMPONENTS Core REQUIRED)
# Only Core, no Widgets
```

**README.md:**
- No X11 instructions needed
- Streamlined run commands

**main.cpp:**
```cpp
#include <QCoreApplication>
// Use QCoreApplication for console apps
```

### 7.3. QML Applications

**CMakeLists.txt:**
```cmake
find_package(Qt6 COMPONENTS Core Quick REQUIRED)
# ... QML-specific configuration
```

**README.md:**
- Document QML-specific requirements
- Include QML file structure explanation

---

## 8. Best Practices

### 8.1. Lesson Content

- **Start Simple:** Begin with basic examples, build complexity
- **Show, Don't Tell:** Use code examples to demonstrate concepts
- **Explain Why:** Help learners understand design decisions
- **Progressive Complexity:** Each example builds on the previous
- **Real-World Context:** Show practical applications

### 8.2. Code Examples

- **Focused:** Each example demonstrates one concept
- **Complete:** Examples should compile and run
- **Idiomatic:** Use Qt and C++ best practices
- **Commented:** Explain non-obvious parts
- **Tested:** Verify examples work before including

### 8.3. Quiz Design

- **Test Understanding:** Not just memorization
- **Progressive Difficulty:** Easier questions first
- **Practical Application:** Include code-writing questions
- **Common Mistakes:** Test for typical pitfalls
- **Reflection:** Encourage deeper thinking

### 8.4. Answer Quality

- **Complete:** Full explanations, not just answers
- **Educational:** Help learners understand, not just pass
- **Pitfall Prevention:** Highlight and prevent common mistakes
- **Code Corrections:** Show before/after with explanations
- **Cross-References:** Link back to lesson content

---

## 9. Quick Reference

### File Checklist

| File | Purpose | Requirements |
|------|---------|--------------|
| `lesson.md` | Main content | 300-600 words, all sections |
| `quiz.md` | Assessment | 5-10 questions, mixed types |
| `answers.md` | Answer key | Complete answers, pitfalls |
| `main.cpp` | Code | Single executable, idiomatic |
| `CMakeLists.txt` | Build config | C++17, Qt 6, minimal |
| `Dockerfile` | Container | Multi-stage, ~40 KB |
| `README.md` | Instructions | Build/run, no code comments |

### Lesson Progression

| Level | Lessons | Focus |
|-------|---------|-------|
| **Beginner** | 01-07 | Foundation, widgets, basics |
| **Intermediate** | 08-21 | Advanced features, patterns |
| **Advanced** | 22-28 | Production practices, optimization |

### Code Standards

- **C++ Standard:** C++17
- **Qt Version:** Qt 6
- **Documentation:** Doxygen comments required
- **Error Handling:** `Result<T, Error>` pattern
- **Constants:** No hardcoded values

---

## 10. Related Guidelines

- **C++ Guidelines:** See `.cursor/rules/cpp_guidelines.mdc` for C++ coding standards
- **QML Guidelines:** See `.cursor/rules/qml_guidelines.mdc` for QML coding standards
- **API Documentation:** See `.cursor/rules/api_documentation.mdc` for documentation requirements
- **Common Patterns:** See `.cursor/rules/common_patterns.mdc` for Qt application patterns
- **Docker Architecture:** See `.cursor/rules/docker_architecture.mdc` for Docker patterns

---

## Enforcement

- **Code Review:** All lessons must follow these guidelines
- **Build Verification:** Lessons must build and run in Docker
- **Documentation Review:** All documentation must meet quality standards
- **Educational Review:** Lessons must provide clear learning value

---

**Remember:** Lessons are educational tools. Prioritize clarity, correctness, and learning value. Keep code simple, examples focused, and explanations clear. Each lesson should be a complete, runnable, educational experience.
