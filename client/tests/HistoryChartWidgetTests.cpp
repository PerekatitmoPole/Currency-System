#include "ClientTestSupport.hpp"
#include "views/widgets/HistoryChartWidget.hpp"

#include <QFileInfo>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

namespace currency::client::tests {

namespace {

void spinUi(const int ms) {
    QCoreApplication::processEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    QCoreApplication::processEvents();
}

}

TEST(HistoryChartWidgetTests, SavesChartAsPng) {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    views::HistoryChartWidget widget;
    widget.resize(800, 480);
    widget.setPoints(sampleChartPoints(), "EUR/USD");
    widget.show();
    spinUi(50);

    const auto path = temporaryDir.filePath("history.png");

    ASSERT_TRUE(widget.saveAsPng(path));

    QFileInfo fileInfo(path);
    EXPECT_TRUE(fileInfo.exists());
    EXPECT_GT(fileInfo.size(), 0);
}

TEST(HistoryChartWidgetTests, SavesPlaceholderWhenNoPoints) {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    views::HistoryChartWidget widget;
    widget.resize(800, 480);
    widget.setPoints({}, "Empty");
    widget.show();
    spinUi(50);

    const auto path = temporaryDir.filePath("empty-history.png");
    ASSERT_TRUE(widget.saveAsPng(path));

    QFileInfo fileInfo(path);
    EXPECT_TRUE(fileInfo.exists());
    EXPECT_GT(fileInfo.size(), 0);
}

}
