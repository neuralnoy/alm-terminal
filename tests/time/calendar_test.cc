#include "time/calendar.h"
#include <gtest/gtest.h>

using namespace alm::time;

TEST(CalendarTest, EasterSunday) {
  EXPECT_EQ(easter_sunday(2024), Date(2024, 3, 31));
  EXPECT_EQ(easter_sunday(2025), Date(2025, 4, 20));
  EXPECT_EQ(easter_sunday(2026), Date(2026, 4, 5));
}

TEST(CalendarTest, TargetHolidays) {
  Target cal;
  EXPECT_EQ(cal.name(), "TARGET");

  // New Year's Day
  EXPECT_FALSE(cal.is_business_day(Date(2024, 1, 1)));
  // Regular business day
  EXPECT_TRUE(cal.is_business_day(Date(2024, 1, 2)));

  // Good Friday & Easter Monday 2024
  EXPECT_FALSE(cal.is_business_day(Date(2024, 3, 29)));
  EXPECT_FALSE(cal.is_business_day(Date(2024, 4, 1)));

  // Labour Day
  EXPECT_FALSE(cal.is_business_day(Date(2024, 5, 1)));

  // Christmas & Boxing Day
  EXPECT_FALSE(cal.is_business_day(Date(2024, 12, 25)));
  EXPECT_FALSE(cal.is_business_day(Date(2024, 12, 26)));
}

TEST(CalendarTest, UnitedStatesHolidays) {
  UnitedStates cal(UnitedStates::Market::GovernmentBond);

  // MLK Day 2024: 3rd Monday in Jan -> Jan 15
  EXPECT_FALSE(cal.is_business_day(Date(2024, 1, 15)));

  // Presidents Day 2024: 3rd Monday in Feb -> Feb 19
  EXPECT_FALSE(cal.is_business_day(Date(2024, 2, 19)));

  // Independence Day 2024: July 4
  EXPECT_FALSE(cal.is_business_day(Date(2024, 7, 4)));

  // Thanksgiving 2024: 4th Thu in Nov -> Nov 28
  EXPECT_FALSE(cal.is_business_day(Date(2024, 11, 28)));
}

TEST(CalendarTest, CustomHolidayOverrides) {
  Target cal;
  Date test_date(2024, 6, 12);
  EXPECT_TRUE(cal.is_business_day(test_date));

  cal.add_holiday(test_date);
  EXPECT_FALSE(cal.is_business_day(test_date));

  cal.remove_holiday(test_date);
  EXPECT_TRUE(cal.is_business_day(test_date));
}

TEST(CalendarTest, JointCalendar) {
  UnitedStates us(UnitedStates::Market::GovernmentBond);
  Target target;

  JointCalendar joint(us, target, JointCalendar::Rule::JoinHolidays);

  // July 4 is holiday in US, but regular business day in TARGET
  Date jul4(2024, 7, 4);
  EXPECT_FALSE(us.is_business_day(jul4));
  EXPECT_TRUE(target.is_business_day(jul4));
  // In joint holidays, it is a holiday
  EXPECT_FALSE(joint.is_business_day(jul4));
}

TEST(CalendarTest, BusinessDaysBetween) {
  WeekendsOnly cal;
  Date d1(2024, 2, 2); // Friday
  Date d2(2024, 2, 9); // Next Friday

  // Monday to Friday: 5 business days
  EXPECT_EQ(cal.business_days_between(d1, d2, false, true), 5);
  EXPECT_EQ(cal.business_days_between(d1, d2, true, true), 6);
  EXPECT_EQ(cal.business_days_between(d2, d1, false, true), -5);
}

TEST(CalendarTest, Advance) {
  WeekendsOnly cal;
  Date fri(2024, 2, 2);

  // Advance 1 business day -> Monday
  Date mon = cal.advance(fri, 1, TimeUnit::Days);
  EXPECT_EQ(mon, Date(2024, 2, 5));

  // Advance 5 business days -> Next Friday
  Date next_fri = cal.advance(fri, 5, TimeUnit::Days);
  EXPECT_EQ(next_fri, Date(2024, 2, 9));

  // Advance 1 month with Following
  Date d(2024, 1, 31);
  Date adv_m = cal.advance(d, Period::months(1),
                           BusinessDayConvention::ModifiedFollowing);
  // Feb 2024 ends on Feb 29 (Thursday)
  EXPECT_EQ(adv_m, Date(2024, 2, 29));
}

TEST(CalendarTest, EndOfMonth) {
  Target cal;
  Date mid_mar(2024, 3, 15);
  // Good Friday 2024 was Mar 29, so last business day was Thursday Mar 28!
  Date b_eom = cal.end_of_month(mid_mar);
  EXPECT_EQ(b_eom, Date(2024, 3, 28));
  EXPECT_TRUE(cal.is_end_of_month(b_eom));
}
