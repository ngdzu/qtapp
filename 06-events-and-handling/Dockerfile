FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson
COPY CMakeLists.txt main.cpp ./

RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson06
COPY --from=builder /opt/lesson06/lesson06-events .

CMD ["./lesson06-events"]
