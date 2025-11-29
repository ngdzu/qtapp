#!/bin/bash
#
# @file generate_proto_cpp.sh
# @brief Generate C++ classes from Protocol Buffers schema.
#
# This script generates C++ classes from telemetry.proto using protoc.
# The generated files are placed in src/infrastructure/telemetry/generated/.
#
# Usage:
#   ./scripts/generate_proto_cpp.sh
#
# Requirements:
#   - protoc (Protocol Buffers compiler) must be installed
#   - protobuf C++ runtime library must be available
#

set -euo pipefail

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Directories
PROTO_DIR="${PROJECT_ROOT}/proto"
OUTPUT_DIR="${PROJECT_ROOT}/src/infrastructure/telemetry/generated"
PROTO_FILE="${PROTO_DIR}/telemetry.proto"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "🔧 Generating C++ classes from Protocol Buffers schema..."

# Check if protoc is available
if ! command -v protoc &> /dev/null; then
    echo -e "${RED}❌ Error: protoc (Protocol Buffers compiler) not found${NC}"
    echo "Install protoc:"
    echo "  macOS:   brew install protobuf"
    echo "  Ubuntu:   sudo apt-get install protobuf-compiler"
    echo "  Windows: Download from https://github.com/protocolbuffers/protobuf/releases"
    exit 1
fi

# Check if proto file exists
if [ ! -f "${PROTO_FILE}" ]; then
    echo -e "${RED}❌ Error: Proto file not found: ${PROTO_FILE}${NC}"
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p "${OUTPUT_DIR}"

# Generate C++ classes
echo "📝 Generating C++ classes from ${PROTO_FILE}..."
protoc \
    --cpp_out="${OUTPUT_DIR}" \
    --proto_path="${PROTO_DIR}" \
    "${PROTO_FILE}"

# Check if generation was successful
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ Successfully generated C++ classes${NC}"
    echo "   Output directory: ${OUTPUT_DIR}"
    echo "   Generated files:"
    ls -lh "${OUTPUT_DIR}"/*.pb.h "${OUTPUT_DIR}"/*.pb.cc 2>/dev/null || true
else
    echo -e "${RED}❌ Error: Failed to generate C++ classes${NC}"
    exit 1
fi

# Verify generated files exist
if [ ! -f "${OUTPUT_DIR}/telemetry.pb.h" ] || [ ! -f "${OUTPUT_DIR}/telemetry.pb.cc" ]; then
    echo -e "${YELLOW}⚠️  Warning: Expected generated files not found${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Code generation complete!${NC}"

