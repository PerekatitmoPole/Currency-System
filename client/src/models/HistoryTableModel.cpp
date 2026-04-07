#include "models/HistoryTableModel.hpp"

#include "common/DateTimeUtils.hpp"

namespace currency::client::models {

HistoryTableModel::HistoryTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int HistoryTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : points_.size();
}

int HistoryTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 4;
}

QVariant HistoryTableModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid() || index.row() >= points_.size()) {
        return {};
    }

    const auto& item = points_.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return item.sourceName;
        case 1:
            return item.baseCurrency + "/" + item.quoteCurrency;
        case 2:
            return QString::number(item.rate, 'f', 6);
        case 3:
            return common::DateTimeUtils::toDisplayString(item.timestamp);
        default:
            break;
        }
    }

    return {};
}

QVariant HistoryTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case 0:
        return "Source";
    case 1:
        return "Pair";
    case 2:
        return "Rate";
    case 3:
        return "Timestamp";
    default:
        return {};
    }
}

void HistoryTableModel::setHistoryPoints(const QList<HistoryPointViewModel>& points) {
    beginResetModel();
    points_ = points;
    endResetModel();
}

}