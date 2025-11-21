FROM qtapp-qt-dev-env:latest AS builder
WORKDIR /lesson
COPY . ./
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . && cmake --install .

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
COPY --from=builder /opt/lesson15/lesson15 .
COPY SoundHelix-Song-1.mp3 .
CMD ["./lesson15"]
