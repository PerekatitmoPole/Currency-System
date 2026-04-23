#include "ClientTestSupport.hpp"
#include "services/ExportService.hpp"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <gtest/gtest.h>

namespace currency::client::tests {

TEST(ExportServiceTests, ExportsRatesToJsonCsvAndXlsx) {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    services::ExportService service;
    const auto rates = sampleRates();

    const auto jsonPath = temporaryDir.filePath("rates.json");
    const auto csvPath = temporaryDir.filePath("rates.csv");
    const auto xlsxPath = temporaryDir.filePath("rates.xlsx");

    ASSERT_TRUE(service.exportRates(rates, jsonPath).ok());
    ASSERT_TRUE(service.exportRates(rates, csvPath).ok());
    ASSERT_TRUE(service.exportRates(rates, xlsxPath).ok());

    QFile jsonFile(jsonPath);
    ASSERT_TRUE(jsonFile.open(QIODevice::ReadOnly));
    const auto document = QJsonDocument::fromJson(jsonFile.readAll());
    ASSERT_TRUE(document.isObject());
    EXPECT_EQ(document.object().value("dataset").toString(), QString("latest_rates"));
    EXPECT_EQ(document.object().value("items").toArray().size(), 2);

    QFile csvFile(csvPath);
    ASSERT_TRUE(csvFile.open(QIODevice::ReadOnly));
    const auto csvPayload = QString::fromUtf8(csvFile.readAll());
    EXPECT_TRUE(csvPayload.startsWith("source,base,quote,rate,timestamp,note"));
    EXPECT_TRUE(csvPayload.contains("Frankfurter"));

    QFileInfo xlsxInfo(xlsxPath);
    EXPECT_TRUE(xlsxInfo.exists());
    EXPECT_GT(xlsxInfo.size(), 0);
}

TEST(ExportServiceTests, RejectsUnsupportedHistoryFormat) {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    services::ExportService service;
    const auto result = service.exportHistory(sampleHistory(), temporaryDir.filePath("history.txt"));

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("validation_error"));
}

TEST(ExportServiceTests, RejectsEmptyRatesExport) {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    services::ExportService service;
    const auto result = service.exportRates({}, temporaryDir.filePath("rates.json"));

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("validation_error"));
}

TEST(ExportServiceTests, RejectsEmptyHistoryExport) {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    services::ExportService service;
    const auto result = service.exportHistory({}, temporaryDir.filePath("history.json"));

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, QString("validation_error"));
}

TEST(ExportServiceTests, EscapesCsvFields) {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    services::ExportService service;
    QList<models::RateViewModel> rates{
        models::RateViewModel{
            .sourceName = "Provider, Inc",
            .baseCurrency = "EUR",
            .quoteCurrency = "USD",
            .rate = 1.2345,
            .timestamp = QDateTime(QDate(2026, 4, 18), QTime(10, 30), Qt::UTC),
            .note = "line \"quoted\"",
        },
    };

    const auto csvPath = temporaryDir.filePath("rates.csv");
    ASSERT_TRUE(service.exportRates(rates, csvPath).ok());

    QFile csvFile(csvPath);
    ASSERT_TRUE(csvFile.open(QIODevice::ReadOnly));
    const auto payload = QString::fromUtf8(csvFile.readAll());
    EXPECT_TRUE(payload.contains("\"Provider, Inc\""));
    EXPECT_TRUE(payload.contains("\"line \"\"quoted\"\"\""));
}

}
