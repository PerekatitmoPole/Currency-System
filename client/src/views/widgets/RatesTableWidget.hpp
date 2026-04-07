#pragma once

#include <QWidget>

class QAbstractItemModel;
class QTableView;

namespace currency::client::views {

class RatesTableWidget : public QWidget {
    Q_OBJECT

public:
    RatesTableWidget(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);

private:
    QTableView* tableView_{nullptr};
};

}