#include "models/CurrencyTableModel.hpp"

#include "common/DateTimeUtils.hpp"

namespace currency::client::models {

CurrencyTableModel::CurrencyTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int CurrencyTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : rates_.size();
}

int CurrencyTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 5;
}

QVariant CurrencyTableModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid() || index.row() >= rates_.size()) {
        return {};
    }

    const auto& item = rates_.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return item.sourceName;
        case 1:
            return item.baseCurrency;
        case 2:
            return item.quoteCurrency;
        case 3:
            return QString::number(item.rate, 'f', 6);
        case 4:
            return common::DateTimeUtils::toDisplayString(item.timestamp);
        default:
            break;
        }
    }

    return {};
}

QVariant CurrencyTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case 0:
        return "Source";
    case 1:
        return "Base";
    case 2:
        return "Quote";
    case 3:
        return "Rate";
    case 4:
        return "Updated";
    default:
        return {};
    }
}

void CurrencyTableModel::setRates(const QList<RateViewModel>& rates) {
    beginResetModel();
    rates_ = rates;
    endResetModel();
}

QList<RateViewModel> CurrencyTableModel::rates() const {
    return rates_;
}

}