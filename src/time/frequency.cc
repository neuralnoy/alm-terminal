#include "time/frequency.h"

#include <cctype>
#include <iostream>

namespace alm::time {

std::string to_string(Frequency freq) {
  switch (freq) {
  case Frequency::NoFrequency:
    return "NoFrequency";
  case Frequency::Once:
    return "Once";
  case Frequency::Annual:
    return "Annual";
  case Frequency::SemiAnnual:
    return "SemiAnnual";
  case Frequency::EveryFourthMonth:
    return "EveryFourthMonth";
  case Frequency::Quarterly:
    return "Quarterly";
  case Frequency::Bimonthly:
    return "Bimonthly";
  case Frequency::Monthly:
    return "Monthly";
  case Frequency::EveryFourthWeek:
    return "EveryFourthWeek";
  case Frequency::Biweekly:
    return "Biweekly";
  case Frequency::Weekly:
    return "Weekly";
  case Frequency::Daily:
    return "Daily";
  case Frequency::OtherFrequency:
    return "OtherFrequency";
  }
  return "Unknown";
}

std::optional<Frequency> parse_frequency(std::string_view text) {
  char clean[64];
  size_t len = 0;
  for (char c : text) {
    if (c != '-' && c != '_' && c != ' ') {
      if (len < sizeof(clean)) {
        clean[len++] =
            static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
      }
    }
  }
  std::string_view clean_sv(clean, len);

  auto is_any_of = [&](std::initializer_list<std::string_view> targets) {
    for (auto t : targets) {
      if (clean_sv == t)
        return true;
    }
    return false;
  };

  if (is_any_of({"1Y", "12M", "ANNUAL", "ANNUALLY", "1"})) {
    return Frequency::Annual;
  }
  if (is_any_of({"6M", "SEMIANNUAL", "SEMIANNUALLY", "SA", "2"})) {
    return Frequency::SemiAnnual;
  }
  if (is_any_of({"4M", "EVERYFOURTHMONTH", "3"})) {
    return Frequency::EveryFourthMonth;
  }
  if (is_any_of({"3M", "QUARTERLY", "Q", "4"})) {
    return Frequency::Quarterly;
  }
  if (is_any_of({"2M", "BIMONTHLY", "6"})) {
    return Frequency::Bimonthly;
  }
  if (is_any_of({"1M", "MONTHLY", "M", "12"})) {
    return Frequency::Monthly;
  }
  if (is_any_of({"4W", "EVERYFOURTHWEEK", "13"})) {
    return Frequency::EveryFourthWeek;
  }
  if (is_any_of({"2W", "BIWEEKLY", "BW", "26"})) {
    return Frequency::Biweekly;
  }
  if (is_any_of({"1W", "WEEKLY", "W", "52"})) {
    return Frequency::Weekly;
  }
  if (is_any_of({"1D", "DAILY", "D", "365"})) {
    return Frequency::Daily;
  }
  if (is_any_of({"ONCE", "ZERO", "0"})) {
    return Frequency::Once;
  }
  if (is_any_of({"NONE", "NOFREQUENCY", "-1"})) {
    return Frequency::NoFrequency;
  }

  return std::nullopt;
}

std::ostream &operator<<(std::ostream &os, Frequency freq) {
  return os << to_string(freq);
}

} // namespace alm::time
