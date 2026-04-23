#include "models/AggregationTableModel.hpp"

#include <QColor>

namespace currency::client::models {

AggregationTableModel::AggregationTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int AggregationTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : results_.size();
}

int AggregationTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 7;
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
            return item.providerCount;
        case 6:
            return item.providers;
        default:
            break;
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() >= 2 && index.column() <= 5) {
            return static_cast<int>(Qt::AlignVCenter | Qt::AlignRight);
        }
        return static_cast<int>(Qt::AlignVCenter | Qt::AlignLeft);
    }

    if (role == Qt::BackgroundRole) {
        if (index.column() == 3) {
            return QColor("#e7f6ef");
        }
        if (index.column() == 4) {
            return QColor("#fff1f1");
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
        return "Базовая";
    case 1:
        return "Котируемая";
    case 2:
        return "Среднее";
    case 3:
        return "Минимум";
    case 4:
        return "Максимум";
    case 5:
        return "Источников";
    case 6:
        return "Провайдеры";
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
