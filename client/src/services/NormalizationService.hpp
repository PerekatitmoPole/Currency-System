#pragma once

#include "common/Result.hpp"
#include "dto/ExternalDtos.hpp"
#include "dto/NormalizedDtos.hpp"

namespace currency::client::services {

class NormalizationService {
public:
    common::Result<QList<dto::NormalizedQuoteDto>> normalizeRates(const dto::ExternalRateRawDto& rawDto) const;
    common::Result<QList<dto::NormalizedHistoryPointDto>> normalizeHistory(const dto::ExternalHistoryRawDto& rawDto) const;
};

}