cmake_minimum_required(VERSION 3.24)
project(CurrencySystemServer VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests" ON)
set(MIN_BOOST_VERSION "1.75.0")
set(BOOST_BOOTSTRAP_VERSION "1.87.0")

find_package(Threads REQUIRED)
include(FetchContent)

set(BOOST_HINT_DIRS
    "${BOOST_ROOT}"
    "${Boost_ROOT}"
    "$ENV{BOOST_ROOT}"
    "$ENV{Boost_ROOT}"
    "${PROJECT_SOURCE_DIR}/third_party/boost"
    "${PROJECT_SOURCE_DIR}/external/boost"
    "C:/local/boost_1_87_0"
    "C:/local/boost_1_86_0"
    "C:/local/boost_1_85_0"
    "C:/local/boost_1_84_0"
    "C:/local/boost_1_83_0"
    "C:/local/boost_1_82_0"
    "C:/local/boost_1_81_0"
)

find_path(BOOST_INCLUDE_DIR
    NAMES boost/version.hpp
    HINTS ${BOOST_HINT_DIRS}
    PATH_SUFFIXES include
)

if(NOT BOOST_INCLUDE_DIR)
    message(STATUS "Boost headers were not found locally. Fetching Boost ${BOOST_BOOTSTRAP_VERSION} automatically...")
    string(REPLACE "." "_" BOOST_BOOTSTRAP_VERSION_UNDERSCORE "${BOOST_BOOTSTRAP_VERSION}")
    FetchContent_Declare(
        boost_headers
        URL https://archives.boost.io/release/${BOOST_BOOTSTRAP_VERSION}/source/boost_${BOOST_BOOTSTRAP_VERSION_UNDERSCORE}.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(boost_headers)
    set(BOOST_INCLUDE_DIR "${boost_headers_SOURCE_DIR}")
endif()

file(READ "${BOOST_INCLUDE_DIR}/boost/version.hpp" BOOST_VERSION_HEADER)
string(REGEX MATCH "#define[ \t]+BOOST_LIB_VERSION[ \t]+\"([0-9_]+)\"" _boost_lib_version_match "${BOOST_VERSION_HEADER}")
set(BOOST_LIB_VERSION_RAW "${CMAKE_MATCH_1}")
string(REPLACE "_" "." BOOST_VERSION_STRING "${BOOST_LIB_VERSION_RAW}")

if(BOOST_VERSION_STRING VERSION_LESS MIN_BOOST_VERSION)
    message(FATAL_ERROR
        "Boost version ${BOOST_VERSION_STRING} is too old. Minimum required version is ${MIN_BOOST_VERSION}."
    )
endif()

message(STATUS "Using Boost headers from: ${BOOST_INCLUDE_DIR}")
message(STATUS "Detected Boost version: ${BOOST_VERSION_STRING}")

set(CURRENCY_CORE_SOURCES
        server/src/app/Application.cpp
        server/src/common/TimeUtils.cpp
        server/src/common/Validation.cpp
        server/src/serialization/TextProtocolSerializer.cpp
        server/src/repositories/InMemoryCurrencyRepository.cpp
        server/src/repositories/InMemoryQuoteRepository.cpp
        server/src/repositories/InMemoryHistoryRepository.cpp
        server/src/repositories/InMemoryQueryCache.cpp
        server/src/services/IngestionService.cpp
        server/src/services/CurrencyQueryService.cpp
        server/src/services/QuoteQueryService.cpp
        server/src/services/HistoryQueryService.cpp
        server/src/services/ConversionService.cpp
        server/src/controllers/IngestionController.cpp
        server/src/controllers/CurrencyController.cpp
        server/src/controllers/QuoteController.cpp
        server/src/controllers/HistoryController.cpp
        server/src/controllers/ConversionController.cpp
        server/src/network/RequestRouter.cpp
        server/src/network/Session.cpp
        server/src/network/TcpServer.cpp
)

add_library(currency_core STATIC ${CURRENCY_CORE_SOURCES})
target_include_directories(currency_core PUBLIC server/src ${BOOST_INCLUDE_DIR})
target_link_libraries(currency_core PUBLIC Threads::Threads)

if(WIN32)
    target_link_libraries(currency_core PUBLIC ws2_32 mswsock)
endif()

add_executable(currency_server server/src/main.cpp)
target_link_libraries(currency_server PRIVATE currency_core)

if(BUILD_TESTS)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(currency_tests
            server/tests/QuoteHistoryTests.cpp
            server/tests/TextProtocolSerializerTests.cpp
            server/tests/CommonTests.cpp
            server/tests/RepositoryTests.cpp
            server/tests/ServiceTests.cpp
            server/tests/ControllerTests.cpp
            server/tests/RequestRouterTests.cpp
            server/tests/NetworkIntegrationTests.cpp
            server/tests/TestHelpers.hpp
    )

    target_include_directories(currency_tests PRIVATE server/src ${BOOST_INCLUDE_DIR})
    target_link_libraries(currency_tests PRIVATE currency_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(currency_tests)
endif()