cmake_minimum_required(VERSION 3.16)
project(z-monitor VERSION 0.1.0 LANGUAGES CXX)

# Set project root directory (used by all subdirectories)
set(Z_MONITOR_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(Z_MONITOR_SOURCE_DIR ${Z_MONITOR_ROOT}/src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 Path Configuration
# Supports Qt installation via environment variable CMAKE_PREFIX_PATH or QT6_DIR
# Default location: /Users/dustinwind/Qt/6.9.2/macos (macOS)
# Usage: export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"
# Or: export QT6_DIR="/Users/dustinwind/Qt/6.9.2/macos"
if(DEFINED ENV{QT6_DIR})
    set(CMAKE_PREFIX_PATH "$ENV{QT6_DIR}" ${CMAKE_PREFIX_PATH})
    message(STATUS "Using Qt6 from QT6_DIR: $ENV{QT6_DIR}")
elseif(DEFINED ENV{CMAKE_PREFIX_PATH})
    message(STATUS "Using Qt6 from CMAKE_PREFIX_PATH: $ENV{CMAKE_PREFIX_PATH}")
else()
    # Try default macOS location
    if(APPLE AND EXISTS "/Users/dustinwind/Qt/6.9.2/macos")
        set(CMAKE_PREFIX_PATH "/Users/dustinwind/Qt/6.9.2/macos" ${CMAKE_PREFIX_PATH})
        message(STATUS "Using default Qt6 location: /Users/dustinwind/Qt/6.9.2/macos")
    else()
        message(WARNING "Qt6 path not configured. Set CMAKE_PREFIX_PATH or QT6_DIR environment variable.")
        message(WARNING "Example: export CMAKE_PREFIX_PATH=\"/path/to/Qt/6.x.x/macos:\$CMAKE_PREFIX_PATH\"")
    endif()
endif()

# vcpkg Configuration (if required)
# Check if vcpkg toolchain is specified
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    message(STATUS "Using vcpkg toolchain from: $ENV{VCPKG_ROOT}")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    message(STATUS "Using vcpkg toolchain from project directory")
endif()

# Find Qt6 packages
find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 Test REQUIRED)

# Optional: spdlog library for high-performance logging
# Can be enabled with: cmake -DZ_MONITOR_USE_SPDLOG=ON ..
option(Z_MONITOR_USE_SPDLOG "Enable spdlog backend for high-performance logging" OFF)

# Optional: QxOrm library for object-relational mapping
# Can be enabled with: cmake -DUSE_QXORM=ON ..
option(USE_QXORM "Enable QxOrm for ORM support (hybrid approach with manual SQL)" OFF)

# Optional: SQLCipher for database encryption
# Can be enabled with: cmake -DENABLE_SQLCIPHER=ON ..
# Note: Requires SQLCipher library to be installed separately
option(ENABLE_SQLCIPHER "Enable SQLCipher for database encryption (requires SQLCipher library)" OFF)

if(Z_MONITOR_USE_SPDLOG)
    # Try to find spdlog using FetchContent (recommended for header-only)
    include(FetchContent)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    FetchContent_MakeAvailable(spdlog)

    # Alternative: Use find_package if spdlog is installed system-wide
    # find_package(spdlog REQUIRED)
    message(STATUS "spdlog backend enabled")
    add_compile_definitions(Z_MONITOR_USE_SPDLOG)
else()
    message(STATUS "spdlog backend disabled (using CustomBackend)")
endif()

# Qt6::Sql is required for database operations
find_package(Qt6 COMPONENTS Sql REQUIRED)

if(USE_QXORM)
    # Try to find QxOrm using find_package first (if installed system-wide)
    find_package(QxOrm QUIET)

    if(NOT QxOrm_FOUND)
        # Use FetchContent to download QxOrm from GitHub
        include(FetchContent)
        FetchContent_Declare(
            qxorm
            GIT_REPOSITORY https://github.com/QxOrm/QxOrm.git
            GIT_TAG v1.4.5
        )
        FetchContent_MakeAvailable(qxorm)

        message(STATUS "QxOrm enabled (downloaded via FetchContent)")
    else()
        message(STATUS "QxOrm enabled (found via find_package)")
    endif()

    add_compile_definitions(USE_QXORM)
else()
    message(STATUS "QxOrm disabled (using manual SQL only)")
endif()

if(ENABLE_SQLCIPHER)
    # SQLCipher must be installed separately
    # On Ubuntu/Debian: sudo apt-get install libsqlcipher-dev
    # On macOS: brew install sqlcipher
    # On Windows: Download from https://www.zetetic.net/sqlcipher/
    find_package(SQLCipher QUIET)

    if(NOT SQLCipher_FOUND)
        # Try pkg-config as fallback
        find_package(PkgConfig QUIET)

        if(PkgConfig_FOUND)
            pkg_check_modules(SQLCipher QUIET sqlcipher)
        endif()
    endif()

    if(SQLCipher_FOUND)
        message(STATUS "SQLCipher enabled (found via find_package)")
        add_compile_definitions(ENABLE_SQLCIPHER)

    # Note: Qt6::Sql driver must be built with SQLCipher support
    # This typically requires rebuilding Qt with SQLCipher driver
    else()
        message(WARNING "SQLCipher requested but not found. Database encryption will not work.")
        message(WARNING "Install SQLCipher library and rebuild Qt with SQLCipher driver support.")
        message(WARNING "Continuing without SQLCipher support...")
        set(ENABLE_SQLCIPHER OFF)
    endif()
else()
    message(STATUS "SQLCipher disabled (using standard SQLite)")
endif()

# Protocol Buffers: Required for telemetry schema code generation
# Try to find protobuf using find_package first (if installed system-wide)
find_package(Protobuf QUIET)

if(NOT Protobuf_FOUND)
    # Use FetchContent to download protobuf from GitHub
    include(FetchContent)
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG v21.12
    )

    # Set protobuf options before making it available
    set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(protobuf)

    message(STATUS "Protobuf enabled (downloaded via FetchContent)")
