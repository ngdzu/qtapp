# Lesson 26: CI, Docker, and Builds

## Learning Goals
- Understand Continuous Integration (CI) for Qt applications
- Master Docker containerization for Qt builds
- Implement automated build pipelines
- Learn multi-stage Docker builds for optimization

## Introduction

Modern Qt development requires automated, reproducible builds across multiple platforms. This lesson covers using Docker for containerized builds and integrating Qt projects with CI/CD systems like GitHub Actions, GitLab CI, and Jenkins. By containerizing your build environment, you ensure consistency between development, testing, and production deployments.

## Key Concepts

**Multi-Stage Docker Builds**

Multi-stage builds separate the build environment from the runtime environment, dramatically reducing final image size:

```dockerfile
# Stage 1: Build environment with full SDK
FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y \
    qt6-base-dev cmake g++ make
WORKDIR /build
COPY . .
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# Stage 2: Minimal runtime with only libraries
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    libqt6core6 libqt6gui6 libqt6widgets6
COPY --from=builder /build/build/myapp /usr/local/bin/
CMD ["myapp"]
```

This pattern keeps development tools (SDK, compilers, headers) in the builder stage while the final image contains only runtime libraries and the executable.

**GitHub Actions for Qt**

GitHub Actions can automate building and testing Qt applications on every commit:

```yaml
name: Qt Build
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Qt
        uses: jurplel/install-qt-action@v3
        with:
          version: '6.5.0'
      - name: Build
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release
          cmake --build build
      - name: Test
        run: cd build && ctest --output-on-failure
```

**GitLab CI with Docker**

GitLab CI excels at Docker-based builds. Here's a `.gitlab-ci.yml` example:

```yaml
stages:
  - build
  - test

build-job:
  stage: build
  image: qt-dev-env:latest
  script:
    - cmake -B build -DCMAKE_BUILD_TYPE=Release
    - cmake --build build
  artifacts:
    paths:
      - build/

test-job:
  stage: test
  image: qt-runtime:latest
  script:
    - cd build && ctest --output-on-failure
  dependencies:
    - build-job
```

**Build Matrix for Cross-Platform**

Test across multiple Qt versions and platforms simultaneously:

```yaml
strategy:
  matrix:
    qt-version: ['6.5.0', '6.6.0']
    os: [ubuntu-latest, windows-latest, macos-latest]
runs-on: ${{ matrix.os }}
steps:
  - uses: jurplel/install-qt-action@v3
    with:
      version: ${{ matrix.qt-version }}
```

## Example Walkthrough

Let's build a complete CI/CD pipeline for a Qt project:

1. **Create Dockerfile with multi-stage build** - Separates build tools from runtime
2. **Set up GitHub Actions** - Automate builds on every push
3. **Add testing stage** - Run Qt Test suite automatically
4. **Publish artifacts** - Upload binaries for download

The Docker approach ensures builds are reproducible: "It works on my machine" becomes "It works in the container, which runs anywhere."

## Expected Output

A Qt application that displays:
- Build environment information (compiler, Qt version, CMake version)
- Docker status and container details if running in Docker
- CI/CD environment variables when running in GitHub Actions/GitLab CI
- Build configuration (Debug/Release, architecture)

This demonstrates how to detect and report build environment information, useful for debugging CI issues.

## Try It

1. **Examine the build info demo** - See how to detect build environment
2. **Create a GitHub Actions workflow** - Add `.github/workflows/qt-build.yml`
3. **Experiment with multi-stage builds** - Modify the Dockerfile to add/remove dependencies
4. **Add caching** - Use Docker layer caching or GitHub Actions cache to speed up builds
5. **Test matrix builds** - Build against multiple Qt versions

## Key Takeaways

- **Multi-stage Docker builds** reduce final image size by 10-20x by separating build and runtime
- **CI/CD automation** catches build breaks immediately, before they reach production
- **Reproducible builds** via Docker eliminate "works on my machine" problems
- **Build matrices** test compatibility across Qt versions and platforms efficiently
- **Environment detection** helps debug CI issues by reporting build context
- **Artifact publishing** makes binaries available for testing without manual builds
