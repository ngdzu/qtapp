# Multi-stage Dockerfile for a C++ project with Qt

# Stage 1: Base environment
FROM ubuntu:22.04 AS base

# Set timezone to avoid interactive timezone prompt
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install common dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gdb \
    git \
    # X11 / OpenGL development packages needed to build and run Qt apps
    libgl1-mesa-dev \
    libgl1-mesa-dri \
    libglu1-mesa-dev \
    mesa-common-dev \
    libx11-dev \
    libxext-dev \
    libxrender-dev \
    libxrandr-dev \
    libxfixes-dev \
    libxcb1-dev \
    libxkbcommon-dev \
    pkg-config \
    && \
    apt-get clean

# Stage 2: Qt development environment
FROM base AS qt-dev-env

RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libqt6widgets6 \
    qt6-qpa-plugins \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qt6-declarative-dev \
    qt6-multimedia-dev \
    libqt6multimedia6 \
    libqt6multimediawidgets6 \
    libxkbcommon0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-render-util0 \
    libxcb-xinerama0 \
    x11-apps \
    qml6-module-qtquick \
    qml6-module-qtquick-window \
    && \
    apt-get clean

# Set timezone in the runtime image as well
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Define workspace
WORKDIR /workspace

# Stage 3: Qt runtime-only environment (minimal, for lesson containers)
FROM ubuntu:22.04 AS qt-runtime

ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
    libqt6widgets6 \
    libqt6gui6 \
    libqt6core6 \
    libqt6sql6 \
    qt6-qpa-plugins \
    libgl1-mesa-glx \
    libgl1-mesa-dri \
    libxkbcommon0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-render-util0 \
    libxcb-xinerama0 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Stage 4: Qt Quick/QML runtime environment (for lessons 12-13)
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

# Final runtime image: reuse qt-dev-env so Qt libs are available
FROM qt-dev-env AS runtime

# copy built binary from builder stage
COPY --from=builder /opt/qtapp /opt/qtapp

WORKDIR /opt/qtapp

# Default command runs the built Qt application. Compose can override this.
ENTRYPOINT ["/opt/qtapp/d0"]













