#pragma once

#include "models/viewmodels/HistoryPointViewModel.hpp"

#include <QAbstractTableModel>
#include <QList>

namespace currency::client::models {

class HistoryTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    HistoryTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setHistoryPoints(const QList<HistoryPointViewModel>& points);

private:
    QList<HistoryPointViewModel> points_;
};

}