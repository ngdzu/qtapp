# Lesson 12 Dockerfile - reuses qtapp base images to minimize size
# This assumes qtapp-qt-qtquick-dev and qtapp-qt-qtquick base images are available locally

# Builder stage: use qt-qtquick-dev which has Qt Quick/QML development tools
FROM qtapp-qt-qtquick-dev:latest AS builder

WORKDIR /lesson

# Copy all lesson source files including QML resources
COPY . ./

# Configure and build with CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

# Runtime stage: use qtquick runtime (includes Qt Quick/QML libraries)
FROM qtapp-qt-qtquick:latest AS runtime

WORKDIR /opt/lesson12

# Copy only the built executable
COPY --from=builder /opt/lesson12/lesson12-qml .

# Note: For GUI apps, you'll need to set DISPLAY and mount X11 socket
# This is covered in the README.md

CMD ["./lesson12-qml"]
