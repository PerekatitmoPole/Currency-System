#pragma once

#include <QString>

namespace currency::client::models {

struct CurrencyViewModel {
    QString code;
    QString name;
    int minorUnits{2};
};

}