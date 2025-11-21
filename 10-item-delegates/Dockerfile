FROM qtapp-qt-dev-env:latest AS builder
WORKDIR /lesson
COPY CMakeLists.txt main.cpp ./
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . && cmake --install .

FROM qtapp-qt-runtime:latest AS runtime
WORKDIR /opt/lesson10
COPY --from=builder /opt/lesson10/lesson10-delegates .
CMD ["./lesson10-delegates"]
