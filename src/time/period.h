/**
 * @file period.h
 *
 * Defines explicit time intervals (Tenors) used in financial modeling.
 *
 * Responsibilities:
 * - Define a Period (or Tenor) representing an exact duration of time (e.g.
 * "3M", "5Y", "1W").
 * - Provide date arithmetic using periods.
 * - Serve as standard vocabulary for describing maturities, reset intervals,
 * and payment tenors.
 */

#pragma once

#include "time/date.h"
#include "time/frequency.h"

#include <compare>
#include <cstdint>
#include <format>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

namespace alm::time {

enum class TimeUnit : uint8_t { Days = 0, Weeks = 1, Months = 2, Years = 3 };

class Period {
public:
  constexpr Period() noexcept = default;
  constexpr Period(int length, TimeUnit units) noexcept
      : length_{length}, units_{units} {}

  // Convenience factories
  static constexpr Period days(int n) noexcept {
    return Period(n, TimeUnit::Days);
  }
  static constexpr Period weeks(int n) noexcept {
    return Period(n, TimeUnit::Weeks);
  }
  static constexpr Period months(int n) noexcept {
    return Period(n, TimeUnit::Months);
  }
  static constexpr Period years(int n) noexcept {
    return Period(n, TimeUnit::Years);
  }

  static Period from_frequency(Frequency freq);

  [[nodiscard]] constexpr int length() const noexcept { return length_; }
  [[nodiscard]] constexpr TimeUnit units() const noexcept { return units_; }

  [[nodiscard]] std::optional<Frequency> frequency() const noexcept;

  // Normalization, e.g. 12M -> 1Y, 7D -> 1W
  [[nodiscard]] Period normalize() const noexcept;

  // Arithmetic
  [[nodiscard]] constexpr Period operator-() const noexcept {
    return Period{-length_, units_};
  }

  [[nodiscard]] constexpr Period operator*(int scalar) const noexcept {
    return Period{length_ * scalar, units_};
  }

  constexpr Period &operator*=(int scalar) noexcept {
    length_ *= scalar;
    return *this;
  }

  Period operator+(const Period &other) const;
  Period operator-(const Period &other) const;

  // Comparisons
  bool operator==(const Period &other) const noexcept;
  std::partial_ordering operator<=>(const Period &other) const noexcept;

  // Formatting and parsing
  [[nodiscard]] std::string to_string() const;
  static std::optional<Period> parse(std::string_view text);

  friend std::ostream &operator<<(std::ostream &os, const Period &p);

private:
  int length_{0};
  TimeUnit units_{TimeUnit::Days};
};

[[nodiscard]] constexpr Period operator*(int scalar, const Period &p) noexcept {
  return p * scalar;
}

// Date arithmetic with Period
[[nodiscard]] constexpr Date advance(Date date, const Period &period,
                                     bool preserve_eom = false) noexcept {
  switch (period.units()) {
  case TimeUnit::Days:
    return date.add_days(period.length());
  case TimeUnit::Weeks:
    return date.add_days(static_cast<int64_t>(period.length()) * 7);
  case TimeUnit::Months:
    return date.add_months(period.length(), preserve_eom);
  case TimeUnit::Years:
    return date.add_years(period.length(), preserve_eom);
  }
  return date;
}

[[nodiscard]] constexpr Date operator+(Date date,
                                       const Period &period) noexcept {
  return advance(date, period, false);
}

[[nodiscard]] constexpr Date operator-(Date date,
                                       const Period &period) noexcept {
  return advance(date, -period, false);
}

} // namespace alm::time

template <>
struct std::formatter<alm::time::Period> : std::formatter<std::string> {
  auto format(const alm::time::Period &p, std::format_context &ctx) const {
    return std::formatter<std::string>::format(p.to_string(), ctx);
  }
};
