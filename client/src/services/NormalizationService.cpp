#include "services/NormalizationService.hpp"

#include "common/Errors.hpp"
#include "dto/ApiDtos.hpp"

#include <algorithm>
#include <QHash>
#include <QQueue>
#include <QSet>

namespace currency::client::services {

namespace {

struct Edge {
    QString targetCurrency;
    double rate{0.0};
    QDateTime timestamp;
};

using Graph = QHash<QString, QList<Edge>>;

Graph buildGraph(const QList<dto::ExternalQuoteEntryRawDto>& entries) {
    Graph graph;
    for (const auto& entry : entries) {
        if (entry.rate <= 0.0) {
            continue;
        }

        graph[entry.baseCurrency].push_back(Edge{entry.quoteCurrency, entry.rate, entry.timestamp});
        graph[entry.quoteCurrency].push_back(Edge{entry.baseCurrency, 1.0 / entry.rate, entry.timestamp});
    }
    return graph;
}

Graph buildHistoryGraph(const QList<dto::ExternalHistoryPointRawDto>& points) {
    Graph graph;
    for (const auto& entry : points) {
        if (entry.rate <= 0.0) {
            continue;
        }

        graph[entry.baseCurrency].push_back(Edge{entry.quoteCurrency, entry.rate, entry.timestamp});
        graph[entry.quoteCurrency].push_back(Edge{entry.baseCurrency, 1.0 / entry.rate, entry.timestamp});
    }
    return graph;
}

struct ResolutionResult {
    bool found{false};
    double rate{0.0};
    QDateTime timestamp;
};

ResolutionResult resolveRate(const Graph& graph, const QString& from, const QString& to) {
    if (from == to) {
        return ResolutionResult{true, 1.0, QDateTime::currentDateTimeUtc()};
    }

    struct State {
        QString currency;
        double rate{1.0};
        QDateTime timestamp;
    };

    QQueue<State> queue;
    QSet<QString> visited;
    queue.enqueue(State{from, 1.0, {}});
    visited.insert(from);

    while (!queue.isEmpty()) {
        const auto current = queue.dequeue();
        const auto edges = graph.value(current.currency);
        for (const auto& edge : edges) {
            const auto nextRate = current.rate * edge.rate;
            const auto nextTimestamp = edge.timestamp.isValid() ? edge.timestamp : current.timestamp;

            if (edge.targetCurrency == to) {
                return ResolutionResult{true, nextRate, nextTimestamp};
            }

            if (!visited.contains(edge.targetCurrency)) {
                visited.insert(edge.targetCurrency);
                queue.enqueue(State{edge.targetCurrency, nextRate, nextTimestamp});
            }
        }
    }

    return {};
}

}

common::Result<QList<dto::NormalizedQuoteDto>> NormalizationService::normalizeRates(const dto::ExternalRateRawDto& rawDto) const {
    QList<dto::NormalizedQuoteDto> normalized;
    const auto graph = buildGraph(rawDto.entries);
    for (const auto& quoteCurrency : rawDto.requestedQuoteCurrencies) {
        const auto resolved = resolveRate(graph, rawDto.requestedBaseCurrency, quoteCurrency);
        if (!resolved.found) {
            continue;
        }

        normalized.push_back(dto::NormalizedQuoteDto{
            .source = rawDto.source,
            .providerName = dto::toDisplayName(rawDto.source),
            .baseCurrency = rawDto.requestedBaseCurrency,
            .quoteCurrency = quoteCurrency,
            .baseName = rawDto.requestedBaseCurrency,
            .quoteName = quoteCurrency,
            .rate = resolved.rate,
            .timestamp = resolved.timestamp.isValid() ? resolved.timestamp : QDateTime::currentDateTimeUtc(),
        });
    }

    if (normalized.isEmpty()) {
        return common::Result<QList<dto::NormalizedQuoteDto>>::failure(common::Errors::validationError(
            QString("Provider %1 did not expose requested pair(s)").arg(dto::toDisplayName(rawDto.source)),
            QString("Base: %1, quotes: %2").arg(rawDto.requestedBaseCurrency, rawDto.requestedQuoteCurrencies.join(','))));
    }

    return common::Result<QList<dto::NormalizedQuoteDto>>::success(normalized);
}

common::Result<QList<dto::NormalizedHistoryPointDto>> NormalizationService::normalizeHistory(const dto::ExternalHistoryRawDto& rawDto) const {
    QHash<QDateTime, QList<dto::ExternalHistoryPointRawDto>> buckets;
    for (const auto& point : rawDto.points) {
        buckets[point.timestamp].push_back(point);
    }

    QList<dto::NormalizedHistoryPointDto> normalized;
    for (auto iterator = buckets.cbegin(); iterator != buckets.cend(); ++iterator) {
        const auto graph = buildHistoryGraph(iterator.value());
        const auto resolved = resolveRate(graph, rawDto.requestedBaseCurrency, rawDto.requestedQuoteCurrency);
        if (!resolved.found) {
            continue;
        }

        normalized.push_back(dto::NormalizedHistoryPointDto{
            .source = rawDto.source,
            .providerName = dto::toDisplayName(rawDto.source),
            .baseCurrency = rawDto.requestedBaseCurrency,
            .quoteCurrency = rawDto.requestedQuoteCurrency,
            .rate = resolved.rate,
            .timestamp = iterator.key(),
        });
    }

    std::sort(normalized.begin(), normalized.end(), [](const auto& left, const auto& right) {
        return left.timestamp < right.timestamp;
    });

    if (normalized.isEmpty()) {
        return common::Result<QList<dto::NormalizedHistoryPointDto>>::failure(common::Errors::validationError(
            QString("Provider %1 did not expose requested history pair").arg(dto::toDisplayName(rawDto.source)),
            QString("Pair: %1/%2").arg(rawDto.requestedBaseCurrency, rawDto.requestedQuoteCurrency)));
    }

    return common::Result<QList<dto::NormalizedHistoryPointDto>>::success(normalized);
}

}