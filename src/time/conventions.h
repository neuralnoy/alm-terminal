#pragma once

#include "time/business_day.h"
#include "time/calendar.h"
#include "time/date.h"
#include "time/day_count.h"
#include "time/frequency.h"

#include <optional>
#include <string>
#include <string_view>

namespace alm::time {

class MarketConvention {
public:
  MarketConvention() = default;
  MarketConvention(std::string name, Calendar calendar, DayCounter day_counter,
                   BusinessDayConvention business_day_convention,
                   int settlement_days,
                   Frequency payment_frequency = Frequency::Annual,
                   bool end_of_month = false);

  // Accessors
  [[nodiscard]] std::string_view name() const noexcept { return name_; }
  [[nodiscard]] const Calendar &calendar() const noexcept { return calendar_; }
  [[nodiscard]] const DayCounter &day_counter() const noexcept {
    return day_counter_;
  }
  [[nodiscard]] BusinessDayConvention business_day_convention() const noexcept {
    return bdc_;
  }
  [[nodiscard]] int settlement_days() const noexcept {
    return settlement_days_;
  }
  [[nodiscard]] Frequency payment_frequency() const noexcept {
    return payment_freq_;
  }
  [[nodiscard]] bool end_of_month() const noexcept { return end_of_month_; }

  // Calculation helpers
  [[nodiscard]] Date settlement_date(Date trade_date) const;
  [[nodiscard]] Date adjust(Date date) const;

  // Pre-defined market conventions
  static MarketConvention sofr_ois();
  static MarketConvention estr_ois();
  static MarketConvention sonia_ois();
  static MarketConvention euribor_3m();
  static MarketConvention euribor_6m();
  static MarketConvention us_treasury();
  static MarketConvention german_bund();
  static MarketConvention uk_gilt();

  // Central lookup
  static std::optional<MarketConvention> lookup(std::string_view name);

private:
  std::string name_{"Unknown"};
  Calendar calendar_{WeekendsOnly()};
  DayCounter day_counter_{DayCounter::actual_360()};
  BusinessDayConvention bdc_{BusinessDayConvention::ModifiedFollowing};
  int settlement_days_{2};
  Frequency payment_freq_{Frequency::Annual};
  bool end_of_month_{false};
};

} // namespace alm::time
