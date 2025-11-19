# Lesson 1 Dockerfile - reuses qtapp base image to minimize size
# This assumes the qtapp base image (qt-dev-env stage) is available locally

# Use the qt-dev-env stage from the root qtapp project as our base
# This provides Qt 6, CMake, and all build tools already installed
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson

# Copy only lesson source files
COPY CMakeLists.txt main.cpp ./

# Configure and build with CMake (using make for consistency with base image)
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    mkdir -p /opt/lesson01 && \
    cp qt-setup /opt/lesson01/

# Runtime stage: use minimal qt-runtime base
# This keeps runtime dependencies minimal - only Qt libs, no dev tools
FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson01

# Copy only the built executable
COPY --from=builder /opt/lesson01/qt-setup .

# Note: For GUI apps, you'll need to set DISPLAY and mount X11 socket
# This is covered in the README.md

CMD ["./qt-setup"]
