#include "dto/ApiDtos.hpp"

#include <QHash>

#include <array>
#include <type_traits>

namespace currency::client::dto {

namespace {

const std::array<ApiSourceDescriptor, 3> kCatalog{{
    {
        .source = ApiSource::Cbr,
        .stableKey = "cbr",
        .displayName = "ЦБ РФ",
        .shortDescription = "Официальные курсы Банка России. Поддерживает ограниченный перечень валют, публикуемый ЦБ РФ.",
        .baseUrl = "https://www.cbr.ru/",
        .requiresApiKey = false,
        .availableViaServer = true,
        .supportsLatest = true,
        .supportsHistory = true,
        .supportsCurrencyCatalog = true,
    },
    {
        .source = ApiSource::Ecb,
        .stableKey = "ecb",
        .displayName = "ECB",
        .shortDescription = "Справочные курсы Европейского центрального банка относительно EUR.",
        .baseUrl = "https://www.ecb.europa.eu/",
        .requiresApiKey = false,
        .availableViaServer = true,
        .supportsLatest = true,
        .supportsHistory = true,
        .supportsCurrencyCatalog = true,
    },
    {
        .source = ApiSource::Frankfurter,
        .stableKey = "frankfurter",
        .displayName = "Frankfurter",
        .shortDescription = "Открытый REST-сервис для актуальных курсов, истории и справочника валют.",
        .baseUrl = "https://api.frankfurter.dev/",
        .requiresApiKey = false,
        .availableViaServer = true,
        .supportsLatest = true,
        .supportsHistory = true,
        .supportsCurrencyCatalog = true,
    },
}};

const ApiSourceDescriptor& findDescriptor(const ApiSource source) {
    for (const auto& descriptor : kCatalog) {
        if (descriptor.source == source) {
            return descriptor;
        }
    }

    return kCatalog.front();
}

QList<ApiSource> sourcesByCapability(const bool latest, const bool history) {
    QList<ApiSource> result;
    for (const auto& descriptor : kCatalog) {
        if (!descriptor.availableViaServer) {
            continue;
        }
        if (latest && !descriptor.supportsLatest) {
            continue;
        }
        if (history && !descriptor.supportsHistory) {
            continue;
        }
        result.push_back(descriptor.source);
    }
    return result;
}

}

const ApiSourceDescriptor& describeApiSource(const ApiSource source) {
    return findDescriptor(source);
}

QList<ApiSourceDescriptor> apiSourceCatalog() {
    QList<ApiSourceDescriptor> result;
    result.reserve(static_cast<qsizetype>(kCatalog.size()));
    for (const auto& descriptor : kCatalog) {
        result.push_back(descriptor);
    }
    return result;
}

QString toDisplayName(const ApiSource source) {
    return describeApiSource(source).displayName;
}

QString toStableKey(const ApiSource source) {
    return describeApiSource(source).stableKey;
}

std::optional<ApiSource> fromStableKey(const QString& stableKey) {
    const auto normalizedKey = stableKey.trimmed().toLower();
    for (const auto& descriptor : kCatalog) {
        if (descriptor.stableKey == normalizedKey) {
            return descriptor.source;
        }
    }

    if (normalizedKey == "cbrrf") {
        return ApiSource::Cbr;
    }

    return std::nullopt;
}

bool requiresApiKey(const ApiSource source) {
    return describeApiSource(source).requiresApiKey;
}

bool isServerAvailable(const ApiSource source) {
    return describeApiSource(source).availableViaServer;
}

bool supportsLatest(const ApiSource source) {
    return describeApiSource(source).supportsLatest;
}

bool supportsHistory(const ApiSource source) {
    return describeApiSource(source).supportsHistory;
}

bool supportsCurrencyCatalog(const ApiSource source) {
    return describeApiSource(source).supportsCurrencyCatalog;
}

QList<ApiSource> allApiSources() {
    return sourcesByCapability(false, false);
}

QList<ApiSource> sourcesWithLatest() {
    return sourcesByCapability(true, false);
}

QList<ApiSource> sourcesWithHistory() {
    return sourcesByCapability(false, true);
}

}

uint qHash(const currency::client::dto::ApiSource source, const uint seed) noexcept {
    return ::qHash(static_cast<std::underlying_type_t<currency::client::dto::ApiSource>>(source), seed);
}
