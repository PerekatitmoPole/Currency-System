#include "services/ExportService.hpp"

#include "common/DateTimeUtils.hpp"
#include "common/Errors.hpp"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTextStream>

#include <xlsxwriter.h>

#include <optional>

namespace currency::client::services {

namespace {

enum class ExportFormat {
    Json,
    Csv,
    Xlsx,
};

std::optional<ExportFormat> detectFormat(const QString& path) {
    const auto suffix = QFileInfo(path).suffix().trimmed().toLower();
    if (suffix == "json") {
        return ExportFormat::Json;
    }
    if (suffix == "csv") {
        return ExportFormat::Csv;
    }
    if (suffix == "xlsx") {
        return ExportFormat::Xlsx;
    }
    return std::nullopt;
}

common::Result<void> writeBinaryFile(const QString& path, const QByteArray& payload) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return common::Result<void>::failure(common::Errors::internalError(
            QString("Не удалось открыть файл '%1' для записи.").arg(path),
            file.errorString()));
    }

    if (file.write(payload) != payload.size()) {
        return common::Result<void>::failure(common::Errors::internalError(
            QString("Не удалось записать файл '%1'.").arg(path),
            file.errorString()));
    }

    if (!file.commit()) {
        return common::Result<void>::failure(common::Errors::internalError(
            QString("Не удалось завершить сохранение файла '%1'.").arg(path),
            file.errorString()));
    }

    return common::Result<void>::success();
}

QString escapeCsv(const QString& value) {
    auto escaped = value;
    escaped.replace('"', "\"\"");
    if (escaped.contains(',') || escaped.contains('"') || escaped.contains('\n') || escaped.contains('\r')) {
        escaped = '"' + escaped + '"';
    }
    return escaped;
}

common::Result<void> exportRatesJson(const QList<models::RateViewModel>& rates, const QString& path) {
    QJsonArray items;
    for (const auto& rate : rates) {
        items.push_back(QJsonObject{
            {"source", rate.sourceName},
            {"base", rate.baseCurrency},
            {"quote", rate.quoteCurrency},
            {"rate", rate.rate},
            {"timestamp", common::DateTimeUtils::toIsoUtc(rate.timestamp)},
            {"note", rate.note},
        });
    }

    const QJsonObject root{
        {"dataset", "latest_rates"},
        {"generated_at", common::DateTimeUtils::toIsoUtc(QDateTime::currentDateTimeUtc())},
        {"items", items},
    };

    return writeBinaryFile(path, QJsonDocument(root).toJson(QJsonDocument::Indented));
}

common::Result<void> exportHistoryJson(const QList<models::HistoryPointViewModel>& points, const QString& path) {
    QJsonArray items;
    for (const auto& point : points) {
        items.push_back(QJsonObject{
            {"source", point.sourceName},
            {"base", point.baseCurrency},
            {"quote", point.quoteCurrency},
            {"rate", point.rate},
            {"timestamp", common::DateTimeUtils::toIsoUtc(point.timestamp)},
        });
    }

    const QJsonObject root{
        {"dataset", "history"},
        {"generated_at", common::DateTimeUtils::toIsoUtc(QDateTime::currentDateTimeUtc())},
        {"items", items},
    };

    return writeBinaryFile(path, QJsonDocument(root).toJson(QJsonDocument::Indented));
}

common::Result<void> exportRatesCsv(const QList<models::RateViewModel>& rates, const QString& path) {
    QString output = "source,base,quote,rate,timestamp,note\n";
    for (const auto& rate : rates) {
        output += QString("%1,%2,%3,%4,%5,%6\n")
            .arg(escapeCsv(rate.sourceName))
            .arg(escapeCsv(rate.baseCurrency))
            .arg(escapeCsv(rate.quoteCurrency))
            .arg(QString::number(rate.rate, 'f', 6))
            .arg(escapeCsv(common::DateTimeUtils::toIsoUtc(rate.timestamp)))
            .arg(escapeCsv(rate.note));
    }

    return writeBinaryFile(path, output.toUtf8());
}

common::Result<void> exportHistoryCsv(const QList<models::HistoryPointViewModel>& points, const QString& path) {
    QString output = "source,base,quote,rate,timestamp\n";
    for (const auto& point : points) {
        output += QString("%1,%2,%3,%4,%5\n")
            .arg(escapeCsv(point.sourceName))
            .arg(escapeCsv(point.baseCurrency))
            .arg(escapeCsv(point.quoteCurrency))
            .arg(QString::number(point.rate, 'f', 6))
            .arg(escapeCsv(common::DateTimeUtils::toIsoUtc(point.timestamp)));
    }

    return writeBinaryFile(path, output.toUtf8());
}

template <typename RowWriter>
common::Result<void> exportXlsx(const QString& path, const QString& sheetName, int rowCount, RowWriter writeRow) {
    const auto encodedPath = QFile::encodeName(path);
    auto* workbook = workbook_new(encodedPath.constData());
    if (workbook == nullptr) {
        return common::Result<void>::failure(common::Errors::internalError(
            QString("Не удалось создать книгу Excel '%1'.").arg(path), {}));
    }

    auto* worksheet = workbook_add_worksheet(workbook, sheetName.toUtf8().constData());
    if (worksheet == nullptr) {
        workbook_close(workbook);
        return common::Result<void>::failure(common::Errors::internalError(
            QString("Не удалось создать лист в книге '%1'.").arg(path), {}));
    }

    for (int row = 0; row < rowCount; ++row) {
        writeRow(worksheet, row);
    }

    const auto error = workbook_close(workbook);
    if (error != LXW_NO_ERROR) {
        return common::Result<void>::failure(common::Errors::internalError(
            QString("Не удалось завершить сохранение книги '%1'.").arg(path),
            QString::fromUtf8(lxw_strerror(error))));
    }

    return common::Result<void>::success();
}

