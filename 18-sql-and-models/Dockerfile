# Lesson 18 Dockerfile - reuses qtapp base images to minimize size
# This assumes qtapp-qt-dev-env and qtapp-qt-runtime base images are available locally
# Note: This lesson requires Qt SQL support, so uses qt-runtime (not nano)

# Builder stage: use qt-dev-env which has development tools
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson

# Copy all lesson source files
COPY . ./

# Configure and build with CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

# Runtime stage: use qt-runtime (includes SQL support)
FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson18

# Copy only the built executable
COPY --from=builder /opt/lesson18/lesson18 .

# Note: For GUI apps, you'll need to set DISPLAY and mount X11 socket
# This is covered in the README.md

CMD ["./lesson18"]
