#pragma once

#include "time/business_day.h"
#include "time/calendar.h"
#include "time/date.h"
#include "time/frequency.h"
#include "time/period.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace alm::time {

enum class DateGenerationRule : uint8_t {
  Backward,
  Forward,
  Zero,
  ThirdWednesday,
  Twentieth,
  TwentiethIMM
};

struct SchedulePeriod {
  Date start_date;
  Date end_date;
  Date unadjusted_start_date;
  Date unadjusted_end_date;
  bool is_regular{true};
};

class Schedule {
public:
  Schedule() = default;
  Schedule(std::vector<Date> dates, std::vector<Date> unadjusted_dates,
           std::vector<bool> is_regular, Calendar calendar,
           BusinessDayConvention convention);

  [[nodiscard]] size_t size() const noexcept { return dates_.size(); }
  [[nodiscard]] bool empty() const noexcept { return dates_.empty(); }
  [[nodiscard]] size_t periods_count() const noexcept {
    return dates_.empty() ? 0 : dates_.size() - 1;
  }

  [[nodiscard]] const Date &operator[](size_t index) const {
    return dates_[index];
  }
  [[nodiscard]] const Date &date(size_t index) const { return dates_[index]; }
  [[nodiscard]] const Date &unadjusted_date(size_t index) const {
    return unadjusted_dates_[index];
  }

  [[nodiscard]] const Date &start_date() const { return dates_.front(); }
  [[nodiscard]] const Date &end_date() const { return dates_.back(); }

  [[nodiscard]] const std::vector<Date> &dates() const noexcept {
    return dates_;
  }
  [[nodiscard]] const std::vector<Date> &unadjusted_dates() const noexcept {
    return unadjusted_dates_;
  }

  [[nodiscard]] bool is_regular(size_t period_index) const {
    return is_regular_[period_index];
  }

  [[nodiscard]] SchedulePeriod period(size_t index) const;

  // STL Iterators
  auto begin() const noexcept { return dates_.begin(); }
  auto end() const noexcept { return dates_.end(); }

  [[nodiscard]] const Calendar &calendar() const noexcept { return calendar_; }
  [[nodiscard]] BusinessDayConvention convention() const noexcept {
    return convention_;
  }

private:
  std::vector<Date> dates_;
  std::vector<Date> unadjusted_dates_;
  std::vector<bool> is_regular_;
  Calendar calendar_{WeekendsOnly()};
  BusinessDayConvention convention_{BusinessDayConvention::Following};
};

class ScheduleBuilder {
public:
  ScheduleBuilder() = default;

  ScheduleBuilder &from(Date effective_date);
  ScheduleBuilder &to(Date termination_date);
  ScheduleBuilder &tenor(const Period &tenor);
  ScheduleBuilder &frequency(Frequency freq);
  ScheduleBuilder &calendar(Calendar cal);
  ScheduleBuilder &convention(BusinessDayConvention conv);
  ScheduleBuilder &termination_convention(BusinessDayConvention conv);
  ScheduleBuilder &rule(DateGenerationRule rule);
  ScheduleBuilder &end_of_month(bool eom = true);
  ScheduleBuilder &first_date(Date d);
  ScheduleBuilder &next_to_last_date(Date d);

  [[nodiscard]] Schedule build() const;

private:
  Date effective_date_{};
  Date termination_date_{};
  Period tenor_{Period::months(6)};
  Calendar calendar_{WeekendsOnly()};
  BusinessDayConvention convention_{BusinessDayConvention::Following};
  std::optional<BusinessDayConvention> termination_convention_{};
  DateGenerationRule rule_{DateGenerationRule::Backward};
  bool end_of_month_{false};
  std::optional<Date> first_date_{};
  std::optional<Date> next_to_last_date_{};
};

[[nodiscard]] std::string to_string(DateGenerationRule rule);

} // namespace alm::time
