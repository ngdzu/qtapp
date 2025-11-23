FROM qtapp-qt-qtquick-dev:local AS builder
WORKDIR /workspace

# Copy source and build; the qt dev image already contains Qt development
# packages so we don't reinstall them here.
COPY . .

# Install app-specific development packages (WebSockets) required to build
# this simulator. The shared qt-qtquick-dev image contains Qt Quick
# development packages but not always WebSockets dev packages.
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libqt6websockets6-dev \
    && rm -rf /var/lib/apt/lists/* || true

RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --target install

FROM qtapp-qt-qtquick:local AS runtime
WORKDIR /opt/sensor-simulator

# The qt-qtquick runtime image includes Qt Quick/QML modules. Install
# only app-specific runtime packages (WebSockets) which are not always
# present in the base runtime image.
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libqt6websockets6 \
    xvfb \
    x11vnc \
    && rm -rf /var/lib/apt/lists/* || true

# Copy the installed executable from the builder
COPY --from=builder /opt/sensor-simulator/sensor_simulator .
# Also copy QML files (fallback load) into runtime image
COPY --from=builder /workspace/qml ./qml
CMD ["./sensor_simulator"]
