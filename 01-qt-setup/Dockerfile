# Lesson 1 Dockerfile - reuses qtapp base images to minimize size
# This assumes qtapp-qt-dev-env and qtapp-qt-runtime base images are available locally

# Builder stage: use qt-dev-env which has development tools
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson

# Copy only lesson source files
COPY CMakeLists.txt main.cpp ./

# Configure and build with CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    mkdir -p /opt/lesson01 && \
    cp qt-setup /opt/lesson01/

# Runtime stage: use nano runtime (aggressively optimized)
FROM qtapp-qt-runtime-nano:latest AS runtime

WORKDIR /opt/lesson01

# Copy only the built executable
COPY --from=builder /opt/lesson01/qt-setup .

# Note: For GUI apps, you'll need to set DISPLAY and mount X11 socket
# This is covered in the README.md

CMD ["./qt-setup"]
