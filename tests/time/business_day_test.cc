#include "time/business_day.h"
#include "time/calendar.h"
#include <gtest/gtest.h>

using namespace alm::time;

TEST(BusinessDayTest, StringConversionsAndParsing) {
  EXPECT_EQ(to_string(BusinessDayConvention::Following), "Following");
  EXPECT_EQ(to_string(BusinessDayConvention::ModifiedFollowing),
            "ModifiedFollowing");
  EXPECT_EQ(to_string(BusinessDayConvention::Preceding), "Preceding");
  EXPECT_EQ(to_string(BusinessDayConvention::ModifiedPreceding),
            "ModifiedPreceding");
  EXPECT_EQ(to_string(BusinessDayConvention::Unadjusted), "Unadjusted");

  EXPECT_EQ(parse_business_day_convention("Following"),
            BusinessDayConvention::Following);
  EXPECT_EQ(parse_business_day_convention("MF"),
            BusinessDayConvention::ModifiedFollowing);
  EXPECT_EQ(parse_business_day_convention("ModifiedFollowing"),
            BusinessDayConvention::ModifiedFollowing);
  EXPECT_EQ(parse_business_day_convention("Preceding"),
            BusinessDayConvention::Preceding);
  EXPECT_EQ(parse_business_day_convention("Unadjusted"),
            BusinessDayConvention::Unadjusted);
}

TEST(BusinessDayTest, StandardAdjustment) {
  WeekendsOnly cal;

  // Friday is already a business day
  Date fri(2024, 2, 2);
  EXPECT_EQ(adjust(fri, BusinessDayConvention::Following, cal), fri);
  EXPECT_EQ(adjust(fri, BusinessDayConvention::Preceding, cal), fri);
  EXPECT_EQ(adjust(fri, BusinessDayConvention::Unadjusted, cal), fri);

  // Saturday rolls
  Date sat(2024, 2, 3);
  Date mon(2024, 2, 5);
  EXPECT_EQ(adjust(sat, BusinessDayConvention::Following, cal), mon);
  EXPECT_EQ(adjust(sat, BusinessDayConvention::Preceding, cal), fri);
  EXPECT_EQ(adjust(sat, BusinessDayConvention::Unadjusted, cal), sat);
}

TEST(BusinessDayTest, ModifiedFollowing) {
  WeekendsOnly cal;

  // 2024-03-31 is Sunday (last day of March)
  // Following would roll to 2024-04-01 (next month)
  // ModifiedFollowing must roll backward to Friday 2024-03-29
  Date sun_eom(2024, 3, 31);
  EXPECT_EQ(adjust(sun_eom, BusinessDayConvention::Following, cal),
            Date(2024, 4, 1));
  EXPECT_EQ(adjust(sun_eom, BusinessDayConvention::ModifiedFollowing, cal),
            Date(2024, 3, 29));
}

TEST(BusinessDayTest, ModifiedPreceding) {
  WeekendsOnly cal;

  // 2024-06-01 is Saturday (first day of June)
  // Preceding would roll to Friday 2024-05-31 (prior month)
  // ModifiedPreceding must roll forward to Monday 2024-06-03
  Date sat_bom(2024, 6, 1);
  EXPECT_EQ(adjust(sat_bom, BusinessDayConvention::Preceding, cal),
            Date(2024, 5, 31));
  EXPECT_EQ(adjust(sat_bom, BusinessDayConvention::ModifiedPreceding, cal),
            Date(2024, 6, 3));
}

TEST(BusinessDayTest, NearestAdjustment) {
  WeekendsOnly cal;

  // Sunday 2024-02-04 is closer to Monday 2024-02-05 than Friday 2024-02-02
  Date sun(2024, 2, 4);
  EXPECT_EQ(adjust(sun, BusinessDayConvention::Nearest, cal), Date(2024, 2, 5));
}
