#pragma once

#include "models/viewmodels/RateViewModel.hpp"

#include <QAbstractTableModel>
#include <QList>

namespace currency::client::models {

class CurrencyTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    CurrencyTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setRates(const QList<RateViewModel>& rates);
    QList<RateViewModel> rates() const;

private:
    QList<RateViewModel> rates_;
};

}