#include "common/ThemeManager.hpp"

#include <QColor>
#include <QFile>
#include <QPalette>

namespace currency::client::common {

void ThemeManager::apply(QApplication& application) {
    application.setStyle("Fusion");

    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#f4f7fb"));
    palette.setColor(QPalette::WindowText, QColor("#17212b"));
    palette.setColor(QPalette::Base, QColor("#ffffff"));
    palette.setColor(QPalette::AlternateBase, QColor("#edf2f7"));
    palette.setColor(QPalette::Text, QColor("#17212b"));
    palette.setColor(QPalette::Button, QColor("#ffffff"));
    palette.setColor(QPalette::ButtonText, QColor("#17212b"));
    palette.setColor(QPalette::Highlight, QColor("#0f766e"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    application.setPalette(palette);

    QFile file(":/client/styles/app.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        application.setStyleSheet(QString::fromUtf8(file.readAll()));
    }
}

}