#pragma once

#include "dto/ApiDtos.hpp"

#include <QList>
#include <QSet>

namespace currency::client::models {

class ApiSelectionState {
public:
    void setSelectedSources(const QList<dto::ApiSource>& sources);
    void select(dto::ApiSource source);
    void unselect(dto::ApiSource source);
    bool isSelected(dto::ApiSource source) const;
    QList<dto::ApiSource> selectedSources() const;

private:
    QSet<dto::ApiSource> selectedSources_;
};

}