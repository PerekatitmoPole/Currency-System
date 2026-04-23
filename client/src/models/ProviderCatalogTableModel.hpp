#pragma once

#include "dto/ApiDtos.hpp"

#include <QAbstractTableModel>

namespace currency::client::models {

class ProviderCatalogTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit ProviderCatalogTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    dto::ApiSourceDescriptor descriptorAt(int row) const;

private:
    QList<dto::ApiSourceDescriptor> descriptors_;
};

}
