#pragma once

#include <chrono>
#include <cstdint>
#include <format>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

namespace alm::time {

class Date {
public:
  // Constructors
  constexpr Date() noexcept = default;
  constexpr Date(std::chrono::year y, std::chrono::month m,
                 std::chrono::day d) noexcept;
  constexpr Date(int year, unsigned month, unsigned day) noexcept;
  constexpr explicit Date(std::chrono::sys_days tp) noexcept;

  // Static factory & sentinel values
  static Date today() noexcept;
  static constexpr Date min() noexcept {
    return Date{std::chrono::sys_days{std::chrono::days{-292277022}}};
  }
  static constexpr Date max() noexcept {
    return Date{std::chrono::sys_days{std::chrono::days{292277022}}};
  }

  // Validity checks
  static constexpr bool is_valid_date(int year, unsigned month,
                                      unsigned day) noexcept {
    auto ymd = std::chrono::year_month_day{std::chrono::year{year},
                                           std::chrono::month{month},
                                           std::chrono::day{day}};
    return ymd.ok();
  }

  [[nodiscard]] constexpr bool is_valid() const noexcept {
    return std::chrono::year_month_day{tp_}.ok();
  }

  // Observers
  [[nodiscard]] constexpr int year() const noexcept {
    return static_cast<int>(std::chrono::year_month_day{tp_}.year());
  }

  [[nodiscard]] constexpr unsigned month() const noexcept {
    return static_cast<unsigned>(std::chrono::year_month_day{tp_}.month());
  }

  [[nodiscard]] constexpr unsigned day() const noexcept {
    return static_cast<unsigned>(std::chrono::year_month_day{tp_}.day());
  }

  [[nodiscard]] constexpr std::chrono::weekday weekday() const noexcept {
    return std::chrono::weekday{tp_};
  }

  /// ISO weekday number: 1 = Monday, 2 = Tuesday, ..., 7 = Sunday.
  [[nodiscard]] constexpr unsigned day_of_week() const noexcept {
    return weekday().iso_encoding();
  }

  [[nodiscard]] constexpr int day_of_year() const noexcept {
    auto ymd = std::chrono::year_month_day{tp_};
    auto start_of_year = std::chrono::sys_days{std::chrono::year_month_day{
        ymd.year(), std::chrono::month{1}, std::chrono::day{1}}};
    return static_cast<int>((tp_ - start_of_year).count()) + 1;
  }

  [[nodiscard]] constexpr bool is_weekend() const noexcept {
    auto wd = weekday();
    return wd == std::chrono::Saturday || wd == std::chrono::Sunday;
  }

  [[nodiscard]] constexpr bool is_end_of_month() const noexcept {
    auto ymd = std::chrono::year_month_day{tp_};
    auto last_day = std::chrono::year_month_day_last{
        ymd.year(), std::chrono::month_day_last{ymd.month()}};
    return ymd.day() == last_day.day();
  }

  [[nodiscard]] constexpr Date end_of_month() const noexcept {
    auto ymd = std::chrono::year_month_day{tp_};
    auto last_day = std::chrono::year_month_day_last{
        ymd.year(), std::chrono::month_day_last{ymd.month()}};
    return Date{std::chrono::sys_days{last_day}};
  }

  [[nodiscard]] constexpr bool is_leap_year() const noexcept {
    return std::chrono::year_month_day{tp_}.year().is_leap();
  }

  static constexpr bool is_leap_year(int year) noexcept {
    return std::chrono::year{year}.is_leap();
  }

  // Conversions
  [[nodiscard]] constexpr std::chrono::sys_days to_sys_days() const noexcept {
    return tp_;
  }
  [[nodiscard]] constexpr std::chrono::year_month_day
  to_year_month_day() const noexcept {
    return std::chrono::year_month_day{tp_};
  }
  constexpr explicit operator std::chrono::sys_days() const noexcept {
    return tp_;
  }

