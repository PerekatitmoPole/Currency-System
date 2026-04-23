file(GLOB_RECURSE CURRENCY_CLIENT_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.hpp
)

set(CURRENCY_CLIENT_LIBRARY_SOURCES ${CURRENCY_CLIENT_SOURCES})
list(REMOVE_ITEM CURRENCY_CLIENT_LIBRARY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

set(QT_CMAKE_EXPORT_NAMESPACE Qt6)

qt_add_library(currency_client_lib STATIC
    ${CURRENCY_CLIENT_LIBRARY_SOURCES}
)

set_target_properties(currency_client_lib PROPERTIES
    AUTOMOC ON
    AUTORCC OFF
    AUTOUIC OFF
)

target_include_directories(currency_client_lib PUBLIC src)
target_link_libraries(currency_client_lib PUBLIC currency_qt_dependencies)

if(MSVC)
    target_compile_options(currency_client_lib PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(currency_client_lib PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

qt_add_executable(currency_client
    src/main.cpp
    resources.qrc
)

set_target_properties(currency_client PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC OFF
)

target_link_libraries(currency_client PRIVATE currency_client_lib)

if(MSVC)
    target_compile_options(currency_client PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(currency_client PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(WIN32)
    get_filename_component(QT_RUNTIME_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_RUNTIME_BIN_DIR}")

    function(currency_deploy_qt_runtime target_name)
        if(WINDEPLOYQT_EXECUTABLE)
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations
                    "$<TARGET_FILE:${target_name}>"
                COMMENT "Deploying Qt runtime for ${target_name}"
                VERBATIM
            )
        endif()
    endfunction()

    currency_deploy_qt_runtime(currency_client)
endif()

if(BUILD_TESTS)
    qt_add_executable(currency_client_tests
        tests/ApiOrchestratorServiceTests.cpp
        tests/ClientTestSupport.hpp
        tests/ClientTests.cpp
        tests/ExportServiceTests.cpp
        tests/HistoryChartWidgetTests.cpp
        tests/ServerGatewayTests.cpp
    )

    set_target_properties(currency_client_tests PROPERTIES
        AUTOMOC ON
        AUTORCC OFF
        AUTOUIC OFF
    )

    target_include_directories(currency_client_tests PRIVATE src tests)
    target_link_libraries(currency_client_tests PRIVATE
        currency_client_lib
        currency_qt_test_dependencies
        GTest::gtest
    )

    if(MSVC)
        target_compile_options(currency_client_tests PRIVATE /utf-8)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(currency_client_tests PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()

    include(GoogleTest)
    gtest_discover_tests(currency_client_tests
        DISCOVERY_TIMEOUT 60
        TEST_LIST CURRENCY_CLIENT_DISCOVERED_TESTS
    )

    if(WIN32)
        currency_deploy_qt_runtime(currency_client_tests)

        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        set(_currency_client_test_env "PATH=${QT_RUNTIME_BIN_DIR};${MINGW_BIN_DIR};$ENV{PATH}")
        if(CURRENCY_CLIENT_DISCOVERED_TESTS)
            set_tests_properties(${CURRENCY_CLIENT_DISCOVERED_TESTS} PROPERTIES
                ENVIRONMENT "${_currency_client_test_env}"
                TIMEOUT 90
            )
        endif()
    elseif(CURRENCY_CLIENT_DISCOVERED_TESTS)
        set_tests_properties(${CURRENCY_CLIENT_DISCOVERED_TESTS} PROPERTIES TIMEOUT 90)
    endif()
endif()
