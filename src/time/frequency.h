#pragma once

#include <cstdint>
#include <format>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

namespace alm::time {

enum class Frequency : int32_t {
  NoFrequency = -1,
  Once = 0,
  Annual = 1,
  SemiAnnual = 2,
  EveryFourthMonth = 3,
  Quarterly = 4,
  Bimonthly = 6,
  Monthly = 12,
  EveryFourthWeek = 13,
  Biweekly = 26,
  Weekly = 52,
  Daily = 365,
  OtherFrequency = 999
};

[[nodiscard]] constexpr double events_per_year(Frequency freq) noexcept {
  switch (freq) {
  case Frequency::Annual:
    return 1.0;
  case Frequency::SemiAnnual:
    return 2.0;
  case Frequency::EveryFourthMonth:
    return 3.0;
  case Frequency::Quarterly:
    return 4.0;
  case Frequency::Bimonthly:
    return 6.0;
  case Frequency::Monthly:
    return 12.0;
  case Frequency::EveryFourthWeek:
    return 13.0;
  case Frequency::Biweekly:
    return 26.0;
  case Frequency::Weekly:
    return 52.0;
  case Frequency::Daily:
    return 365.0;
  case Frequency::Once:
    return 0.0;
  default:
    return 0.0;
  }
}

[[nodiscard]] constexpr double year_fraction(Frequency freq) noexcept {
  switch (freq) {
  case Frequency::Annual:
    return 1.0;
  case Frequency::SemiAnnual:
    return 0.5;
  case Frequency::EveryFourthMonth:
    return 1.0 / 3.0;
  case Frequency::Quarterly:
    return 0.25;
  case Frequency::Bimonthly:
    return 1.0 / 6.0;
  case Frequency::Monthly:
    return 1.0 / 12.0;
  case Frequency::EveryFourthWeek:
    return 4.0 / 52.0;
  case Frequency::Biweekly:
    return 2.0 / 52.0;
  case Frequency::Weekly:
    return 1.0 / 52.0;
  case Frequency::Daily:
    return 1.0 / 365.0;
  case Frequency::Once:
    return 1.0;
  default:
    return 0.0;
  }
}

[[nodiscard]] std::string to_string(Frequency freq);
[[nodiscard]] std::optional<Frequency> parse_frequency(std::string_view text);

std::ostream &operator<<(std::ostream &os, Frequency freq);

} // namespace alm::time

template <>
struct std::formatter<alm::time::Frequency> : std::formatter<std::string> {
  auto format(alm::time::Frequency f, std::format_context &ctx) const {
    return std::formatter<std::string>::format(alm::time::to_string(f), ctx);
  }
};
