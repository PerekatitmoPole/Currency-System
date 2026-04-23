set(CURRENCY_SERVER_SOURCES
    src/app/Application.cpp
    src/common/TimeUtils.cpp
    src/common/Validation.cpp
    src/controllers/ConversionController.cpp
    src/controllers/CurrencyController.cpp
    src/controllers/HistoryController.cpp
    src/controllers/IngestionController.cpp
    src/controllers/QuoteController.cpp
    src/logging/ConsoleLogSink.cpp
    src/logging/JsonFileLogSink.cpp
    src/logging/LogEntry.cpp
    src/logging/Logger.cpp
    src/network/RequestRouter.cpp
    src/network/Session.cpp
    src/network/TcpServer.cpp
    src/providers/CbrProvider.cpp
    src/providers/EcbProvider.cpp
    src/providers/FrankfurterProvider.cpp
    src/providers/HttpClient.cpp
    src/repositories/InMemoryCurrencyRepository.cpp
    src/repositories/InMemoryHistoryRepository.cpp
    src/repositories/InMemoryQueryCache.cpp
    src/repositories/InMemoryQuoteRepository.cpp
    src/serialization/TextProtocolSerializer.cpp
    src/services/ConversionService.cpp
    src/services/CurrencyQueryService.cpp
    src/services/HistoryQueryService.cpp
    src/services/IngestionService.cpp
    src/services/MarketDataRefreshService.cpp
    src/services/QuoteQueryService.cpp
)

add_library(currency_core STATIC ${CURRENCY_SERVER_SOURCES})
target_include_directories(currency_core PUBLIC src)
target_link_libraries(currency_core PUBLIC currency_server_dependencies)

if(WIN32)
    target_link_libraries(currency_core PUBLIC ws2_32 mswsock)
endif()

add_executable(currency_server src/main.cpp)
target_link_libraries(currency_server PRIVATE currency_core)

if(BUILD_TESTS)
    add_executable(currency_tests
        tests/CommonTests.cpp
        tests/ControllerTests.cpp
        tests/NetworkIntegrationTests.cpp
        tests/QuoteHistoryTests.cpp
        tests/RepositoryTests.cpp
        tests/RequestRouterTests.cpp
        tests/ServerWorkTests.cpp
        tests/ServiceTests.cpp
        tests/TestHelpers.hpp
        tests/TextProtocolSerializerTests.cpp
    )

    target_include_directories(currency_tests PRIVATE src)
    target_link_libraries(currency_tests PRIVATE currency_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(currency_tests)
endif()
