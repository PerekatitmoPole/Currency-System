#include "services/ChartPreparationService.hpp"

#include <algorithm>

namespace currency::client::services {

QList<dto::ChartPointDto> ChartPreparationService::prepareChartPoints(const QList<dto::NormalizedHistoryPointDto>& history) const {
    QList<dto::ChartPointDto> points;
    for (const auto& item : history) {
        points.push_back(dto::ChartPointDto{item.timestamp, item.rate});
    }

    std::sort(points.begin(), points.end(), [](const auto& left, const auto& right) {
        return left.timestamp < right.timestamp;
    });
    return points;
}

QList<models::HistoryPointViewModel> ChartPreparationService::toHistoryViewModels(const QList<dto::NormalizedHistoryPointDto>& history) const {
    QList<models::HistoryPointViewModel> points;
    for (const auto& item : history) {
        points.push_back(models::HistoryPointViewModel{
            .sourceName = item.providerName,
            .baseCurrency = item.baseCurrency,
            .quoteCurrency = item.quoteCurrency,
            .rate = item.rate,
            .timestamp = item.timestamp,
        });
    }

    std::sort(points.begin(), points.end(), [](const auto& left, const auto& right) {
        return left.timestamp < right.timestamp;
    });
    return points;
}

}