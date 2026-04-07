#pragma once

#include "dto/NormalizedDtos.hpp"
#include "dto/UiDtos.hpp"
#include "models/viewmodels/AggregationResultViewModel.hpp"
#include "models/viewmodels/RateViewModel.hpp"

namespace currency::client::services {

class AggregationService {
public:
    QList<models::RateViewModel> toRateViewModels(const QList<dto::NormalizedQuoteDto>& quotes) const;
    QList<models::AggregationResultViewModel> aggregate(const QList<dto::NormalizedQuoteDto>& quotes) const;
    dto::AggregationSummaryDto summarize(const QList<dto::NormalizedQuoteDto>& quotes) const;
};

}