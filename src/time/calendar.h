/**
 * @file calendar.h
 *
 * Defines financial calendars and holiday schedules for global markets.
 *
 * Responsibilities:
 * - Provide high-performance calendar checks (business day, holiday, weekend).
 * - Implement market-specific holiday rules for TARGET, US, UK, Switzerland,
 * Japan.
 * - Support joint / composite calendars and custom holiday overrides.
 * - Provide business day rolling (adjust) and date advancement logic.
 */

#pragma once

#include "time/business_day.h"
#include "time/date.h"
#include "time/period.h"

#include <memory>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace alm::time {

class Calendar {
public:
  class Impl {
  public:
    virtual ~Impl() = default;
    [[nodiscard]] virtual std::string_view name() const = 0;
    [[nodiscard]] virtual bool is_business_day(Date date) const = 0;
    [[nodiscard]] virtual bool is_weekend(Date date) const {
      return date.is_weekend();
    }
  };

  Calendar() = default;
  explicit Calendar(std::shared_ptr<const Impl> impl);

  [[nodiscard]] bool is_business_day(Date date) const;
  [[nodiscard]] bool is_holiday(Date date) const {
    return !is_business_day(date);
  }
  [[nodiscard]] bool is_weekend(Date date) const;
  [[nodiscard]] bool is_end_of_month(Date date) const;
  [[nodiscard]] Date end_of_month(Date date) const;

  [[nodiscard]] Date
  adjust(Date date,
         BusinessDayConvention conv = BusinessDayConvention::Following) const;
  [[nodiscard]] Date
  advance(Date date, int n, TimeUnit unit,
          BusinessDayConvention conv = BusinessDayConvention::Following,
          bool end_of_month = false) const;
  [[nodiscard]] Date
  advance(Date date, const Period &period,
          BusinessDayConvention conv = BusinessDayConvention::Following,
          bool end_of_month = false) const;

  [[nodiscard]] int64_t business_days_between(Date from, Date to,
                                              bool include_first = false,
                                              bool include_last = true) const;
  [[nodiscard]] std::vector<Date> holiday_list(Date from, Date to) const;

  [[nodiscard]] std::string_view name() const;

  void add_holiday(Date date);
  void remove_holiday(Date date);

  bool operator==(const Calendar &other) const noexcept;

protected:
  std::shared_ptr<const Impl> impl_;
  std::shared_ptr<std::unordered_set<Date>> added_holidays_;
  std::shared_ptr<std::unordered_set<Date>> removed_holidays_;
};

// --- Standard Global Financial Calendars ---

class NullCalendar : public Calendar {
public:
  NullCalendar();
};

class WeekendsOnly : public Calendar {
public:
  WeekendsOnly();
};

class Target : public Calendar {
public:
  Target();
};

class UnitedStates : public Calendar {
public:
  enum class Market { Settlement, GovernmentBond, NYSE, SOFR };
  explicit UnitedStates(Market market = Market::GovernmentBond);
};

class UnitedKingdom : public Calendar {
public:
  enum class Market { Settlement, Exchange, Metals };
  explicit UnitedKingdom(Market market = Market::Settlement);
};

class Switzerland : public Calendar {
public:
  enum class Market { Settlement, SIX };
  explicit Switzerland(Market market = Market::Settlement);
};

class Japan : public Calendar {
public:
  Japan();
};

class JointCalendar : public Calendar {
public:
  enum class Rule {
    JoinHolidays,    // Day is holiday if holiday in ANY calendar (standard)
    JoinBusinessDays // Day is business day if business day in ANY calendar
  };

  JointCalendar(const Calendar &c1, const Calendar &c2,
                Rule rule = Rule::JoinHolidays);
  JointCalendar(const std::vector<Calendar> &calendars,
                Rule rule = Rule::JoinHolidays);
};

// Holiday calculation utilities
[[nodiscard]] constexpr Date easter_sunday(int year) noexcept {
  int a = year % 19;
  int b = year / 100;
  int c = year % 100;
  int d = b / 4;
  int e = b % 4;
  int f = (b + 8) / 25;
  int g = (b - f + 1) / 3;
  int h = (19 * a + b - d - g + 15) % 30;
  int i = c / 4;
  int k = c % 4;
  int l = (32 + 2 * e + 2 * i - h - k) % 7;
  int m = (a + 11 * h + 22 * l) / 451;
  int month = (h + l - 7 * m + 114) / 31;
  int day = ((h + l - 7 * m + 114) % 31) + 1;
  return Date(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
}

[[nodiscard]] Date nth_weekday(int n, std::chrono::weekday wd, unsigned month,
                               int year);
[[nodiscard]] Date last_weekday(std::chrono::weekday wd, unsigned month,
                                int year);

} // namespace alm::time
