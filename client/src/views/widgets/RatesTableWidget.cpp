#include "views/widgets/RatesTableWidget.hpp"

#include <QAbstractItemModel>
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

    titleLabel_ = new QLabel("Последние курсы", this);
    titleLabel_->setObjectName("sectionTitleLabel");
    layout->addWidget(titleLabel_);

    emptyLabel_ = new QLabel("Данные пока не загружены. Нажмите кнопку загрузки, чтобы получить курсы от выбранных источников.", this);
    emptyLabel_->setObjectName("mutedLabel");
    emptyLabel_->setWordWrap(true);
    layout->addWidget(emptyLabel_);

    tableView_ = new QTableView(this);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView_->horizontalHeader()->setMinimumSectionSize(120);
    tableView_->verticalHeader()->setVisible(false);
    tableView_->verticalHeader()->setDefaultSectionSize(40);
    tableView_->setAlternatingRowColors(true);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->setSortingEnabled(true);
    tableView_->setWordWrap(false);
    tableView_->setShowGrid(false);
    tableView_->setMinimumHeight(320);
    layout->addWidget(tableView_);
}

void RatesTableWidget::setModel(QAbstractItemModel* model) {
    model_ = model;
    tableView_->setModel(model);
    if (model_ != nullptr) {
        connect(model_, &QAbstractItemModel::modelReset, this, &RatesTableWidget::refreshEmptyState);
        connect(model_, &QAbstractItemModel::rowsInserted, this, &RatesTableWidget::refreshEmptyState);
        connect(model_, &QAbstractItemModel::rowsRemoved, this, &RatesTableWidget::refreshEmptyState);
    }
    refreshEmptyState();
}

void RatesTableWidget::setTitle(const QString& title) {
    titleLabel_->setText(title);
}

void RatesTableWidget::refreshEmptyState() {
    const auto hasRows = model_ != nullptr && model_->rowCount() > 0;
    emptyLabel_->setVisible(!hasRows);
    tableView_->setVisible(hasRows);
}

}
