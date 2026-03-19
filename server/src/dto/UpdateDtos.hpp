#pragma once

#include <string>
#include <vector>

namespace currency::dto {

struct QuoteSnapshotDto {
    std::string baseCode;
    std::string baseName;
    std::string quoteCode;
    std::string quoteName;
    double rate{0.0};
    std::string sourceTimestamp;
};

struct UpdateQuotesRequestDto {
    std::string provider;
    std::string batchTimestamp;
    std::vector<QuoteSnapshotDto> quotes;
};

struct UpdateQuotesResponseDto {
    std::string provider;
    std::string processedAt;
    std::size_t acceptedCount{0};
};

}
