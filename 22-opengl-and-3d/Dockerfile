FROM qtapp-qt-dev-env:latest AS builder
WORKDIR /lesson
COPY . ./
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . && cmake --install .

FROM qtapp-qt-runtime:latest AS runtime
RUN apt-get update && apt-get install -y libqt6opengl6 libqt6openglwidgets6 && apt-get clean && rm -rf /var/lib/apt/lists/*
WORKDIR /opt/lesson22
COPY --from=builder /opt/lesson22/lesson22 .
CMD ["./lesson22"]
