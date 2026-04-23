#include "models/ApiSelectionState.hpp"

namespace currency::client::models {

void ApiSelectionState::setSelectedSources(const QList<dto::ApiSource>& sources) {
    selectedSources_ = QSet<dto::ApiSource>(sources.cbegin(), sources.cend());
}

void ApiSelectionState::select(const dto::ApiSource source) {
    selectedSources_.insert(source);
}

void ApiSelectionState::unselect(const dto::ApiSource source) {
    selectedSources_.remove(source);
}

bool ApiSelectionState::isSelected(const dto::ApiSource source) const {
    return selectedSources_.contains(source);
}

QList<dto::ApiSource> ApiSelectionState::selectedSources() const {
    QList<dto::ApiSource> orderedSources;
    for (const auto source : dto::apiSourceCatalog()) {
        if (selectedSources_.contains(source.source)) {
            orderedSources.push_back(source.source);
        }
    }
    return orderedSources;
}

}
