#pragma once

#include "dto/ApiDtos.hpp"

#include <QHash>
#include <QWidget>

class QCheckBox;

namespace currency::client::views {

class ApiSelectorWidget : public QWidget {
    Q_OBJECT

public:
    ApiSelectorWidget(QWidget* parent = nullptr);

    QList<dto::ApiSource> selectedSources() const;
    void setSelectedSources(const QList<dto::ApiSource>& sources);

signals:
    void selectionChanged(const QList<dto::ApiSource>& sources);

private:
    QHash<dto::ApiSource, QCheckBox*> checkboxes_;
};

}