common::Result<void> exportRatesXlsx(const QList<models::RateViewModel>& rates, const QString& path) {
    return exportXlsx(path, "Rates", rates.size() + 1, [&rates](lxw_worksheet* worksheet, const int row) {
        if (row == 0) {
            worksheet_write_string(worksheet, 0, 0, "Source", nullptr);
            worksheet_write_string(worksheet, 0, 1, "Base", nullptr);
            worksheet_write_string(worksheet, 0, 2, "Quote", nullptr);
            worksheet_write_string(worksheet, 0, 3, "Rate", nullptr);
            worksheet_write_string(worksheet, 0, 4, "Timestamp", nullptr);
            worksheet_write_string(worksheet, 0, 5, "Note", nullptr);
            return;
        }

        const auto& item = rates.at(row - 1);
        const auto source = item.sourceName.toUtf8();
        const auto base = item.baseCurrency.toUtf8();
        const auto quote = item.quoteCurrency.toUtf8();
        const auto timestamp = common::DateTimeUtils::toIsoUtc(item.timestamp).toUtf8();
        const auto note = item.note.toUtf8();

        worksheet_write_string(worksheet, row, 0, source.constData(), nullptr);
        worksheet_write_string(worksheet, row, 1, base.constData(), nullptr);
        worksheet_write_string(worksheet, row, 2, quote.constData(), nullptr);
        worksheet_write_number(worksheet, row, 3, item.rate, nullptr);
        worksheet_write_string(worksheet, row, 4, timestamp.constData(), nullptr);
        worksheet_write_string(worksheet, row, 5, note.constData(), nullptr);
    });
}

common::Result<void> exportHistoryXlsx(const QList<models::HistoryPointViewModel>& points, const QString& path) {
    return exportXlsx(path, "History", points.size() + 1, [&points](lxw_worksheet* worksheet, const int row) {
        if (row == 0) {
            worksheet_write_string(worksheet, 0, 0, "Source", nullptr);
            worksheet_write_string(worksheet, 0, 1, "Base", nullptr);
            worksheet_write_string(worksheet, 0, 2, "Quote", nullptr);
            worksheet_write_string(worksheet, 0, 3, "Rate", nullptr);
            worksheet_write_string(worksheet, 0, 4, "Timestamp", nullptr);
            return;
        }

        const auto& item = points.at(row - 1);
        const auto source = item.sourceName.toUtf8();
        const auto base = item.baseCurrency.toUtf8();
        const auto quote = item.quoteCurrency.toUtf8();
        const auto timestamp = common::DateTimeUtils::toIsoUtc(item.timestamp).toUtf8();

        worksheet_write_string(worksheet, row, 0, source.constData(), nullptr);
        worksheet_write_string(worksheet, row, 1, base.constData(), nullptr);
        worksheet_write_string(worksheet, row, 2, quote.constData(), nullptr);
        worksheet_write_number(worksheet, row, 3, item.rate, nullptr);
        worksheet_write_string(worksheet, row, 4, timestamp.constData(), nullptr);
    });
}

}

common::Result<void> ExportService::exportRates(const QList<models::RateViewModel>& rates, const QString& path) const {
    if (rates.isEmpty()) {
        return common::Result<void>::failure(common::Errors::validationError(
            "Нет данных по курсам для экспорта.", {}));
    }

    const auto format = detectFormat(path);
    if (!format.has_value()) {
        return common::Result<void>::failure(common::Errors::validationError(
            "Неподдерживаемый формат экспорта. Используйте .json, .csv или .xlsx.", path));
    }

    switch (*format) {
    case ExportFormat::Json:
        return exportRatesJson(rates, path);
    case ExportFormat::Csv:
        return exportRatesCsv(rates, path);
    case ExportFormat::Xlsx:
        return exportRatesXlsx(rates, path);
    }

    return common::Result<void>::failure(common::Errors::internalError("Неподдерживаемый формат экспорта.", path));
}

common::Result<void> ExportService::exportHistory(const QList<models::HistoryPointViewModel>& points, const QString& path) const {
    if (points.isEmpty()) {
        return common::Result<void>::failure(common::Errors::validationError(
            "Нет исторических данных для экспорта.", {}));
    }

    const auto format = detectFormat(path);
    if (!format.has_value()) {
        return common::Result<void>::failure(common::Errors::validationError(
            "Неподдерживаемый формат экспорта. Используйте .json, .csv или .xlsx.", path));
    }

    switch (*format) {
    case ExportFormat::Json:
        return exportHistoryJson(points, path);
    case ExportFormat::Csv:
        return exportHistoryCsv(points, path);
    case ExportFormat::Xlsx:
        return exportHistoryXlsx(points, path);
    }

    return common::Result<void>::failure(common::Errors::internalError("Неподдерживаемый формат экспорта.", path));
}

}
