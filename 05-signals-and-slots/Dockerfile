# Multi-stage Dockerfile for Lesson 5: Signals and Slots
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson
COPY CMakeLists.txt main.cpp ./

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson05
COPY --from=builder /opt/lesson05/signals-and-slots .

CMD ["./signals-and-slots"]
