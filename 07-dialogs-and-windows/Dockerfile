FROM qtapp-qt-dev-env:latest AS builder

WORKDIR /lesson
COPY CMakeLists.txt main.cpp ./

RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .

FROM qtapp-qt-runtime:latest AS runtime

WORKDIR /opt/lesson07
COPY --from=builder /opt/lesson07/lesson07-dialogs .

CMD ["./lesson07-dialogs"]
