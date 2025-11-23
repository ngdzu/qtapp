# GitHub Copilot Instructions for Qt Learning Course

## Project Overview

This is a lesson-based Qt learning course where each lesson is a self-contained folder that can be built and run inside a Docker container. The course teaches Qt 6 development from beginner to advanced topics, with a focus on practical, hands-on learning.

## Lesson Structure Requirements

When creating or modifying lessons, ensure each lesson folder contains:

### Required Files

1. **lesson.md** (300-600 words)
   - Clear learning goals at the start
   - Concept explanation with 2-3 code snippets
   - Example walkthrough
   - "Expected Output" section
   - "Try It" exercise section
   - Key takeaways

2. **quiz.md** (5-10 questions)
   - 2-3 conceptual questions (why/trade-offs/when to use)
   - 2-4 practical code questions (what does this do/fill in the blank/spot the bug)
   - 1-2 reflection questions (how would you use this in your project)
   - Number all questions clearly
   - Include Qt/C++ code snippets in questions where applicable

3. **answers.md**
   - Repeat each question before its answer
   - Provide: (1) one-sentence direct answer, (2) 1-3 sentence explanation referencing the lesson, (3) corrected/complete code for code questions
   - For pitfall questions, describe the pitfall and how to avoid it

4. **main.cpp** (and any .h files)
   - Single executable with one `main()` function
   - Demonstrates the lesson concept clearly
   - Small, focused example (not complex)
   - Idiomatic Qt/C++ code

5. **CMakeLists.txt**
   - Minimal, cross-platform CMake setup
   - Use `find_package(Qt6 COMPONENTS ... REQUIRED)`
   - Set `CMAKE_CXX_STANDARD 17`
   - Enable `CMAKE_AUTOMOC ON`
   - Install target to `/opt/lesson##/`

6. **Dockerfile**
   - Multi-stage build: builder stage uses `qtapp-qt-dev-env:latest`, runtime stage uses `qtapp-qt-runtime:latest`
   - Builder stage: copy source, build with CMake, install to `/opt/lesson##/`
   - Runtime stage: copy only the built executable
   - Minimal layers for optimal storage efficiency

7. **README.md**
   - Prerequisites section (X11 setup for GUI apps)
   - Build instructions (shared base image, then lesson image)
   - Run instructions (macOS and Linux examples)
   - Expected behavior description
   - **CRITICAL**: No comments inside code blocks - all explanations go in descriptive text before the code

## Docker Architecture

### Base Images

- **qtapp-qt-dev-env:latest** - Development base with Qt 6 SDK, CMake, build tools (used in builder stage)
- **qtapp-qt-runtime:latest** - Runtime-only base with Qt 6 libraries, no dev tools (used in runtime stage)

### Build Pattern

```dockerfile
FROM qtapp-qt-dev-env:latest AS builder
WORKDIR /lesson
COPY CMakeLists.txt main.cpp ./
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . && cmake --install .

FROM qtapp-qt-runtime:latest AS runtime
WORKDIR /opt/lesson##
COPY --from=builder /opt/lesson##/<executable> .
CMD ["./<executable>"]
```

### Storage Efficiency

- All lessons share the same runtime base (~611MB)
- Each lesson adds only ~40KB (just the executable)
- Total storage for all 28 lessons: ~612MB
- Never use separate Ubuntu base in lesson Dockerfiles

## Code Style Guidelines

### C++ and Qt

- Use modern C++17 features
- Prefer Qt 6 new-style signals/slots: `QObject::connect(&obj, &Class::signal, lambda)`
- Use automatic memory management via parent-child relationships
- Keep examples simple and focused on one concept
- No unnecessary complexity or features beyond the lesson scope

### Documentation

- Clear, concise language for C++ developers new to Qt
- Explain "why" not just "how"
- Reference Qt documentation when appropriate
- Include common pitfalls and how to avoid them

## Lesson Progression

1. **Beginner (01-07)**: Qt setup, C++ prereqs, basic widgets, layouts, signals/slots, events, dialogs
2. **Intermediate (08-21)**: Model/View, resources, Qt Quick/QML, graphics, multimedia, networking, threading, SQL, serialization, styling, animations
3. **Advanced (22-28)**: OpenGL, plugins, testing, deployment, CI/CD, performance, accessibility/i18n

## Common Patterns

### GUI Applications

- Use `QApplication` (not `QCoreApplication`)
- Include X11 setup instructions in README
- Show macOS (`DISPLAY=host.docker.internal:0`) and Linux (`-v /tmp/.X11-unix:/tmp/.X11-unix`) run examples
- Suppress GL warnings: `-e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false"`

### Console Applications

- Use `QCoreApplication`
- No X11 setup needed
- Simpler Docker run command

### CMake

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

## Quality Checklist

Before considering a lesson complete, verify:

- [ ] All 7 required files present
- [ ] Code compiles in Docker container
- [ ] Single executable demonstrates the concept
- [ ] lesson.md is 300-600 words with all required sections
- [ ] quiz.md has 5-10 well-structured questions
- [ ] answers.md provides complete, detailed answers
- [ ] Dockerfile uses multi-stage build with correct base images
- [ ] README.md has no comments in code blocks
- [ ] Application runs successfully in container
- [ ] Storage footprint is minimal (~40KB unique per lesson)

## Maintenance Notes

- Keep `table_of_contents.md` updated when adding/modifying lessons
- Rebuild base images when updating Qt dependencies: `docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .` and `docker build --target qt-runtime -t qtapp-qt-runtime:latest .`
- Test lesson builds regularly to ensure compatibility
- Document any deviations from standard patterns in the lesson README
 - When modifying Mermaid diagrams (files with `.mmd`), render them to SVG and verify there are no parser errors: run the mermaid CLI and check the terminal output, then fix any errors before committing. Example:

   ```bash
   npx @mermaid-js/mermaid-cli -i doc/12_THREAD_MODEL.mmd -o doc/12_THREAD_MODEL.svg
   ```
   Ensure the command exits without errors and the generated `*.svg` is valid.

## Current Status

- ✅ Lessons 1-3 complete (qt-setup, cpp-prereqs, widgets-basics)
- 🚧 Lessons 4-28 to be generated
- Base infrastructure established and tested
