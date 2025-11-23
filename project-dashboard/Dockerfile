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
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/project-dashboard && \
    cmake --build . && \
    cmake --install .

# Runtime stage: use the shared qt quick/runtime image which already
# contains QML modules and Qt runtime libs to avoid duplicating installs.
FROM qtapp-qt-qtquick:local AS runtime

WORKDIR /opt/project-dashboard

COPY --from=builder /opt/project-dashboard/bin/appproject-dashboard .

CMD ["./appproject-dashboard"]