else()
    message(STATUS "Protobuf enabled (found via find_package)")
endif()

# Generate C++ classes from telemetry.proto
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure/telemetry/generated)
set(PROTO_FILE ${PROTO_DIR}/telemetry.proto)

# Ensure generated directory exists
file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# Generate proto C++ files
# Note: protobuf_generate_cpp generates files in current binary directory
# We'll use protobuf_generate with OUTPUT_DIRECTORY to control output location
get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
set(PROTO_SRCS ${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc)
set(PROTO_HDRS ${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h)

# Find protoc executable (works with both find_package and FetchContent)
if(TARGET protobuf::protoc)
    # Using FetchContent - protoc is a target
    get_target_property(PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)

    if(NOT PROTOC_EXECUTABLE)
        # If IMPORTED_LOCATION is not set, use the target directly in COMMAND
        set(PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc>)
    endif()
elseif(Protobuf_PROTOC_EXECUTABLE)
    # Using find_package - use the variable
    set(PROTOC_EXECUTABLE ${Protobuf_PROTOC_EXECUTABLE})
else()
    # Fallback: try to find protoc in PATH
    find_program(PROTOC_EXECUTABLE protoc)

    if(NOT PROTOC_EXECUTABLE)
        message(FATAL_ERROR "protoc executable not found. Please install Protocol Buffers compiler.")
    endif()
endif()

# Add custom command to generate proto files
add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --cpp_out=${PROTO_OUTPUT_DIR}
    --proto_path=${PROTO_DIR}
    ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ classes from ${PROTO_FILE}"
)

# Create a library for generated proto files
add_library(z_monitor_proto STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_link_libraries(z_monitor_proto
    PUBLIC
    protobuf::libprotobuf
)

target_include_directories(z_monitor_proto
    PUBLIC
    ${PROTO_OUTPUT_DIR}
    ${PROTO_DIR}
)

# Add custom target for proto generation (for manual regeneration)
add_custom_target(generate_proto
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_proto_cpp.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating C++ classes from telemetry.proto"
    DEPENDS ${PROTO_FILE}
)

# Schema Management: Generate SchemaInfo.h from YAML
# Find Python3 (required for schema generation)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Check if PyYAML is available
set(SCHEMA_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure/persistence/generated/SchemaInfo.h)
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import yaml"
    RESULT_VARIABLE YAML_AVAILABLE
    OUTPUT_QUIET
    ERROR_QUIET
)

# Add custom target to generate schema
# If yaml is not available and SchemaInfo.h exists, create a no-op target
if(YAML_AVAILABLE EQUAL 0)
    # PyYAML is available - generate schema normally
    add_custom_target(generate_schema
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_schema.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating schema constants from YAML"
        BYPRODUCTS
        ${SCHEMA_HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/schema/generated/ddl/create_tables.sql
        ${CMAKE_CURRENT_SOURCE_DIR}/schema/generated/ddl/create_indices.sql
        VERBATIM
    )
else()
    # PyYAML is not available - check if SchemaInfo.h exists
    if(EXISTS ${SCHEMA_HEADER})
        # SchemaInfo.h exists - create a no-op target
        message(STATUS "PyYAML not available, but SchemaInfo.h exists. Skipping schema generation.")
        add_custom_target(generate_schema
            COMMAND ${CMAKE_COMMAND} -E echo "SchemaInfo.h already exists, skipping generation - PyYAML not available"
            COMMENT "Skipping schema generation - PyYAML not available, but SchemaInfo.h exists"
        )
    else()
        # SchemaInfo.h doesn't exist and yaml is not available - error
        message(FATAL_ERROR "PyYAML is required for schema generation. Please install it: pip3 install pyyaml")
    endif()
endif()

# Ensure generated directory exists
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure/persistence/generated)

# Add source code subdirectory (handles all DDD layers)
add_subdirectory(src)

# Generate query reference documentation (after src is added so we can link to libraries)
add_executable(generate_query_reference
    scripts/generate_query_reference.cpp
)

target_link_libraries(generate_query_reference
    PRIVATE
    z_monitor_infrastructure
    z_monitor_domain
    Qt6::Core
    Qt6::Sql
)

target_include_directories(generate_query_reference
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Ensure infrastructure is built before generate_query_reference
add_dependencies(generate_query_reference z_monitor_infrastructure)

add_custom_target(generate_query_docs
    COMMAND $<TARGET_FILE:generate_query_reference> > ${CMAKE_CURRENT_SOURCE_DIR}/doc/QUERY_REFERENCE.md
    DEPENDS generate_query_reference
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating QUERY_REFERENCE.md from QueryCatalog"
)

# Add tests subdirectory (optional, can be disabled)
option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install main executable
install(TARGETS z-monitor RUNTIME DESTINATION /opt/z-monitor/)

# Build test_sensor_integration (standalone integration test for SharedMemorySensorDataSource)
add_executable(test_sensor_integration
    test_sensor_integration.cpp
)

target_link_libraries(test_sensor_integration
    PRIVATE
    z_monitor_infrastructure
    z_monitor_domain
    Qt6::Core
)

target_include_directories(test_sensor_integration
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Ensure infrastructure is built before test_sensor_integration
add_dependencies(test_sensor_integration z_monitor_infrastructure)

# Build test_shared_memory_connection (minimal C++ test for shared memory connection)
add_executable(test_shared_memory_connection
    test_shared_memory_connection.cpp
)

# No Qt dependencies - pure C++ with Unix sockets
target_include_directories(test_shared_memory_connection
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Build test_header_size (verify RingBufferHeader layout compatibility)
add_executable(test_header_size
    test_header_size.cpp
)

target_include_directories(test_header_size
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
