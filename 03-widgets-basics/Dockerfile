# Multi-stage Dockerfile for Lesson 3: Qt Widgets Basics
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson
COPY CMakeLists.txt main.cpp ./

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson03
COPY --from=builder /opt/lesson03/widgets-basics .

CMD ["./widgets-basics"]
