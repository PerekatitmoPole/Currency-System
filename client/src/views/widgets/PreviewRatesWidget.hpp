#pragma once

#include "models/viewmodels/RateViewModel.hpp"

#include <QWidget>

class QGridLayout;

namespace currency::client::views {

class PreviewRatesWidget : public QWidget {
    Q_OBJECT

public:
    PreviewRatesWidget(QWidget* parent = nullptr);

    void setRates(const QList<models::RateViewModel>& rates);

private:
    QGridLayout* gridLayout_{nullptr};
};

}