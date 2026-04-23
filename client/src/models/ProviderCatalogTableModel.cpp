#include "models/ProviderCatalogTableModel.hpp"

#include "common/UiText.hpp"

#include <QBrush>
#include <QColor>

namespace currency::client::models {

ProviderCatalogTableModel::ProviderCatalogTableModel(QObject* parent)
    : QAbstractTableModel(parent),
      descriptors_(dto::apiSourceCatalog()) {}

int ProviderCatalogTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : descriptors_.size();
}

int ProviderCatalogTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 5;
}

QVariant ProviderCatalogTableModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid() || index.row() >= descriptors_.size()) {
        return {};
    }

    const auto& item = descriptors_.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return item.displayName;
        case 1:
            return item.shortDescription;
        case 2:
            return item.baseUrl;
        case 3:
            return common::UiText::providerApiKeyRequiredShort(item.requiresApiKey);
        case 4:
            return common::UiText::providerAvailable(item.availableViaServer);
        default:
            return {};
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() >= 3) {
            return static_cast<int>(Qt::AlignCenter);
        }
        return static_cast<int>(Qt::AlignVCenter | Qt::AlignLeft);
    }

    if (role == Qt::BackgroundRole && index.column() == 4) {
        return item.availableViaServer ? QBrush(QColor("#e7f6ef")) : QBrush(QColor("#fff5e8"));
    }

    if (role == Qt::ToolTipRole) {
        return QString("%1\n%2").arg(item.displayName, item.baseUrl);
    }

    return {};
}

QVariant ProviderCatalogTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case 0:
        return QString::fromUtf8(u8"Вид API");
    case 1:
        return QString::fromUtf8(u8"Краткое описание");
    case 2:
        return QString::fromUtf8(u8"Базовый URL");
    case 3:
        return QString::fromUtf8(u8"API-ключ");
    case 4:
        return QString::fromUtf8(u8"Статус");
    default:
        return {};
    }
}

dto::ApiSourceDescriptor ProviderCatalogTableModel::descriptorAt(const int row) const {
    if (row < 0 || row >= descriptors_.size()) {
        return descriptors_.front();
    }
    return descriptors_.at(row);
}

}
