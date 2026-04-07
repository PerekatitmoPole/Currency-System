#pragma once

#include "models/viewmodels/AggregationResultViewModel.hpp"

#include <QAbstractTableModel>
#include <QList>

namespace currency::client::models {

class AggregationTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    AggregationTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setResults(const QList<AggregationResultViewModel>& results);

private:
    QList<AggregationResultViewModel> results_;
};

}