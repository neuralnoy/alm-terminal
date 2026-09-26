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
  std::string clean;
  clean.reserve(text.size());
  for (char c : text) {
    if (c != '-' && c != '_' && c != ' ') {
      clean.push_back(
          static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
  }

  if (clean == "1Y" || clean == "12M" || clean == "ANNUAL" ||
      clean == "ANNUALLY" || clean == "1") {
    return Frequency::Annual;
  }
  if (clean == "6M" || clean == "SEMIANNUAL" || clean == "SEMIANNUALLY" ||
      clean == "SA" || clean == "2") {
    return Frequency::SemiAnnual;
  }
  if (clean == "4M" || clean == "EVERYFOURTHMONTH" || clean == "3") {
    return Frequency::EveryFourthMonth;
  }
  if (clean == "3M" || clean == "QUARTERLY" || clean == "Q" || clean == "4") {
    return Frequency::Quarterly;
  }
  if (clean == "2M" || clean == "BIMONTHLY" || clean == "6") {
    return Frequency::Bimonthly;
  }
  if (clean == "1M" || clean == "MONTHLY" || clean == "M" || clean == "12") {
    return Frequency::Monthly;
  }
  if (clean == "4W" || clean == "EVERYFOURTHWEEK" || clean == "13") {
    return Frequency::EveryFourthWeek;
  }
  if (clean == "2W" || clean == "BIWEEKLY" || clean == "BW" || clean == "26") {
    return Frequency::Biweekly;
  }
  if (clean == "1W" || clean == "WEEKLY" || clean == "W" || clean == "52") {
    return Frequency::Weekly;
  }
  if (clean == "1D" || clean == "DAILY" || clean == "D" || clean == "365") {
    return Frequency::Daily;
  }
  if (clean == "ONCE" || clean == "ZERO" || clean == "0") {
    return Frequency::Once;
  }
  if (clean == "NONE" || clean == "NOFREQUENCY" || clean == "-1") {
    return Frequency::NoFrequency;
  }

  return std::nullopt;
}

std::ostream &operator<<(std::ostream &os, Frequency freq) {
  return os << to_string(freq);
}

} // namespace alm::time
