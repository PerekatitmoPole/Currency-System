#include "models/AggregationTableModel.hpp"

namespace currency::client::models {

AggregationTableModel::AggregationTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int AggregationTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : results_.size();
}

int AggregationTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 6;
}

QVariant AggregationTableModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid() || index.row() >= results_.size()) {
        return {};
    }

    const auto& item = results_.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return item.baseCurrency;
        case 1:
            return item.quoteCurrency;
        case 2:
            return QString::number(item.averageRate, 'f', 6);
        case 3:
            return QString::number(item.minimumRate, 'f', 6);
        case 4:
            return QString::number(item.maximumRate, 'f', 6);
        case 5:
            return item.providers;
        default:
            break;
        }
    }

    return {};
}

QVariant AggregationTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case 0:
        return "Base";
    case 1:
        return "Quote";
    case 2:
        return "Average";
    case 3:
        return "Min";
    case 4:
        return "Max";
    case 5:
        return "Providers";
    default:
        return {};
    }
}

void AggregationTableModel::setResults(const QList<AggregationResultViewModel>& results) {
    beginResetModel();
    results_ = results;
    endResetModel();
}

}