FROM qtapp-qt-dev-env:latest AS builder
WORKDIR /lesson
COPY . ./
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . && cmake --install .

FROM qtapp-qt-qtquick:latest AS runtime
WORKDIR /opt/lesson13
COPY --from=builder /opt/lesson13/lesson13 .
CMD ["./lesson13"]
