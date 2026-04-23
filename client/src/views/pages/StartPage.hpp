#pragma once

#include "models/ProviderCatalogTableModel.hpp"

#include <QString>
#include <QWidget>

class QPushButton;
class QLabel;
class QPlainTextEdit;
class QTableView;

namespace currency::client::views {

class ConnectionStatusWidget;

class StartPage : public QWidget {
    Q_OBJECT

public:
    StartPage(QWidget* parent = nullptr);

    ConnectionStatusWidget* connectionStatusWidget() const;
    QTableView* providerTableView() const;
    QPushButton* dashboardButton() const;
    QPushButton* settingsButton() const;
    void setSystemMetrics(const QString& lastSyncText, int activeSourceCount, int currencyCount);
    void setProviderDetails(const QString& title, const QString& baseCurrency, const QString& conversions, const QString& statusText);

private:
    ConnectionStatusWidget* connectionStatusWidget_{nullptr};
    QTableView* providerTableView_{nullptr};
    QPushButton* dashboardButton_{nullptr};
    QPushButton* settingsButton_{nullptr};
    QLabel* lastSyncValueLabel_{nullptr};
    QLabel* activeSourcesValueLabel_{nullptr};
    QLabel* currenciesValueLabel_{nullptr};
    QLabel* providerTitleLabel_{nullptr};
    QLabel* providerBaseValueLabel_{nullptr};
    QPlainTextEdit* providerConversionsTextEdit_{nullptr};
    QLabel* providerStatusValueLabel_{nullptr};
    models::ProviderCatalogTableModel providerCatalogModel_;
};

}
