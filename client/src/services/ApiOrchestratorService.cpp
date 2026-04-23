#include "services/ApiOrchestratorService.hpp"

#include "common/DateTimeUtils.hpp"

#include <algorithm>
#include <QDateTime>

namespace currency::client::services {

namespace {

common::Error toClientError(const dto::ServerEnvelopeDto& envelope, const QString& action) {
    const auto message = envelope.errorMessage.isEmpty()
        ? QString("Сервер не смог выполнить действие: %1.").arg(action)
        : envelope.errorMessage;

    if (envelope.errorCode == "validation_error" || envelope.errorCode == "not_found") {
        return common::Errors::validationError(message, envelope.errorCode);
    }
    if (envelope.errorCode == "provider_unavailable") {
        return common::Errors::validationError(message, envelope.errorCode);
    }
    if (envelope.errorCode == "protocol_error") {
        return common::Errors::protocolError(message, envelope.errorCode);
    }

    return common::Errors::internalError(message, envelope.errorCode);
}

common::Result<int> readIntField(const dto::FieldMap& fields, const QString& key) {
    bool ok = false;
    const auto value = fields.value(key).toInt(&ok);
    if (!ok) {
        return common::Result<int>::failure(common::Errors::protocolError(
            QString("Поле ответа сервера '%1' имеет неверный формат.").arg(key), fields.value(key)));
    }
    return common::Result<int>::success(value);
}

common::Result<double> readDoubleField(const dto::FieldMap& fields, const QString& key) {
    bool ok = false;
    const auto value = fields.value(key).toDouble(&ok);
    if (!ok) {
        return common::Result<double>::failure(common::Errors::protocolError(
            QString("Поле ответа сервера '%1' имеет неверный формат.").arg(key), fields.value(key)));
    }
    return common::Result<double>::success(value);
}

QDateTime endOfDayUtc(const QDate& value) {
    return QDateTime(value, QTime(23, 59, 59), Qt::UTC);
}

QDateTime startOfDayUtc(const QDate& value) {
    return QDateTime(value, QTime(0, 0), Qt::UTC);
}

QDateTime parseTimestampOrNow(const QString& rawValue) {
    const auto parsed = common::DateTimeUtils::parseIsoUtc(rawValue);
    return parsed.isValid() ? parsed : QDateTime::currentDateTimeUtc();
}

}

ApiOrchestratorService::ApiOrchestratorService(gateways::ServerGateway& gateway)
    : gateway_(gateway) {}

common::Result<QList<models::CurrencyViewModel>> ApiOrchestratorService::fetchCurrencies(const std::optional<dto::ApiSource> source) {
    const auto provider = source.has_value() ? dto::toStableKey(*source) : QString{};
    const auto envelope = ensureSuccess(gateway_.sendGetCurrencies(provider), "загрузка каталога валют");
    if (!envelope.ok()) {
        return common::Result<QList<models::CurrencyViewModel>>::failure(envelope.error());
    }

    const auto count = readIntField(envelope.value().fields, "currency_count");
    if (!count.ok()) {
        return common::Result<QList<models::CurrencyViewModel>>::failure(count.error());
    }

    QList<models::CurrencyViewModel> currencies;
    currencies.reserve(count.value());

    for (int index = 0; index < count.value(); ++index) {
        const auto codeKey = QString("currency%1_code").arg(index);
        const auto nameKey = QString("currency%1_name").arg(index);
        const auto unitsKey = QString("currency%1_minor_units").arg(index);
        const auto minorUnits = readIntField(envelope.value().fields, unitsKey);
        if (!minorUnits.ok()) {
            return common::Result<QList<models::CurrencyViewModel>>::failure(minorUnits.error());
        }

        const auto code = envelope.value().fields.value(codeKey).trimmed().toUpper();
        if (code.isEmpty()) {
        return common::Result<QList<models::CurrencyViewModel>>::failure(common::Errors::protocolError(
                QString("В ответе сервера отсутствует поле '%1'.").arg(codeKey), {}));
        }

        currencies.push_back(models::CurrencyViewModel{
            .code = code,
            .name = envelope.value().fields.value(nameKey, code),
            .minorUnits = minorUnits.value(),
        });
    }

    std::sort(currencies.begin(), currencies.end(), [](const auto& left, const auto& right) {
        return left.code < right.code;
    });
    return common::Result<QList<models::CurrencyViewModel>>::success(currencies);
}

common::Result<HistoryFetchResult> ApiOrchestratorService::fetchHistory(
    const dto::ApiSource source,
    const QString& requestedBaseCurrency,
    const QString& requestedQuoteCurrency,
    const QDate& from,
    const QDate& to,
    const QString& step) {
    if (!from.isValid() || !to.isValid()) {
        return common::Result<HistoryFetchResult>::failure(common::Errors::validationError(
            "Период истории указан некорректно.", {}));
    }

    dto::GetHistoryRequestDto request{
        .provider = dto::toStableKey(source),
        .baseCode = requestedBaseCurrency.trimmed().toUpper(),
        .quoteCode = requestedQuoteCurrency.trimmed().toUpper(),
        .from = startOfDayUtc(from),
        .to = endOfDayUtc(to),
        .step = step.trimmed(),
    };

    const auto envelope = ensureSuccess(gateway_.sendGetHistory(request), QString("загрузка истории из %1").arg(dto::toDisplayName(source)));
    if (!envelope.ok()) {
        return common::Result<HistoryFetchResult>::failure(envelope.error());
    }

    const auto count = readIntField(envelope.value().fields, "point_count");
    if (!count.ok()) {
        return common::Result<HistoryFetchResult>::failure(count.error());
    }

    HistoryFetchResult result;
    result.warnings = extractWarnings(envelope.value(), dto::toDisplayName(source));
    result.points.reserve(count.value());

    for (int index = 0; index < count.value(); ++index) {
        const auto rateKey = QString("point%1_rate").arg(index);
        const auto value = readDoubleField(envelope.value().fields, rateKey);
        if (!value.ok()) {
            return common::Result<HistoryFetchResult>::failure(value.error());
        }

        result.points.push_back(dto::NormalizedHistoryPointDto{
            .source = source,
            .providerName = dto::toDisplayName(source),
            .baseCurrency = envelope.value().fields.value("base_code", request.baseCode).trimmed().toUpper(),
            .quoteCurrency = envelope.value().fields.value("quote_code", request.quoteCode).trimmed().toUpper(),
            .rate = value.value(),
            .timestamp = parseTimestampOrNow(envelope.value().fields.value(QString("point%1_timestamp").arg(index))),
        });
    }

    return common::Result<HistoryFetchResult>::success(result);
}

common::Result<dto::ConversionResultDto> ApiOrchestratorService::convert(
    const dto::ApiSource source,
    const QString& fromCurrency,
    const QString& toCurrency,
    const double amount) {
    dto::ConvertRequestDto request{
        .provider = dto::toStableKey(source),
        .fromCurrency = fromCurrency.trimmed().toUpper(),
        .toCurrency = toCurrency.trimmed().toUpper(),
        .amount = amount,
    };

    const auto envelope = ensureSuccess(gateway_.sendConvert(request), QString("конвертация через %1").arg(dto::toDisplayName(source)));
    if (!envelope.ok()) {
        return common::Result<dto::ConversionResultDto>::failure(envelope.error());
    }

    const auto rate = readDoubleField(envelope.value().fields, "rate");
    if (!rate.ok()) {
        return common::Result<dto::ConversionResultDto>::failure(rate.error());
    }
    const auto result = readDoubleField(envelope.value().fields, "result");
    if (!result.ok()) {
        return common::Result<dto::ConversionResultDto>::failure(result.error());
    }

    dto::ConversionResultDto conversion{
        .sourceName = dto::toDisplayName(source),
        .providerKey = envelope.value().fields.value("provider", request.provider),
        .fromCurrency = envelope.value().fields.value("from_currency", request.fromCurrency).trimmed().toUpper(),
        .toCurrency = envelope.value().fields.value("to_currency", request.toCurrency).trimmed().toUpper(),
        .amount = amount,
        .rate = rate.value(),
        .result = result.value(),
        .timestamp = parseTimestampOrNow(envelope.value().fields.value("timestamp")),
        .stale = envelope.value().fields.value("stale").compare("true", Qt::CaseInsensitive) == 0,
        .warning = envelope.value().fields.value("warning"),
    };

    return common::Result<dto::ConversionResultDto>::success(conversion);
}

common::Result<dto::ServerEnvelopeDto> ApiOrchestratorService::ensureSuccess(
    common::Result<dto::ServerEnvelopeDto> response,
    const QString& action) const {
    if (!response.ok()) {
        return response;
    }

    if (response.value().status == "ok") {
        return response;
    }

    return common::Result<dto::ServerEnvelopeDto>::failure(toClientError(response.value(), action));
}

QList<common::Error> ApiOrchestratorService::extractWarnings(
    const dto::ServerEnvelopeDto& envelope,
    const QString& sourceName) const {
    QList<common::Error> warnings;
    if (envelope.fields.value("stale").compare("true", Qt::CaseInsensitive) != 0) {
        return warnings;
    }

    const auto warning = envelope.fields.value("warning");
    warnings.push_back(common::Errors::networkError(
        warning.isEmpty()
            ? QString("%1 вернул сохраненные данные.").arg(sourceName)
            : QString("%1: %2").arg(sourceName, warning),
        warning));
    return warnings;
}

}
