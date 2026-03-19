#pragma once

#include "dto/QueryDtos.hpp"
#include "repositories/InMemoryHistoryRepository.hpp"

namespace currency::services {

class HistoryQueryService {
public:
    explicit HistoryQueryService(repositories::InMemoryHistoryRepository& historyRepository);

    dto::GetHistoryResponseDto getHistory(const dto::GetHistoryRequestDto& request) const;

private:
    repositories::InMemoryHistoryRepository& historyRepository_;
};

}
