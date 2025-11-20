# Multi-stage Dockerfile for Lesson 4: Layouts and Containers
FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson
COPY CMakeLists.txt main.cpp ./

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson04
COPY --from=builder /opt/lesson04/layouts-and-containers .

CMD ["./layouts-and-containers"]
