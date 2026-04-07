#pragma once

#include <QString>

namespace currency::client::models {

struct AggregationResultViewModel {
    QString baseCurrency;
    QString quoteCurrency;
    double averageRate{0.0};
    double minimumRate{0.0};
    double maximumRate{0.0};
    int providerCount{0};
    QString providers;
};

}