  // Arithmetic
  [[nodiscard]] constexpr Date add_days(int64_t days) const noexcept {
    return Date{tp_ + std::chrono::days{days}};
  }

  [[nodiscard]] constexpr Date
  add_months(int months, bool preserve_eom = false) const noexcept {
    auto ymd = std::chrono::year_month_day{tp_};
    bool was_eom = preserve_eom && is_end_of_month();

    auto new_ym = ymd.year() / ymd.month() + std::chrono::months{months};
    auto target_y = new_ym.year();
    auto target_m = new_ym.month();

    if (was_eom) {
      auto last_day = std::chrono::year_month_day_last{
          target_y, std::chrono::month_day_last{target_m}};
      return Date{std::chrono::sys_days{last_day}};
    }

    auto last_day = std::chrono::year_month_day_last{
        target_y, std::chrono::month_day_last{target_m}};
    unsigned max_d = static_cast<unsigned>(last_day.day());
    unsigned cur_d = static_cast<unsigned>(ymd.day());
    unsigned target_d = cur_d < max_d ? cur_d : max_d;
    return Date{target_y, target_m, std::chrono::day{target_d}};
  }

  [[nodiscard]] constexpr Date
  add_years(int years, bool preserve_eom = false) const noexcept {
    return add_months(years * 12, preserve_eom);
  }

  constexpr Date &operator+=(int64_t days) noexcept {
    tp_ += std::chrono::days{days};
    return *this;
  }

  constexpr Date &operator-=(int64_t days) noexcept {
    tp_ -= std::chrono::days{days};
    return *this;
  }

  constexpr Date &operator++() noexcept {
    tp_ += std::chrono::days{1};
    return *this;
  }

  constexpr Date operator++(int) noexcept {
    Date tmp = *this;
    tp_ += std::chrono::days{1};
    return tmp;
  }

  constexpr Date &operator--() noexcept {
    tp_ -= std::chrono::days{1};
    return *this;
  }

  constexpr Date operator--(int) noexcept {
    Date tmp = *this;
    tp_ -= std::chrono::days{1};
    return tmp;
  }

  [[nodiscard]] constexpr Date operator+(int64_t days) const noexcept {
    return add_days(days);
  }

  [[nodiscard]] constexpr Date operator-(int64_t days) const noexcept {
    return add_days(-days);
  }

  [[nodiscard]] constexpr int64_t operator-(const Date &other) const noexcept {
    return (tp_ - other.tp_).count();
  }

  // Comparisons
  constexpr auto operator<=>(const Date &other) const noexcept = default;
  constexpr bool operator==(const Date &other) const noexcept = default;

  // Formatting and Parsing
  [[nodiscard]] std::string to_string() const;
  static std::optional<Date> parse(std::string_view text);

  // Stream output
  friend std::ostream &operator<<(std::ostream &os, const Date &date);

private:
  std::chrono::sys_days tp_{std::chrono::days{0}};
};

constexpr inline Date::Date(std::chrono::year y, std::chrono::month m,
                            std::chrono::day d) noexcept
    : tp_{std::chrono::year_month_day{y, m, d}} {}

constexpr inline Date::Date(int year, unsigned month, unsigned day) noexcept
    : Date(std::chrono::year{year}, std::chrono::month{month},
           std::chrono::day{day}) {}

constexpr inline Date::Date(std::chrono::sys_days tp) noexcept : tp_{tp} {}

} // namespace alm::time

// Standard hash support
template <> struct std::hash<alm::time::Date> {
  std::size_t operator()(const alm::time::Date &d) const noexcept {
    return std::hash<int64_t>{}(d.to_sys_days().time_since_epoch().count());
  }
};

// C++20/C++23 std::format support
template <>
struct std::formatter<alm::time::Date> : std::formatter<std::string> {
  auto format(const alm::time::Date &d, std::format_context &ctx) const {
    return std::formatter<std::string>::format(d.to_string(), ctx);
  }
};
