cmake_minimum_required(VERSION 3.16)
project(pgw_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(include)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ------------------------------------------------------------------------------
# Common library
# ------------------------------------------------------------------------------
add_library(pgw_common STATIC
    src/network/udp_client.cpp
    src/network/udp_server.cpp
    src/utils/logger.cpp
    src/utils/bcd_converter.cpp
    src/config/client_config.cpp
    src/config/server_config.cpp
    src/cdr/cdr_manager.cpp
    src/session/session_manager.cpp
)

target_link_libraries(pgw_common
    PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    httplib::httplib
    pthread
)

# ------------------------------------------------------------------------------
# Main executables
# ------------------------------------------------------------------------------
# Server
add_executable(pgw_server
    src/server_main.cpp 
    src/pgw/pgw_server.cpp
    src/http/http_server.cpp
)

target_link_libraries(pgw_server
    PRIVATE
    pgw_common
)

# Client
add_executable(pgw_client
    src/client_main.cpp 
    src/pgw/pgw_client.cpp
)

target_link_libraries(pgw_client
    PRIVATE
    pgw_common
)

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------
# Unit Tests
add_executable(unit_tests
    tests/unit/test_bcd_converter.cpp
    tests/unit/test_session_manager.cpp
    tests/unit/test_logger.cpp
    tests/unit/test_config.cpp
)

target_link_libraries(unit_tests
    PRIVATE
    pgw_common
    GTest::gtest_main
    GTest::gmock_main
)

# Integration Tests
add_executable(integration_tests
    tests/integration/test_server_client.cpp
)

target_link_libraries(integration_tests
    PRIVATE
    pgw_common
    GTest::gtest_main
)

# Load Test
add_executable(mass_test
    tests/load/mass_test.cpp
    src/pgw/pgw_client.cpp
)

target_link_libraries(mass_test
    PRIVATE
    pgw_common
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
gtest_discover_tests(integration_tests)
