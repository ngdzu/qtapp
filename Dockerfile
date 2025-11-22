# Multi-stage Dockerfile for a C++ project with Qt

# Stage 1: Base environment (for building only)
FROM ubuntu:22.04 AS base

# Set timezone to avoid interactive timezone prompt
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gdb \
    git \
    pkg-config \
    # X11 / OpenGL development packages needed to build Qt apps
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    mesa-common-dev \
    libx11-dev \
    libxext-dev \
    libxrender-dev \
    libxrandr-dev \
    libxfixes-dev \
    libxcb1-dev \
    libxkbcommon-dev \
    && \
    apt-get clean

# Stage 2: Qt development environment (for building lessons in builder stage)
FROM base AS qt-dev-env

RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Stage 3: Qt runtime environment (nano - ultra-minimal for basic Qt Widgets apps)
FROM ubuntu:22.04 AS qt-runtime-nano

ENV TZ=America/New_York \
    DEBIAN_FRONTEND=noninteractive

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
    libqt6widgets6 \
    libqt6gui6 \
    libqt6core6 \
    qt6-qpa-plugins \
    libgl1-mesa-glx \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    # Aggressive cleanup
    && rm -rf /usr/share/doc/* \
    && rm -rf /usr/share/man/* \
    && rm -rf /usr/share/locale/* \
    && rm -rf /usr/share/info/* \
    && rm -rf /usr/share/lintian/* \
    && rm -rf /usr/share/linda/* \
    && rm -rf /usr/share/bug/* \
    && rm -rf /var/cache/* \
    && rm -rf /tmp/* \
    && find /usr/lib -name "*.a" -delete \
    # Remove Python if installed (not needed for Qt C++ apps)
    && apt-get purge -y --auto-remove python3* || true \
    # Strip debug symbols from libraries
    && find /usr/lib -name "lib*.so*" -type f -exec strip --strip-unneeded {} \; 2>/dev/null || true \
    # Remove gconv (7MB) - locale conversions not needed
    && rm -rf /usr/lib/aarch64-linux-gnu/gconv \
    # Remove perl-base if present (5MB)
    && rm -rf /usr/lib/aarch64-linux-gnu/perl-base \
    # Remove LLVM (107MB) - not actually needed for basic Qt GUI
    && rm -f /usr/lib/aarch64-linux-gnu/libLLVM*.so* \
    # Remove all Mesa DRI drivers - not needed for basic Qt Widgets
    && rm -rf /usr/lib/aarch64-linux-gnu/dri \
    # Remove Python bytecode
    && find /usr -name "*.pyc" -delete 2>/dev/null || true \
    && find /usr -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true

WORKDIR /workspace

# Stage 4: Qt runtime environment (standard - includes SQL support for lessons that need it)
FROM ubuntu:22.04 AS qt-runtime

# Set timezone
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
    libqt6widgets6 \
    libqt6gui6 \
    libqt6core6 \
    libqt6sql6 \
    libqt6sql6-sqlite \
    qt6-qpa-plugins \
    qt6-gtk-platformtheme \
    libgl1-mesa-glx \
    libgl1-mesa-dri \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Stage 5: Qt Quick/QML development environment (for building QML lessons)
FROM qt-dev-env AS qt-qtquick-dev

RUN apt-get update && apt-get install -y \
    qt6-declarative-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Stage 6: Qt Quick/QML runtime environment (for running lessons 12-13)
FROM qt-runtime AS qt-qtquick

RUN apt-get update && apt-get install -y \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-templates \
    qml6-module-qtquick-window \
    qml6-module-qtqml-workerscript \
    libqt6qml6 \
    libqt6quick6 \
    libqt6qmlworkerscript6 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Stage: builder — compile the application into a deterministic location
FROM qt-dev-env AS builder

# copy source into builder stage
COPY . /workspace

WORKDIR /workspace

# Configure and build (out-of-source)
RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j"$(nproc)" && \
    mkdir -p /opt/qtapp && \
    cp d0 /opt/qtapp/ || true

# Final runtime image: reuse qt-runtime so Qt libs are available
FROM qt-runtime AS runtime

# copy built binary from builder stage
COPY --from=builder /opt/qtapp /opt/qtapp

WORKDIR /opt/qtapp

# Default command runs the built Qt application. Compose can override this.
ENTRYPOINT ["/opt/qtapp/d0"]













