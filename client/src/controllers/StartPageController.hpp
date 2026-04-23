#pragma once

#include <QObject>

namespace currency::client::app {
class MainWindow;
}

namespace currency::client::models {
class AppState;
}

namespace currency::client::views {
class StartPage;
}

namespace currency::client::controllers {

class StartPageController : public QObject {
    Q_OBJECT

public:
    StartPageController(
        views::StartPage& page,
        app::MainWindow& mainWindow,
        models::AppState& appState,
        QObject* parent = nullptr);

private:
    void updateSelectedProviderDetails();
    void updateOverviewMetrics();

    views::StartPage& page_;
    app::MainWindow& mainWindow_;
    models::AppState& appState_;
};

}
