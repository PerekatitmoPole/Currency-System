#pragma once

#include <QApplication>

namespace currency::client::common {

class ThemeManager {
public:
    static void apply(QApplication& application);
};

}