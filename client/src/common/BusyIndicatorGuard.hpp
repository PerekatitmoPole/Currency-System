#pragma once

#include <QApplication>
#include <QPushButton>

namespace currency::client::common {

class BusyIndicatorGuard {
public:
    BusyIndicatorGuard(QPushButton* button, QString busyText)
        : button_(button) {
        if (button_ != nullptr) {
            originalText_ = button_->text();
            button_->setEnabled(false);
            if (!busyText.isEmpty()) {
                button_->setText(std::move(busyText));
            }
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    ~BusyIndicatorGuard() {
        if (button_ != nullptr) {
            button_->setEnabled(true);
            button_->setText(originalText_);
        }

        QApplication::restoreOverrideCursor();
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

private:
    QPushButton* button_{nullptr};
    QString originalText_;
};

}
