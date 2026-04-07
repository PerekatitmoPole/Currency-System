#include "services/AggregationService.hpp"

#include <algorithm>
#include <QHash>
#include <QStringList>

namespace currency::client::services {

QList<models::RateViewModel> AggregationService::toRateViewModels(const QList<dto::NormalizedQuoteDto>& quotes) const {
    QList<models::RateViewModel> rows;
    rows.reserve(quotes.size());
    for (const auto& quote : quotes) {
        rows.push_back(models::RateViewModel{
            .sourceName = quote.providerName,
            .baseCurrency = quote.baseCurrency,
            .quoteCurrency = quote.quoteCurrency,
            .rate = quote.rate,
            .timestamp = quote.timestamp,
            .note = {},
        });
    }

    std::sort(rows.begin(), rows.end(), [](const auto& left, const auto& right) {
        if (left.quoteCurrency == right.quoteCurrency) {
            return left.sourceName < right.sourceName;
        }
        return left.quoteCurrency < right.quoteCurrency;
    });

    return rows;
}

QList<models::AggregationResultViewModel> AggregationService::aggregate(const QList<dto::NormalizedQuoteDto>& quotes) const {
    struct Bucket {
        double sum{0.0};
        double min{0.0};
        double max{0.0};
        int count{0};
        QStringList providers;
        QString base;
        QString quote;
    };

    QHash<QString, Bucket> buckets;
    for (const auto& quote : quotes) {
        const auto key = quote.baseCurrency + '|' + quote.quoteCurrency;
        auto& bucket = buckets[key];
        if (bucket.count == 0) {
            bucket.min = quote.rate;
            bucket.max = quote.rate;
            bucket.base = quote.baseCurrency;
            bucket.quote = quote.quoteCurrency;
        }
        bucket.sum += quote.rate;
        bucket.min = std::min(bucket.min, quote.rate);
        bucket.max = std::max(bucket.max, quote.rate);
        bucket.count += 1;
        bucket.providers.push_back(quote.providerName);
    }

    QList<models::AggregationResultViewModel> results;
    for (auto iterator = buckets.cbegin(); iterator != buckets.cend(); ++iterator) {
        const auto& bucket = iterator.value();
        results.push_back(models::AggregationResultViewModel{
            .baseCurrency = bucket.base,
            .quoteCurrency = bucket.quote,
            .averageRate = bucket.count > 0 ? bucket.sum / bucket.count : 0.0,
            .minimumRate = bucket.min,
            .maximumRate = bucket.max,
            .providerCount = bucket.count,
            .providers = bucket.providers.join(", "),
        });
    }

    std::sort(results.begin(), results.end(), [](const auto& left, const auto& right) {
        return left.quoteCurrency < right.quoteCurrency;
    });

    return results;
}

dto::AggregationSummaryDto AggregationService::summarize(const QList<dto::NormalizedQuoteDto>& quotes) const {
    dto::AggregationSummaryDto summary;
    if (quotes.isEmpty()) {
        return summary;
    }

    summary.baseCurrency = quotes.first().baseCurrency;
    summary.quoteCurrency = quotes.first().quoteCurrency;
    summary.sampleCount = quotes.size();
    summary.averageRate = 0.0;
    summary.minimumRate = quotes.first().rate;
    summary.maximumRate = quotes.first().rate;

    for (const auto& quote : quotes) {
        summary.averageRate += quote.rate;
        summary.minimumRate = std::min(summary.minimumRate, quote.rate);
        summary.maximumRate = std::max(summary.maximumRate, quote.rate);
    }

    summary.averageRate /= quotes.size();
    return summary;
}

}