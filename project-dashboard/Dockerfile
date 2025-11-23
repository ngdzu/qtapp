# Builder stage
FROM qtapp-qt-dev-env:latest AS builder

# Install Qt Quick/QML dependencies
RUN apt-get update && apt-get install -y \
    qt6-declarative-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .
COPY resources.qrc .
COPY src/ src/
COPY resources/ resources/

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/project-dashboard && \
    cmake --build . && \
    cmake --install .

# Runtime stage
FROM qtapp-qt-runtime-nano:latest AS runtime

# Install Qt Quick/QML runtime dependencies
RUN apt-get update && apt-get install -y \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-templates \
    qml6-module-qtquick-window \
    qml6-module-qtqml-workerscript \
    qml6-module-qtquick-shapes \
    libqt6qml6 \
    libqt6quick6 \
    libqt6quickcontrols2-6 \
    libqt6quicktemplates2-6 \
    libqt6quickshapes6 \
    libgl1-mesa-dri \
    libgl1-mesa-glx \
    mesa-utils \
    libegl1-mesa \
    libglu1-mesa \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/project-dashboard

COPY --from=builder /opt/project-dashboard/bin/appproject-dashboard .

CMD ["./appproject-dashboard"]
