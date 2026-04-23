#pragma once

#include "common/Result.hpp"
#include "models/viewmodels/HistoryPointViewModel.hpp"
#include "models/viewmodels/RateViewModel.hpp"

namespace currency::client::services {

class ExportService {
public:
    common::Result<void> exportRates(const QList<models::RateViewModel>& rates, const QString& path) const;
    common::Result<void> exportHistory(const QList<models::HistoryPointViewModel>& points, const QString& path) const;
};

}
