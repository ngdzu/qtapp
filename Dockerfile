# Multi-stage Dockerfile for a C++ project with Qt

# Stage 1: Base environment
FROM ubuntu:23.04 AS base

# Set timezone to avoid interactive timezone prompt
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install common dependencies
RUN apt-get update && apt-get install -y \
        build-essential \
        cmake \
        gdb \
        git \
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













