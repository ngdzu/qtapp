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

# Find Qt6 packages
find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)

# Optional: spdlog library for high-performance logging
# Can be enabled with: cmake -DZ_MONITOR_USE_SPDLOG=ON ..
option(Z_MONITOR_USE_SPDLOG "Enable spdlog backend for high-performance logging" OFF)

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

# Schema Management: Generate SchemaInfo.h from YAML
# Find Python3 (required for schema generation)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Add custom target to generate schema
add_custom_target(generate_schema
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_schema.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating schema constants from YAML"
    BYPRODUCTS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure/persistence/generated/SchemaInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/schema/generated/ddl/create_tables.sql
        ${CMAKE_CURRENT_SOURCE_DIR}/schema/generated/ddl/create_indices.sql
    VERBATIM
)

# Ensure generated directory exists
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure/persistence/generated)

# Add source code subdirectory (handles all DDD layers)
add_subdirectory(src)

# Add tests subdirectory (optional, can be disabled)
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Ensure schema is generated before building main executable
add_dependencies(z-monitor generate_schema)

# Install main executable
install(TARGETS z-monitor RUNTIME DESTINATION /opt/z-monitor/)


