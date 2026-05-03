include(FetchContent)

set(CURRENCY_MIN_BOOST_VERSION "1.75.0")
set(CURRENCY_BOOST_BOOTSTRAP_VERSION "1.87.0")

function(currency_prepare_dependencies)
    set(boost_hint_dirs
        "${BOOST_ROOT}"
        "${Boost_ROOT}"
        "$ENV{BOOST_ROOT}"
        "$ENV{Boost_ROOT}"
        "${PROJECT_SOURCE_DIR}/third_party/boost"
        "${PROJECT_SOURCE_DIR}/external/boost"
        "${PROJECT_SOURCE_DIR}/cmake-build-debug/_deps/boost_headers-src"
        "${PROJECT_SOURCE_DIR}/build/_deps/boost_headers-src"
        "C:/local/boost_1_87_0"
        "C:/local/boost_1_86_0"
        "C:/local/boost_1_85_0"
        "C:/local/boost_1_84_0"
        "C:/local/boost_1_83_0"
        "C:/local/boost_1_82_0"
        "C:/local/boost_1_81_0"
    )

    find_path(currency_boost_include_dir
        NAMES boost/version.hpp
        HINTS ${boost_hint_dirs}
        PATH_SUFFIXES include
    )

    if(NOT currency_boost_include_dir)
        message(STATUS "Boost headers were not found locally. Fetching Boost ${CURRENCY_BOOST_BOOTSTRAP_VERSION} automatically...")
        string(REPLACE "." "_" boost_version_underscore "${CURRENCY_BOOST_BOOTSTRAP_VERSION}")
        FetchContent_Declare(
            boost_headers
            URL https://archives.boost.io/release/${CURRENCY_BOOST_BOOTSTRAP_VERSION}/source/boost_${boost_version_underscore}.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(boost_headers)
        set(currency_boost_include_dir "${boost_headers_SOURCE_DIR}")
    endif()

    file(READ "${currency_boost_include_dir}/boost/version.hpp" currency_boost_version_header)
    string(REGEX MATCH "#define[ \t]+BOOST_LIB_VERSION[ \t]+\"([0-9_]+)\"" _boost_lib_version_match "${currency_boost_version_header}")
    set(currency_boost_version_raw "${CMAKE_MATCH_1}")
    string(REPLACE "_" "." currency_boost_version_string "${currency_boost_version_raw}")

    if(currency_boost_version_string VERSION_LESS CURRENCY_MIN_BOOST_VERSION)
        message(FATAL_ERROR
            "Boost version ${currency_boost_version_string} is too old. Minimum required version is ${CURRENCY_MIN_BOOST_VERSION}."
        )
    endif()

    message(STATUS "Using Boost headers from: ${currency_boost_include_dir}")
    message(STATUS "Detected Boost version: ${currency_boost_version_string}")

    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_Declare(
        tinyxml2
        URL https://github.com/leethomason/tinyxml2/archive/refs/tags/10.0.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_Declare(
        libxlsxwriter
        URL https://github.com/jmcnamara/libxlsxwriter/archive/refs/tags/v1.2.4.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_Declare(
        zlib
        URL https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    if(EXISTS "${PROJECT_SOURCE_DIR}/vcpkg_installed/x64-mingw-dynamic/include/zlib.h")
        set(ZLIB_ROOT "${PROJECT_SOURCE_DIR}/vcpkg_installed/x64-mingw-dynamic" CACHE STRING "Preferred ZLIB root" FORCE)
    endif()

    find_package(ZLIB QUIET)
    if(NOT ZLIB_FOUND)
        set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(zlib)
    endif()

    set(fetch_targets nlohmann_json tinyxml2 libxlsxwriter)

    if(BUILD_TESTS)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    set(currency_project_build_tests ${BUILD_TESTS})
    if(DEFINED BUILD_TESTING)
        set(currency_project_build_testing ${BUILD_TESTING})
        set(currency_has_build_testing ON)
    else()
        set(currency_has_build_testing OFF)
    endif()

    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_FUZZERS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(${fetch_targets})

    set(BUILD_TESTS ${currency_project_build_tests} CACHE BOOL "Build unit tests" FORCE)
    if(currency_has_build_testing)
        set(BUILD_TESTING ${currency_project_build_testing} CACHE BOOL "" FORCE)
    else()
        unset(BUILD_TESTING CACHE)
    endif()

    if(currency_project_build_tests)
        FetchContent_MakeAvailable(googletest)
    endif()

    add_library(currency_project_options INTERFACE)
    target_compile_features(currency_project_options INTERFACE cxx_std_20)

    add_library(currency_server_dependencies INTERFACE)
    target_include_directories(currency_server_dependencies INTERFACE "${currency_boost_include_dir}")
    target_link_libraries(currency_server_dependencies INTERFACE
        Threads::Threads
        currency_project_options
        nlohmann_json::nlohmann_json
        tinyxml2::tinyxml2
    )

    add_library(currency_client_dependencies INTERFACE)
    target_link_libraries(currency_client_dependencies INTERFACE
        currency_project_options
        xlsxwriter
    )

    if(WIN32)
        target_link_libraries(currency_server_dependencies INTERFACE winhttp)
        target_link_libraries(currency_client_dependencies INTERFACE shlwapi)
    else()
        find_package(CURL REQUIRED)
        target_link_libraries(currency_server_dependencies INTERFACE CURL::libcurl)
    endif()

    set(CURRENCY_ENABLE_CLIENT OFF PARENT_SCOPE)
    if(BUILD_CLIENT)
        set(currency_qt_components Widgets Network Charts Xml)
        if(BUILD_TESTS)
            list(APPEND currency_qt_components Test)
        endif()

        find_package(Qt6 QUIET COMPONENTS ${currency_qt_components})
        if(Qt6_FOUND)
            set(QT_VERSION_MAJOR 6 PARENT_SCOPE)
            set(QT_VERSION_MINOR ${Qt6Core_VERSION_MINOR} PARENT_SCOPE)
            set(Qt6Core_VERSION_MAJOR ${Qt6Core_VERSION_MAJOR} PARENT_SCOPE)
            set(Qt6Core_VERSION_MINOR ${Qt6Core_VERSION_MINOR} PARENT_SCOPE)
            add_library(currency_qt_dependencies INTERFACE)
            target_link_libraries(currency_qt_dependencies INTERFACE
                Qt6::Widgets
                Qt6::Network
                Qt6::Charts
                Qt6::Xml
                currency_client_dependencies
            )
            if(BUILD_TESTS AND TARGET Qt6::Test)
                add_library(currency_qt_test_dependencies INTERFACE)
                target_link_libraries(currency_qt_test_dependencies INTERFACE
                    currency_qt_dependencies
                    Qt6::Test
                )
            endif()
            set(CURRENCY_ENABLE_CLIENT ON PARENT_SCOPE)
        endif()
    endif()
endfunction()
