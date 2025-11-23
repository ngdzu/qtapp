#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

echo "Building qt-dev-env image..."
docker build --target qt-dev-env -t qtapp-qt-dev-env:local .

echo "Building qt-qtquick runtime image..."
docker build --target qt-qtquick -t qtapp-qt-qtquick:local .

echo "Building qt-qtquick-dev (development with QML) image..."
docker build --target qt-qtquick-dev -t qtapp-qt-qtquick-dev:local .

echo "(Optional) Building qt-runtime image..."
docker build --target qt-runtime -t qtapp-qt-runtime:local .

echo "Base images built: qtapp-qt-dev-env:local, qtapp-qt-qtquick:local, qtapp-qt-qtquick-dev:local, qtapp-qt-runtime:local"