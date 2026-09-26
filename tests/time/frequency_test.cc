#include "time/frequency.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace alm::time;

TEST(FrequencyTest, EventsPerYearAndYearFraction) {
  EXPECT_DOUBLE_EQ(events_per_year(Frequency::Annual), 1.0);
  EXPECT_DOUBLE_EQ(events_per_year(Frequency::SemiAnnual), 2.0);
  EXPECT_DOUBLE_EQ(events_per_year(Frequency::Quarterly), 4.0);
  EXPECT_DOUBLE_EQ(events_per_year(Frequency::Bimonthly), 6.0);
  EXPECT_DOUBLE_EQ(events_per_year(Frequency::Monthly), 12.0);
  EXPECT_DOUBLE_EQ(events_per_year(Frequency::Weekly), 52.0);
  EXPECT_DOUBLE_EQ(events_per_year(Frequency::Daily), 365.0);

  EXPECT_DOUBLE_EQ(year_fraction(Frequency::Annual), 1.0);
  EXPECT_DOUBLE_EQ(year_fraction(Frequency::SemiAnnual), 0.5);
  EXPECT_DOUBLE_EQ(year_fraction(Frequency::Quarterly), 0.25);
  EXPECT_NEAR(year_fraction(Frequency::Monthly), 1.0 / 12.0, 1e-9);
}

TEST(FrequencyTest, ParsingAndFormatting) {
  EXPECT_EQ(parse_frequency("Annual"), Frequency::Annual);
  EXPECT_EQ(parse_frequency("1Y"), Frequency::Annual);
  EXPECT_EQ(parse_frequency("SemiAnnual"), Frequency::SemiAnnual);
  EXPECT_EQ(parse_frequency("6M"), Frequency::SemiAnnual);
  EXPECT_EQ(parse_frequency("SA"), Frequency::SemiAnnual);
  EXPECT_EQ(parse_frequency("Quarterly"), Frequency::Quarterly);
  EXPECT_EQ(parse_frequency("3M"), Frequency::Quarterly);
  EXPECT_EQ(parse_frequency("Monthly"), Frequency::Monthly);
  EXPECT_EQ(parse_frequency("1M"), Frequency::Monthly);
  EXPECT_EQ(parse_frequency("Weekly"), Frequency::Weekly);
  EXPECT_EQ(parse_frequency("Daily"), Frequency::Daily);
  EXPECT_FALSE(parse_frequency("InvalidFrequency").has_value());

  EXPECT_EQ(to_string(Frequency::Quarterly), "Quarterly");
  std::ostringstream oss;
  oss << Frequency::SemiAnnual;
  EXPECT_EQ(oss.str(), "SemiAnnual");

  EXPECT_EQ(std::format("Freq: {}", Frequency::Annual), "Freq: Annual");
}
