# Lesson 15 Dockerfile - reuses qtapp base images to minimize size
# This assumes qtapp-qt-dev-env and qtapp-qt-runtime base images are available locally
# Note: This lesson requires Qt Multimedia packages which add ~100MB to the image

# Builder stage: use qt-dev-env and add multimedia dev packages
FROM qtapp-qt-dev-env:latest AS builder

RUN apt-get update && apt-get install -y \
    qt6-multimedia-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /lesson

# Copy all lesson source files
COPY . ./

# Configure and build with CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

# Runtime stage: use qt-runtime and add multimedia runtime packages
FROM qtapp-qt-runtime:latest AS runtime

RUN apt-get update && apt-get install -y \
    libqt6multimedia6 \
    libqt6multimediawidgets6 \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-libav \
    gstreamer1.0-pulseaudio \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/lesson15

# Copy the built executable and audio file
COPY --from=builder /opt/lesson15/lesson15 .
COPY SoundHelix-Song-1.mp3 .

# Note: For GUI apps, you'll need to set DISPLAY and mount X11 socket
# For audio, you'll need PulseAudio setup (see README.md)

CMD ["./lesson15"]
