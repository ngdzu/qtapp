# Lesson 8 Dockerfile - reuses qtapp base images to minimize size
# This assumes qtapp-qt-dev-env and qtapp-qt-runtime-nano base images are available locally

# Builder stage: use qt-dev-env which has development tools
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson

# Copy only lesson source files
COPY CMakeLists.txt main.cpp ./

# Configure and build with CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

# Runtime stage: use nano runtime (aggressively optimized)
FROM qtapp-qt-runtime-nano:latest AS runtime

WORKDIR /opt/lesson08

# Copy only the built executable
COPY --from=builder /opt/lesson08/lesson08-modelview .

# Note: For GUI apps, you'll need to set DISPLAY and mount X11 socket
# This is covered in the README.md

CMD ["./lesson08-modelview"]
