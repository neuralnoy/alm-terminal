#include "time/date.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>
#include <unordered_set>
#include <vector>

using alm::time::Date;

TEST(DateTest, ConstructionAndFieldAccess) {
  Date d(2026, 9, 26);

  EXPECT_EQ(d.year(), 2026);
  EXPECT_EQ(d.month(), 9);
  EXPECT_EQ(d.day(), 26);
  EXPECT_TRUE(d.is_valid());
}

TEST(DateTest, DayOfWeekAndWeekend) {
  // 2026-09-26 is a Saturday
  Date sat(2026, 9, 26);
  EXPECT_EQ(sat.day_of_week(), 6);
  EXPECT_TRUE(sat.is_weekend());

  // 2026-09-27 is a Sunday
  Date sun(2026, 9, 27);
  EXPECT_EQ(sun.day_of_week(), 7);
  EXPECT_TRUE(sun.is_weekend());

  // 2026-09-28 is a Monday
  Date mon(2026, 9, 28);
  EXPECT_EQ(mon.day_of_week(), 1);
  EXPECT_FALSE(mon.is_weekend());

  // 2026-09-25 is a Friday
  Date fri(2026, 9, 25);
  EXPECT_EQ(fri.day_of_week(), 5);
  EXPECT_FALSE(fri.is_weekend());
}

TEST(DateTest, LeapYears) {
  EXPECT_TRUE(Date::is_leap_year(2000));
  EXPECT_FALSE(Date::is_leap_year(1900));
  EXPECT_TRUE(Date::is_leap_year(2024));
  EXPECT_FALSE(Date::is_leap_year(2025));

  Date leap(2024, 2, 29);
  EXPECT_TRUE(leap.is_leap_year());
  EXPECT_TRUE(leap.is_end_of_month());

  Date non_leap(2023, 2, 28);
  EXPECT_FALSE(non_leap.is_leap_year());
  EXPECT_TRUE(non_leap.is_end_of_month());
}

TEST(DateTest, DayOfYear) {
  Date jan1(2024, 1, 1);
  EXPECT_EQ(jan1.day_of_year(), 1);

  Date feb29(2024, 2, 29);
  EXPECT_EQ(feb29.day_of_year(), 60);

  Date dec31(2024, 12, 31);
  EXPECT_EQ(dec31.day_of_year(), 366);
}

TEST(DateTest, EndOfMonth) {
  Date d1(2024, 1, 15);
  EXPECT_FALSE(d1.is_end_of_month());
  EXPECT_EQ(d1.end_of_month(), Date(2024, 1, 31));

  Date d2(2024, 2, 10);
  EXPECT_EQ(d2.end_of_month(), Date(2024, 2, 29));

  Date d3(2023, 2, 10);
  EXPECT_EQ(d3.end_of_month(), Date(2023, 2, 28));

  Date d4(2024, 4, 30);
  EXPECT_TRUE(d4.is_end_of_month());
}

TEST(DateTest, DayArithmetic) {
  Date d(2024, 2, 28);
  EXPECT_EQ(d + 1, Date(2024, 2, 29));
  EXPECT_EQ(d + 2, Date(2024, 3, 1));
  EXPECT_EQ(d - 1, Date(2024, 2, 27));

  Date diff1(2024, 3, 1);
  Date diff2(2024, 2, 28);
  EXPECT_EQ(diff1 - diff2, 2);
  EXPECT_EQ(diff2 - diff1, -2);

  Date inc(2024, 2, 28);
  EXPECT_EQ(++inc, Date(2024, 2, 29));
  EXPECT_EQ(inc++, Date(2024, 2, 29));
  EXPECT_EQ(inc, Date(2024, 3, 1));
  EXPECT_EQ(--inc, Date(2024, 2, 29));
  EXPECT_EQ(inc--, Date(2024, 2, 29));
  EXPECT_EQ(inc, Date(2024, 2, 28));

  inc += 5;
  EXPECT_EQ(inc, Date(2024, 3, 4));
  inc -= 5;
  EXPECT_EQ(inc, Date(2024, 2, 28));
}

