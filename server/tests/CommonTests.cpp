#include "common/Exceptions.hpp"
#include "common/TimeUtils.hpp"
#include "common/Validation.hpp"

#include <gtest/gtest.h>

namespace currency::tests {

TEST(TimeUtilsTests, RoundTripsIsoString) {
    const auto timestamp = common::fromIsoString("2026-03-19T12:34:56Z");
    EXPECT_EQ(common::toIsoString(timestamp), "2026-03-19T12:34:56Z");
}

TEST(TimeUtilsTests, InvalidIsoStringThrowsValidationError) {
    EXPECT_THROW(common::fromIsoString("2026/03/19 12:34:56"), common::ValidationError);
}

TEST(TimeUtilsTests, ParseStepSeconds) {
    EXPECT_EQ(common::parseStep("45s"), std::chrono::seconds(45));
}

TEST(TimeUtilsTests, ParseStepMinutes) {
    EXPECT_EQ(common::parseStep("15m"), std::chrono::minutes(15));
}

TEST(TimeUtilsTests, ParseStepHours) {
    EXPECT_EQ(common::parseStep("2h"), std::chrono::hours(2));
}

TEST(TimeUtilsTests, ParseStepDays) {
    EXPECT_EQ(common::parseStep("3d"), std::chrono::hours(72));
}

TEST(TimeUtilsTests, ParseStepRejectsUnsupportedUnit) {
    EXPECT_THROW(common::parseStep("5w"), common::ValidationError);
}

TEST(ValidationTests, NormalizeCurrencyCodeUppercasesValue) {
    EXPECT_EQ(common::normalizeCurrencyCode("eur"), "EUR");
}

TEST(ValidationTests, NormalizeCurrencyCodeRejectsNonLetters) {
    EXPECT_THROW(common::normalizeCurrencyCode("E2R"), common::ValidationError);
}

TEST(ValidationTests, NormalizeCurrencyCodeRejectsWrongLength) {
    EXPECT_THROW(common::normalizeCurrencyCode("EURO"), common::ValidationError);
}

TEST(ValidationTests, RequireNotBlankRejectsEmptyString) {
    EXPECT_THROW(common::requireNotBlank("", "provider"), common::ValidationError);
}

TEST(ValidationTests, RequirePositiveRejectsZero) {
    EXPECT_THROW(common::requirePositive(0.0, "amount"), common::ValidationError);
}

}
