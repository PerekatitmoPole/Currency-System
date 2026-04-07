#pragma once

#include "dto/NormalizedDtos.hpp"
#include "dto/UiDtos.hpp"
#include "models/viewmodels/HistoryPointViewModel.hpp"

namespace currency::client::services {

class ChartPreparationService {
public:
    QList<dto::ChartPointDto> prepareChartPoints(const QList<dto::NormalizedHistoryPointDto>& history) const;
    QList<models::HistoryPointViewModel> toHistoryViewModels(const QList<dto::NormalizedHistoryPointDto>& history) const;
};

}