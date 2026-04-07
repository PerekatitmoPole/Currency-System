#pragma once

#include "common/Errors.hpp"
#include "common/Result.hpp"
#include "dto/ApiDtos.hpp"
#include "dto/NormalizedDtos.hpp"
#include "gateways/IApiSourceClient.hpp"
#include "services/NormalizationService.hpp"

#include <QDate>
#include <map>

namespace currency::client::services {

struct LatestFetchResult {
    QList<dto::NormalizedQuoteDto> quotes;
    QList<common::Error> warnings;
};

class ApiOrchestratorService {
public:
    explicit ApiOrchestratorService(NormalizationService& normalizationService);

    void registerClient(gateways::IApiSourceClient& client);
    void setApiKey(dto::ApiSource source, const QString& apiKey);

    common::Result<LatestFetchResult> fetchLatest(
        const QList<dto::ApiSource>& selectedSources,
        const QString& requestedBaseCurrency,
        const QStringList& requestedQuoteCurrencies);

    common::Result<QList<dto::NormalizedHistoryPointDto>> fetchHistory(
        dto::ApiSource source,
        const QString& requestedBaseCurrency,
        const QString& requestedQuoteCurrency,
        const QDate& from,
        const QDate& to);

private:
    gateways::IApiSourceClient* findClient(dto::ApiSource source) const;

    NormalizationService& normalizationService_;
    std::map<dto::ApiSource, gateways::IApiSourceClient*> clients_;
};

}