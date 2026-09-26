#include "time/schedule.h"
#include <gtest/gtest.h>

using namespace alm::time;

TEST(ScheduleTest, RegularSemiAnnualBackward) {
  // 1-year semi-annual schedule
  Date start(2024, 1, 15);
  Date end(2025, 1, 15);

  Schedule s = ScheduleBuilder()
                   .from(start)
                   .to(end)
                   .tenor(Period::months(6))
                   .calendar(Target())
                   .convention(BusinessDayConvention::ModifiedFollowing)
                   .rule(DateGenerationRule::Backward)
                   .build();

  EXPECT_EQ(s.size(), 3);
  EXPECT_EQ(s.periods_count(), 2);
  EXPECT_EQ(s[0], Date(2024, 1, 15));
  EXPECT_EQ(s[1], Date(2024, 7, 15));
  EXPECT_EQ(s[2], Date(2025, 1, 15));

  EXPECT_TRUE(s.is_regular(0));
  EXPECT_TRUE(s.is_regular(1));

  auto p0 = s.period(0);
  EXPECT_EQ(p0.start_date, Date(2024, 1, 15));
  EXPECT_EQ(p0.end_date, Date(2024, 7, 15));
  EXPECT_TRUE(p0.is_regular);
}

TEST(ScheduleTest, FrontStubBackward) {
  // Start on Feb 15, 2024; end on Jan 15, 2025. Tenor 6M.
  // Backward stepping from 2025-01-15 -> 2024-07-15 -> 2024-01-15 (breaks)
  // Generates: 2024-02-15, 2024-07-15, 2025-01-15.
  // Period 0 (Feb 15 to Jul 15) is a short stub (5 months).
  Date start(2024, 2, 15);
  Date end(2025, 1, 15);

  Schedule s = ScheduleBuilder()
                   .from(start)
                   .to(end)
                   .tenor(Period::months(6))
                   .calendar(Target())
                   .convention(BusinessDayConvention::ModifiedFollowing)
                   .rule(DateGenerationRule::Backward)
                   .build();

  EXPECT_EQ(s.size(), 3);
  EXPECT_EQ(s[0], Date(2024, 2, 15));
  EXPECT_EQ(s[1], Date(2024, 7, 15));
  EXPECT_EQ(s[2], Date(2025, 1, 15));

  EXPECT_FALSE(s.is_regular(0)); // front stub!
  EXPECT_TRUE(s.is_regular(1));  // regular 6M period
}

TEST(ScheduleTest, BackStubForward) {
  // Start on Jan 15, 2024; end on Nov 15, 2024. Tenor 6M.
  // Forward stepping: 2024-01-15 -> 2024-07-15 -> 2025-01-15 (breaks)
  // Generates: 2024-01-15, 2024-07-15, 2024-11-15.
  // Period 1 (Jul 15 to Nov 15) is a short stub (4 months).
  Date start(2024, 1, 15);
  Date end(2024, 11, 15);

  Schedule s = ScheduleBuilder()
                   .from(start)
                   .to(end)
                   .tenor(Period::months(6))
                   .calendar(Target())
                   .convention(BusinessDayConvention::ModifiedFollowing)
                   .rule(DateGenerationRule::Forward)
                   .build();

  EXPECT_EQ(s.size(), 3);
  EXPECT_EQ(s[0], Date(2024, 1, 15));
  EXPECT_EQ(s[1], Date(2024, 7, 15));
  EXPECT_EQ(s[2], Date(2024, 11, 15));

  EXPECT_TRUE(s.is_regular(0));
  EXPECT_FALSE(s.is_regular(1)); // back stub
}

TEST(ScheduleTest, ZeroCouponSchedule) {
  Date start(2024, 1, 15);
  Date end(2026, 1, 15);

  Schedule s = ScheduleBuilder()
                   .from(start)
                   .to(end)
                   .rule(DateGenerationRule::Zero)
                   .build();

  EXPECT_EQ(s.size(), 2);
  EXPECT_EQ(s.periods_count(), 1);
  EXPECT_EQ(s[0], start);
  EXPECT_EQ(s[1], end);
}

TEST(ScheduleTest, BusinessDayAdjustmentInSchedule) {
  // 2024-06-30 is Sunday.
  Date start(2023, 12,
             31); // Sunday -> adjusted to Friday Dec 29 with ModFollowing or
                  // Mon Jan 1 (TARGET holiday) -> Tue Jan 2
  Date end(2024, 12, 31);

  Schedule s = ScheduleBuilder()
                   .from(start)
                   .to(end)
                   .tenor(Period::months(6))
                   .calendar(Target())
                   .convention(BusinessDayConvention::ModifiedFollowing)
                   .rule(DateGenerationRule::Backward)
                   .build();

  // 2024-06-30 is Sunday -> ModifiedFollowing rolls backward to Friday
  // 2024-06-28
  EXPECT_EQ(s.unadjusted_date(1), Date(2024, 6, 30));
  EXPECT_EQ(s.date(1), Date(2024, 6, 28));
}

TEST(ScheduleTest, EndOfMonthSchedule) {
  Date start(2024, 2, 29); // Leap year EOM
  Date end(2025, 2, 28);   // Non-leap year EOM

  Schedule s = ScheduleBuilder()
                   .from(start)
                   .to(end)
                   .tenor(Period::months(6))
                   .calendar(WeekendsOnly())
                   .convention(BusinessDayConvention::ModifiedFollowing)
                   .rule(DateGenerationRule::Backward)
                   .end_of_month(true)
                   .build();

  EXPECT_EQ(s.size(), 3);
  // Unadjusted intermediate date for EOM stepping should be 2024-08-31
  EXPECT_EQ(s.unadjusted_date(1), Date(2024, 8, 31));
}
