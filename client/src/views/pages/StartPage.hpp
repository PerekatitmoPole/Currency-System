#pragma once

#include <QWidget>

class QPushButton;

namespace currency::client::views {

class ApiSelectorWidget;
class ConnectionStatusWidget;
class PreviewRatesWidget;

class StartPage : public QWidget {
    Q_OBJECT

public:
    StartPage(QWidget* parent = nullptr);

    ApiSelectorWidget* apiSelector() const;
    ConnectionStatusWidget* connectionStatusWidget() const;
    PreviewRatesWidget* previewRatesWidget() const;
    QPushButton* aggregateButton() const;

private:
    ApiSelectorWidget* apiSelector_{nullptr};
    ConnectionStatusWidget* connectionStatusWidget_{nullptr};
    PreviewRatesWidget* previewRatesWidget_{nullptr};
    QPushButton* aggregateButton_{nullptr};
};

}