TEST(DateTest, MonthAndYearArithmetic) {
  // Clamping without preserve_eom
  Date d1(2024, 1, 31);
  EXPECT_EQ(d1.add_months(1, false), Date(2024, 2, 29));
  EXPECT_EQ(d1.add_months(2, false), Date(2024, 3, 31));
  EXPECT_EQ(d1.add_months(3, false), Date(2024, 4, 30));

  // With preserve_eom = true from 28 Feb (non-leap year, which is EOM)
  Date feb_eom(2023, 2, 28);
  EXPECT_TRUE(feb_eom.is_end_of_month());
  EXPECT_EQ(feb_eom.add_months(1, true), Date(2023, 3, 31));
  EXPECT_EQ(feb_eom.add_months(2, true), Date(2023, 4, 30));

  // Without preserve_eom from 28 Feb non-leap
  EXPECT_EQ(feb_eom.add_months(1, false), Date(2023, 3, 28));

  // Adding years
  Date leap_day(2024, 2, 29);
  EXPECT_EQ(leap_day.add_years(1, false), Date(2025, 2, 28));
  EXPECT_EQ(leap_day.add_years(4, false), Date(2028, 2, 29));

  // Negative months
  Date march31(2024, 3, 31);
  EXPECT_EQ(march31.add_months(-1, false), Date(2024, 2, 29));
  EXPECT_EQ(march31.add_months(-2, false), Date(2024, 1, 31));
}

TEST(DateTest, ComparisonAndSorting) {
  Date d1(2024, 1, 15);
  Date d2(2024, 1, 15);
  Date d3(2024, 2, 1);
  Date d4(2025, 1, 1);

  EXPECT_EQ(d1, d2);
  EXPECT_LT(d1, d3);
  EXPECT_LE(d1, d2);
  EXPECT_GT(d4, d3);
  EXPECT_GE(d4, d1);

  std::vector<Date> dates = {d4, d1, d3};
  std::sort(dates.begin(), dates.end());
  EXPECT_EQ(dates[0], d1);
  EXPECT_EQ(dates[1], d3);
  EXPECT_EQ(dates[2], d4);
}

TEST(DateTest, FormattingAndStreaming) {
  Date d(2026, 9, 26);
  EXPECT_EQ(d.to_string(), "2026-09-26");

  std::ostringstream oss;
  oss << d;
  EXPECT_EQ(oss.str(), "2026-09-26");

  std::string formatted = std::format("Date is {}", d);
  EXPECT_EQ(formatted, "Date is 2026-09-26");
}

TEST(DateTest, Parsing) {
  auto parsed = Date::parse("2026-09-26");
  ASSERT_TRUE(parsed.has_value());
  EXPECT_EQ(*parsed, Date(2026, 9, 26));

  auto parsed_compact = Date::parse("20260926");
  ASSERT_TRUE(parsed_compact.has_value());
  EXPECT_EQ(*parsed_compact, Date(2026, 9, 26));

  // Invalid date
  EXPECT_FALSE(Date::parse("2023-02-29").has_value());
  EXPECT_FALSE(Date::parse("not-a-date").has_value());
  EXPECT_FALSE(Date::parse("2026-13-01").has_value());
}

TEST(DateTest, HashSupport) {
  std::unordered_set<Date> date_set;
  date_set.insert(Date(2026, 9, 26));
  date_set.insert(Date(2026, 9, 27));
  date_set.insert(Date(2026, 9, 26)); // Duplicate

  EXPECT_EQ(date_set.size(), 2);
  EXPECT_TRUE(date_set.contains(Date(2026, 9, 26)));
  EXPECT_FALSE(date_set.contains(Date(2026, 9, 28)));
}
