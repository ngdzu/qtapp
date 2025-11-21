FROM qtapp-qt-dev-env:latest AS builder
WORKDIR /lesson
COPY . ./
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . && cmake --install .

FROM qtapp-qt-runtime:latest AS runtime
WORKDIR /opt/lesson27
COPY --from=builder /opt/lesson27/lesson27 .
CMD ["./lesson27"]
