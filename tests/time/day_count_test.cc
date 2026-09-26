#include "time/day_count.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace alm::time;

TEST(DayCountTest, Actual360) {
  auto dc = DayCounter::actual_360();
  EXPECT_EQ(dc.name(), "Actual/360");

  Date d1(2024, 1, 1);
  Date d2(2024, 4, 1); // 91 days in leap year 2024

  EXPECT_EQ(dc.day_count(d1, d2), 91);
  EXPECT_DOUBLE_EQ(dc.year_fraction(d1, d2), 91.0 / 360.0);
}

TEST(DayCountTest, Actual365Fixed) {
  auto dc = DayCounter::actual_365_fixed();
  EXPECT_EQ(dc.name(), "Actual/365 Fixed");

  Date d1(2023, 1, 1);
  Date d2(2024, 1, 1); // 365 days in 2023
  EXPECT_EQ(dc.day_count(d1, d2), 365);
  EXPECT_DOUBLE_EQ(dc.year_fraction(d1, d2), 1.0);

  Date d3(2024, 1, 1);
  Date d4(2025, 1, 1); // 366 days in leap year 2024
  EXPECT_EQ(dc.day_count(d3, d4), 366);
  EXPECT_DOUBLE_EQ(dc.year_fraction(d3, d4), 366.0 / 365.0);
}

TEST(DayCountTest, Actual365NoLeap) {
  auto dc = DayCounter::actual_365_no_leap();

  Date d1(2024, 2, 28);
  Date d2(2024, 3, 1); // 2 days in leap year, but Feb 29 excluded => 1 day
  EXPECT_EQ(dc.day_count(d1, d2), 1);
  EXPECT_DOUBLE_EQ(dc.year_fraction(d1, d2), 1.0 / 365.0);
}

TEST(DayCountTest, ActualActualISDA) {
  auto dc = DayCounter::actual_actual_isda();

  // Within leap year 2024
  Date d1(2024, 1, 1);
  Date d2(2024, 7, 1); // 182 days
  EXPECT_DOUBLE_EQ(dc.year_fraction(d1, d2), 182.0 / 366.0);

  // Across year boundary: 2023-11-01 to 2024-03-01
  Date d3(2023, 11, 1);
  Date d4(2024, 3, 1);
  Date jan1_2024(2024, 1, 1);
  double expected = static_cast<double>(jan1_2024 - d3) / 365.0 +
                    static_cast<double>(d4 - jan1_2024) / 366.0;
  EXPECT_DOUBLE_EQ(dc.year_fraction(d3, d4), expected);
}

TEST(DayCountTest, ActualActualICMA) {
  auto dc = DayCounter::actual_actual_icma(Frequency::SemiAnnual);

  Date d1(2024, 1, 15);
  Date d2(2024, 7, 15); // Exactly 1 semi-annual coupon period
  EXPECT_DOUBLE_EQ(dc.year_fraction(d1, d2, d1, d2), 0.5);
}

TEST(DayCountTest, Thirty360US) {
  auto dc = DayCounter::thirty_360_us();

  Date d1(2024, 1, 31);
  Date d2(2024, 7, 31);
  // D1=31 -> 30, D2=31 & D1>=30 -> 30 => 6 months of 30 days = 180 days
  EXPECT_EQ(dc.day_count(d1, d2), 180);
  EXPECT_DOUBLE_EQ(dc.year_fraction(d1, d2), 0.5);

  Date feb_start(2024, 2, 1);
  Date feb_end(2024, 3, 1);
  EXPECT_EQ(dc.day_count(feb_start, feb_end), 30);
}

TEST(DayCountTest, ThirtyE360) {
  auto dc = DayCounter::thirty_e_360();

  Date d1(2024, 1, 31);
  Date d2(2024, 2, 28);
  // D1=31 -> 30, D2=28. Month diff=1 -> 30*1 + (28-30) = 28 days
  EXPECT_EQ(dc.day_count(d1, d2), 28);
}

TEST(DayCountTest, ParsingAndFormatting) {
  auto dc_a360 = DayCounter::parse("Act/360");
  ASSERT_TRUE(dc_a360.has_value());
  EXPECT_EQ(dc_a360->convention(), DayCountConvention::Actual360);

  auto dc_a365 = DayCounter::parse("Act/365");
  ASSERT_TRUE(dc_a365.has_value());
  EXPECT_EQ(dc_a365->convention(), DayCountConvention::Actual365Fixed);

  auto dc_30360 = DayCounter::parse("30/360");
  ASSERT_TRUE(dc_30360.has_value());
  EXPECT_EQ(dc_30360->convention(), DayCountConvention::Thirty360US);

  auto dc_actact = DayCounter::parse("Act/Act");
  ASSERT_TRUE(dc_actact.has_value());
  EXPECT_EQ(dc_actact->convention(), DayCountConvention::ActualActualISDA);

  EXPECT_FALSE(DayCounter::parse("UnknownConvention").has_value());

  std::ostringstream oss;
  oss << *dc_a360;
  EXPECT_EQ(oss.str(), "Actual/360");
}
