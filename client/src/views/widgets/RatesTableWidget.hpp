#pragma once

#include <QString>
#include <QWidget>

class QAbstractItemModel;
class QLabel;
class QTableView;

namespace currency::client::views {

class RatesTableWidget : public QWidget {
    Q_OBJECT

public:
    RatesTableWidget(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);
    void setTitle(const QString& title);

private:
    void refreshEmptyState();

    QAbstractItemModel* model_{nullptr};
    QLabel* titleLabel_{nullptr};
    QLabel* emptyLabel_{nullptr};
    QTableView* tableView_{nullptr};
};

}
