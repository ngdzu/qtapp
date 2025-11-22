# Lesson 2 Dockerfile - reuses qtapp base image to minimize size
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson

# Copy lesson source files
COPY CMakeLists.txt main.cpp ./

# Configure and build with CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    mkdir -p /opt/lesson02 && \
    cp cpp-prereqs /opt/lesson02/

# Runtime stage: use minimal qt-runtime-nano base
FROM qtapp-qt-runtime-nano:latest AS runtime

WORKDIR /opt/lesson02

# Copy only the built executable
COPY --from=builder /opt/lesson02/cpp-prereqs .

# This is a console application, so no X11 needed
CMD ["./cpp-prereqs"]
