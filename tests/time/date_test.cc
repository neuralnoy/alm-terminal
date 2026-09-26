#include "time/date.h"
#include <gtest/gtest.h>

using alm::time::Date;

TEST(DateTest, ConstructionAndFieldAccess) {
  Date d(2026, 9, 26);

  EXPECT_EQ(d.year(), 2026);
  EXPECT_EQ(d.month(), 9);
  EXPECT_EQ(d.day(), 26);
}
