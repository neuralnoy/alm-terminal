/**
 * @file date.hpp
 *
 * Foundational date types and basic date arithmetic for the ALM engine.
 *
 * Responsibilities:
 * - Wrap, alias, or extend standard C++20 date types
 * (std::chrono::year_month_day) for ergonomic use throughout the financial
 * domain.
 * - Provide common date manipulation functions (e.g., adding months, days,
 * years) without adjusting for holidays.
 * - Define standard interfaces for date serialization, formatting, and parsing.
 */

#pragma once

#include <chrono>

namespace alm::time {
class Date {
public:
  // Constructors
  Date() = default;
  Date(std::chrono::year y, std::chrono::month m, std::chrono::day d);
  Date(int year, unsigned month, unsigned day);
  explicit Date(std::chrono::sys_days tp);

  // Observers
  int year() const;
  unsigned month() const;
  unsigned day() const;

private:
  std::chrono::sys_days tp_{};
};
} // namespace alm::time
