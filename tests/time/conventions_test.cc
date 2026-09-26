#include "time/conventions.h"
#include <gtest/gtest.h>

using namespace alm::time;

TEST(ConventionsTest, SofrOisProperties) {
  auto conv = MarketConvention::sofr_ois();
  EXPECT_EQ(conv.name(), "USD SOFR OIS");
  EXPECT_EQ(conv.day_counter().convention(), DayCountConvention::Actual360);
  EXPECT_EQ(conv.business_day_convention(),
            BusinessDayConvention::ModifiedFollowing);
  EXPECT_EQ(conv.settlement_days(), 2);
  EXPECT_EQ(conv.payment_frequency(), Frequency::Annual);

  // Trade on Thursday 2024-02-01 -> T+2 is Monday 2024-02-05
  Date trade_date(2024, 2, 1);
  EXPECT_EQ(conv.settlement_date(trade_date), Date(2024, 2, 5));
}

TEST(ConventionsTest, SoniaOisProperties) {
  auto conv = MarketConvention::sonia_ois();
  EXPECT_EQ(conv.name(), "GBP SONIA OIS");
  EXPECT_EQ(conv.day_counter().convention(),
            DayCountConvention::Actual365Fixed);
  EXPECT_EQ(conv.settlement_days(), 0);

  // T+0 settlement
  Date trade_date(2024, 2, 1);
  EXPECT_EQ(conv.settlement_date(trade_date), Date(2024, 2, 1));
}

TEST(ConventionsTest, UsTreasuryProperties) {
  auto conv = MarketConvention::us_treasury();
  EXPECT_EQ(conv.name(), "USD Treasury");
  EXPECT_EQ(conv.day_counter().convention(),
            DayCountConvention::ActualActualICMA);
  EXPECT_EQ(conv.business_day_convention(), BusinessDayConvention::Following);
  EXPECT_EQ(conv.settlement_days(), 1);
  EXPECT_EQ(conv.payment_frequency(), Frequency::SemiAnnual);

  // Trade on Friday 2024-02-02 -> T+1 is Monday 2024-02-05
  Date fri(2024, 2, 2);
  EXPECT_EQ(conv.settlement_date(fri), Date(2024, 2, 5));
}

TEST(ConventionsTest, Lookup) {
  auto sofr = MarketConvention::lookup("SOFR");
  ASSERT_TRUE(sofr.has_value());
  EXPECT_EQ(sofr->name(), "USD SOFR OIS");

  auto bund = MarketConvention::lookup("German Bund");
  ASSERT_TRUE(bund.has_value());
  EXPECT_EQ(bund->name(), "EUR German Bund");

  auto gilt = MarketConvention::lookup("Gilt");
  ASSERT_TRUE(gilt.has_value());
  EXPECT_EQ(gilt->name(), "GBP UK Gilt");

  EXPECT_FALSE(MarketConvention::lookup("NonExistent").has_value());
}
