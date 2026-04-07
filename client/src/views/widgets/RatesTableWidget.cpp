#include "views/widgets/RatesTableWidget.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>

namespace currency::client::views {

RatesTableWidget::RatesTableWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto* title = new QLabel("Latest rates", this);
    title->setObjectName("sectionTitleLabel");
    layout->addWidget(title);

    tableView_ = new QTableView(this);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView_->setAlternatingRowColors(true);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(tableView_);
}

void RatesTableWidget::setModel(QAbstractItemModel* model) {
    tableView_->setModel(model);
}

}