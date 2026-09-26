#include "time/period.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace alm::time;

TEST(PeriodTest, ConstructionAndAccessors) {
  Period p1(3, TimeUnit::Months);
  EXPECT_EQ(p1.length(), 3);
  EXPECT_EQ(p1.units(), TimeUnit::Months);

  auto p2 = Period::years(2);
  EXPECT_EQ(p2.length(), 2);
  EXPECT_EQ(p2.units(), TimeUnit::Years);
}

TEST(PeriodTest, FrequencyConversion) {
  EXPECT_EQ(Period::months(3).frequency(), Frequency::Quarterly);
  EXPECT_EQ(Period::months(6).frequency(), Frequency::SemiAnnual);
  EXPECT_EQ(Period::years(1).frequency(), Frequency::Annual);
  EXPECT_EQ(Period::months(12).frequency(), Frequency::Annual);
  EXPECT_EQ(Period::months(1).frequency(), Frequency::Monthly);

  EXPECT_EQ(Period::from_frequency(Frequency::Quarterly), Period::months(3));
  EXPECT_EQ(Period::from_frequency(Frequency::Annual), Period::years(1));
}

TEST(PeriodTest, NormalizationAndEquality) {
  EXPECT_EQ(Period::months(12).normalize(), Period::years(1));
  EXPECT_EQ(Period::months(24).normalize(), Period::years(2));
  EXPECT_EQ(Period::days(14).normalize(), Period::weeks(2));
  EXPECT_EQ(Period::days(7).normalize(), Period::weeks(1));

  EXPECT_TRUE(Period::months(12) == Period::years(1));
  EXPECT_TRUE(Period::days(14) == Period::weeks(2));
  EXPECT_FALSE(Period::months(3) == Period::years(1));
}

TEST(PeriodTest, Arithmetic) {
  Period p1 = Period::months(3);
  Period p2 = Period::months(6);
  EXPECT_EQ(p1 + p2, Period::months(9));

  Period p_year = Period::years(1);
  EXPECT_EQ(p_year + p1, Period::months(15));

  Period p_days = Period::days(10);
  Period p_weeks = Period::weeks(1);
  EXPECT_EQ(p_days + p_weeks, Period::days(17));

  EXPECT_THROW(p1 + p_days, std::invalid_argument);

  EXPECT_EQ(p1 * 2, Period::months(6));
  EXPECT_EQ(2 * p1, Period::months(6));
  EXPECT_EQ(-p1, Period::months(-3));
}

TEST(PeriodTest, Comparisons) {
  EXPECT_LT(Period::months(3), Period::months(6));
  EXPECT_LT(Period::months(3), Period::years(1));
  EXPECT_GT(Period::years(2), Period::months(12));
  EXPECT_LE(Period::months(12), Period::years(1));
}

TEST(PeriodTest, DateAdvance) {
  Date start(2024, 1, 15);
  EXPECT_EQ(start + Period::days(10), Date(2024, 1, 25));
  EXPECT_EQ(start + Period::weeks(2), Date(2024, 1, 29));
  EXPECT_EQ(start + Period::months(1), Date(2024, 2, 15));
  EXPECT_EQ(start + Period::years(1), Date(2025, 1, 15));

  Date leap_jan(2024, 1, 31);
  EXPECT_EQ(leap_jan + Period::months(1), Date(2024, 2, 29));

  Date back = start - Period::months(1);
  EXPECT_EQ(back, Date(2023, 12, 15));
}

TEST(PeriodTest, ParsingAndFormatting) {
  EXPECT_EQ(Period::parse("3M"), Period::months(3));
  EXPECT_EQ(Period::parse("1Y"), Period::years(1));
  EXPECT_EQ(Period::parse("2W"), Period::weeks(2));
  EXPECT_EQ(Period::parse("10D"), Period::days(10));
  EXPECT_EQ(Period::parse("ON"), Period::days(1));
  EXPECT_EQ(Period::parse("TN"), Period::days(1));
  EXPECT_EQ(Period::parse("SN"), Period::days(1));
  EXPECT_FALSE(Period::parse("invalid").has_value());

  EXPECT_EQ(Period::months(3).to_string(), "3M");
  EXPECT_EQ(Period::years(5).to_string(), "5Y");

  std::ostringstream oss;
  oss << Period::months(6);
  EXPECT_EQ(oss.str(), "6M");

  EXPECT_EQ(std::format("Tenor: {}", Period::months(3)), "Tenor: 3M");
}
