/**
 * @file business_day.h
 *
 * Defines business day rolling conventions used in financial contracts.
 *
 * Responsibilities:
 * - Define enumerations for standard conventions (Following, Modified
 * Following, Preceding, Modified Preceding, Unadjusted, etc.).
 * - Provide the interface to adjust a non-business day (holiday or weekend)
 *   to a valid business day based on a specified convention and Calendar.
 */

#pragma once

#include "time/date.h"

#include <cstdint>
#include <format>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

namespace alm::time {

class Calendar;

enum class BusinessDayConvention : uint8_t {
  Following = 0,
  ModifiedFollowing = 1,
  Preceding = 2,
  ModifiedPreceding = 3,
  Unadjusted = 4,
  HalfMonthModifiedFollowing = 5,
  Nearest = 6
};

[[nodiscard]] std::string to_string(BusinessDayConvention conv);
[[nodiscard]] std::optional<BusinessDayConvention>
parse_business_day_convention(std::string_view text);

std::ostream &operator<<(std::ostream &os, BusinessDayConvention conv);

// Adjusts a date according to the convention and calendar
[[nodiscard]] Date adjust(Date date, BusinessDayConvention conv,
                          const Calendar &calendar);

} // namespace alm::time

template <>
struct std::formatter<alm::time::BusinessDayConvention>
    : std::formatter<std::string> {
  auto format(alm::time::BusinessDayConvention conv,
              std::format_context &ctx) const {
    return std::formatter<std::string>::format(alm::time::to_string(conv), ctx);
  }
};
