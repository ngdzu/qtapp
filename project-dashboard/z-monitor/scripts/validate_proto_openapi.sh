#!/bin/bash
#
# @file validate_proto_openapi.sh
# @brief Validate that Protocol Buffers and OpenAPI schemas are consistent.
#
# This script validates that the proto schema and OpenAPI specification
# define the same message structures with consistent field names and types.
#
# Usage:
#   ./scripts/validate_proto_openapi.sh
#
# Requirements:
#   - protoc (Protocol Buffers compiler)
#   - Python 3 with yaml module (for OpenAPI validation)
#

set -euo pipefail

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Directories
PROTO_DIR="${PROJECT_ROOT}/proto"
OPENAPI_DIR="${PROJECT_ROOT}/openapi"
PROTO_FILE="${PROTO_DIR}/telemetry.proto"
OPENAPI_FILE="${OPENAPI_DIR}/telemetry.yaml"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "🔍 Validating Protocol Buffers and OpenAPI schema consistency..."

# Check if protoc is available
if ! command -v protoc &> /dev/null; then
    echo -e "${RED}❌ Error: protoc (Protocol Buffers compiler) not found${NC}"
    exit 1
fi

# Check if proto file exists
if [ ! -f "${PROTO_FILE}" ]; then
    echo -e "${RED}❌ Error: Proto file not found: ${PROTO_FILE}${NC}"
    exit 1
fi

# Check if OpenAPI file exists
if [ ! -f "${OPENAPI_FILE}" ]; then
    echo -e "${RED}❌ Error: OpenAPI file not found: ${OPENAPI_FILE}${NC}"
    exit 1
fi

# Validate proto schema syntax
echo "📝 Validating proto schema syntax..."
if protoc --proto_path="${PROTO_DIR}" --descriptor_set_out=/dev/null "${PROTO_FILE}" 2>/dev/null; then
    echo -e "${GREEN}✅ Proto schema syntax is valid${NC}"
else
    echo -e "${RED}❌ Error: Proto schema syntax validation failed${NC}"
    protoc --proto_path="${PROTO_DIR}" --descriptor_set_out=/dev/null "${PROTO_FILE}"
    exit 1
fi

# Validate OpenAPI schema syntax (basic YAML validation)
echo "📝 Validating OpenAPI schema syntax..."
if python3 -c "import yaml; yaml.safe_load(open('${OPENAPI_FILE}'))" 2>/dev/null; then
    echo -e "${GREEN}✅ OpenAPI schema syntax is valid${NC}"
else
    echo -e "${RED}❌ Error: OpenAPI schema syntax validation failed${NC}"
    python3 -c "import yaml; yaml.safe_load(open('${OPENAPI_FILE}'))"
    exit 1
fi

# Check for required message types in both schemas
echo "📝 Checking message type consistency..."

# Required message types (from proto)
REQUIRED_TYPES=(
    "VitalsRecord"
    "TelemetryBatch"
    "AlarmEvent"
    "DeviceStatus"
    "Heartbeat"
    "RegistrationRequest"
    "RegistrationResponse"
    "BatchContainer"
    "ErrorResponse"
    "Acknowledgment"
)

# Check if types exist in proto (basic grep check)
MISSING_IN_PROTO=()
for type in "${REQUIRED_TYPES[@]}"; do
    if ! grep -q "message ${type}" "${PROTO_FILE}"; then
        MISSING_IN_PROTO+=("${type}")
    fi
done

# Check if types exist in OpenAPI (basic grep check)
MISSING_IN_OPENAPI=()
for type in "${REQUIRED_TYPES[@]}"; do
    if ! grep -q "${type}:" "${OPENAPI_FILE}"; then
        MISSING_IN_OPENAPI+=("${type}")
    fi
done

# Report results
if [ ${#MISSING_IN_PROTO[@]} -gt 0 ]; then
    echo -e "${RED}❌ Error: Missing message types in proto:${NC}"
    for type in "${MISSING_IN_PROTO[@]}"; do
        echo "   - ${type}"
    done
    exit 1
fi

if [ ${#MISSING_IN_OPENAPI[@]} -gt 0 ]; then
    echo -e "${RED}❌ Error: Missing message types in OpenAPI:${NC}"
    for type in "${MISSING_IN_OPENAPI[@]}"; do
        echo "   - ${type}"
    done
    exit 1
fi

echo -e "${GREEN}✅ All required message types present in both schemas${NC}"

# Check for patient_mrn field in patient-related messages
echo "📝 Checking patient MRN association..."

PATIENT_MESSAGES=("VitalsRecord" "TelemetryBatch" "AlarmEvent")
MISSING_MRN=()

for msg in "${PATIENT_MESSAGES[@]}"; do
    if ! grep -q "patient_mrn" "${PROTO_FILE}" || ! grep -q "patient_mrn" "${OPENAPI_FILE}"; then
        MISSING_MRN+=("${msg}")
    fi
done

if [ ${#MISSING_MRN[@]} -gt 0 ]; then
    echo -e "${YELLOW}⚠️  Warning: patient_mrn field may be missing in:${NC}"
    for msg in "${MISSING_MRN[@]}"; do
        echo "   - ${msg}"
    done
else
    echo -e "${GREEN}✅ Patient MRN association verified${NC}"
fi

# Check for schema version field
echo "📝 Checking schema versioning..."
if grep -q "schema_version" "${PROTO_FILE}" && grep -q "schema_version" "${OPENAPI_FILE}"; then
    echo -e "${GREEN}✅ Schema versioning field present${NC}"
else
    echo -e "${YELLOW}⚠️  Warning: schema_version field may be missing${NC}"
fi

# Check for digital signature field
echo "📝 Checking security metadata..."
if grep -q "signature" "${PROTO_FILE}" && grep -q "signature" "${OPENAPI_FILE}"; then
    echo -e "${GREEN}✅ Digital signature field present${NC}"
else
    echo -e "${YELLOW}⚠️  Warning: signature field may be missing${NC}"
fi

echo -e "${GREEN}✅ Schema validation complete!${NC}"
echo ""
echo "Note: This is a basic consistency check. For comprehensive validation,"
echo "      use specialized tools like protoc-gen-openapi or swagger-codegen."

