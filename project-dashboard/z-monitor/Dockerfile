# Builder stage
FROM qtapp-qt-dev-env:local AS builder

# Builder uses the shared qt dev environment; app-specific dependencies
# should be minimal and installed here only if required.
WORKDIR /app

COPY CMakeLists.txt .
COPY resources.qrc .
COPY src/ src/
COPY resources/ resources/

# Configure, build, and install to a predictable location
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/z-monitor && \
    cmake --build . && \
    cmake --install .

# Runtime stage: use the shared qt quick/runtime image which already
# contains QML modules and Qt runtime libs to avoid duplicating installs.
FROM qtapp-qt-qtquick:local AS runtime

WORKDIR /opt/z-monitor

COPY --from=builder /opt/z-monitor/bin/z-monitor .

CMD ["./z-monitor"]
