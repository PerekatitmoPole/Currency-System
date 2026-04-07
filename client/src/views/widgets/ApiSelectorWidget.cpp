#include "views/widgets/ApiSelectorWidget.hpp"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QSet>
#include <QVBoxLayout>

namespace currency::client::views {

ApiSelectorWidget::ApiSelectorWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("cardWidget");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(12);

    auto* title = new QLabel("Select API sources", this);
    title->setObjectName("sectionTitleLabel");
    rootLayout->addWidget(title);

    auto* subtitle = new QLabel("Choose one or several providers for aggregation and comparison.", this);
    subtitle->setObjectName("mutedLabel");
    subtitle->setWordWrap(true);
    rootLayout->addWidget(subtitle);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(10);

    const auto sources = dto::allApiSources();
    for (int index = 0; index < sources.size(); ++index) {
        const auto source = sources.at(index);
        auto* checkBox = new QCheckBox(dto::toDisplayName(source), this);
        grid->addWidget(checkBox, index / 2, index % 2);
        checkboxes_.insert(source, checkBox);
        connect(checkBox, &QCheckBox::toggled, this, [this] {
            emit selectionChanged(selectedSources());
        });
    }

    rootLayout->addLayout(grid);
}

QList<dto::ApiSource> ApiSelectorWidget::selectedSources() const {
    QList<dto::ApiSource> result;
    for (auto iterator = checkboxes_.cbegin(); iterator != checkboxes_.cend(); ++iterator) {
        if (iterator.value()->isChecked()) {
            result.push_back(iterator.key());
        }
    }
    return result;
}

void ApiSelectorWidget::setSelectedSources(const QList<dto::ApiSource>& sources) {
    const auto selected = QSet<dto::ApiSource>(sources.cbegin(), sources.cend());
    for (auto iterator = checkboxes_.begin(); iterator != checkboxes_.end(); ++iterator) {
        iterator.value()->setChecked(selected.contains(iterator.key()));
    }
}

}