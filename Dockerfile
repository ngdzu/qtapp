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
    x11-apps \
    && \
    apt-get clean

# Set timezone in the runtime image as well
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Define workspace
WORKDIR /workspace

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













