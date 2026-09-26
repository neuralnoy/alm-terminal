#pragma once

#include "time/date.h"
#include "time/frequency.h"

#include <cstdint>
#include <format>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

namespace alm::time {

enum class DayCountConvention : uint8_t {
  Actual360,
  Actual365Fixed,
  Actual365NoLeap,
  ActualActualISDA,
  ActualActualICMA,
  Thirty360US,
  ThirtyE360,
  ThirtyE360ISDA,
  OneOne
};

class DayCounter {
public:
  constexpr DayCounter() noexcept
      : conv_{DayCountConvention::Actual360}, freq_{Frequency::Annual} {}
  constexpr explicit DayCounter(DayCountConvention conv,
                                Frequency freq = Frequency::Annual) noexcept
      : conv_{conv}, freq_{freq} {}

  // Standard factories
  static constexpr DayCounter actual_360() noexcept {
    return DayCounter(DayCountConvention::Actual360);
  }
  static constexpr DayCounter actual_365_fixed() noexcept {
    return DayCounter(DayCountConvention::Actual365Fixed);
  }
  static constexpr DayCounter actual_365_no_leap() noexcept {
    return DayCounter(DayCountConvention::Actual365NoLeap);
  }
  static constexpr DayCounter actual_actual_isda() noexcept {
    return DayCounter(DayCountConvention::ActualActualISDA);
  }
  static constexpr DayCounter
  actual_actual_icma(Frequency freq = Frequency::SemiAnnual) noexcept {
    return DayCounter(DayCountConvention::ActualActualICMA, freq);
  }
  static constexpr DayCounter thirty_360_us() noexcept {
    return DayCounter(DayCountConvention::Thirty360US);
  }
  static constexpr DayCounter thirty_e_360() noexcept {
    return DayCounter(DayCountConvention::ThirtyE360);
  }
  static constexpr DayCounter thirty_e_360_isda() noexcept {
    return DayCounter(DayCountConvention::ThirtyE360ISDA);
  }
  static constexpr DayCounter one_one() noexcept {
    return DayCounter(DayCountConvention::OneOne);
  }

  [[nodiscard]] int64_t day_count(Date start, Date end) const noexcept;
  [[nodiscard]] double year_fraction(Date start, Date end,
                                     Date ref_start = Date{},
                                     Date ref_end = Date{}) const noexcept;

  [[nodiscard]] std::string_view name() const noexcept;
  [[nodiscard]] constexpr DayCountConvention convention() const noexcept {
    return conv_;
  }
  [[nodiscard]] constexpr Frequency frequency() const noexcept { return freq_; }

  static std::optional<DayCounter> parse(std::string_view text);

  constexpr bool operator==(const DayCounter &other) const noexcept {
    return conv_ == other.conv_ && freq_ == other.freq_;
  }

private:
  DayCountConvention conv_{DayCountConvention::Actual360};
  Frequency freq_{Frequency::Annual};
};

[[nodiscard]] std::string to_string(DayCountConvention conv);
[[nodiscard]] std::optional<DayCountConvention>
parse_day_count_convention(std::string_view text);

std::ostream &operator<<(std::ostream &os, DayCountConvention conv);
std::ostream &operator<<(std::ostream &os, const DayCounter &dc);

} // namespace alm::time

template <>
struct std::formatter<alm::time::DayCountConvention>
    : std::formatter<std::string> {
  auto format(alm::time::DayCountConvention conv,
              std::format_context &ctx) const {
    return std::formatter<std::string>::format(alm::time::to_string(conv), ctx);
  }
};

template <>
struct std::formatter<alm::time::DayCounter> : std::formatter<std::string> {
  auto format(const alm::time::DayCounter &dc, std::format_context &ctx) const {
    return std::formatter<std::string>::format(std::string(dc.name()), ctx);
  }
};
