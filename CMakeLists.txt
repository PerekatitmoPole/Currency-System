cmake_minimum_required(VERSION 3.24)

project(CurrencySystem VERSION 2.0.0 LANGUAGES CXX C)

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_CLIENT "Build Qt client" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(BUILD_TESTS)
    enable_testing()
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)
include(Dependencies)

currency_prepare_dependencies()

add_subdirectory(server)

if(BUILD_CLIENT)
    if(CURRENCY_ENABLE_CLIENT)
        add_subdirectory(client)
    else()
        message(WARNING
            "Qt6 was not found. Skipping currency_client. "
            "Set CMAKE_PREFIX_PATH or Qt6_DIR to a Qt 6 installation to enable the client build."
        )
    endif()
endif()
