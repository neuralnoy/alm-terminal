#include "time/day_count.h"

#include <cctype>
#include <iostream>

namespace alm::time {

int64_t DayCounter::day_count(Date start, Date end) const noexcept {
  switch (conv_) {
  case DayCountConvention::Actual360:
  case DayCountConvention::Actual365Fixed:
  case DayCountConvention::ActualActualISDA:
  case DayCountConvention::ActualActualICMA:
    return end - start;

  case DayCountConvention::Actual365NoLeap: {
    if (start == end)
      return 0;
    if (start > end)
      return -day_count(end, start);
    int64_t raw_days = end - start;
    int leap_days = 0;
    Date cur = start;
    while (cur < end) {
      if (cur.month() == 2 && cur.day() == 29) {
        ++leap_days;
      }
      ++cur;
    }
    return raw_days - leap_days;
  }

  case DayCountConvention::Thirty360US: {
    int y1 = start.year(), m1 = static_cast<int>(start.month()),
        d1 = static_cast<int>(start.day());
    int y2 = end.year(), m2 = static_cast<int>(end.month()),
        d2 = static_cast<int>(end.day());

    if (d1 == 31)
      d1 = 30;
    if (d2 == 31 && d1 >= 30)
      d2 = 30;

    return 360LL * (y2 - y1) + 30LL * (m2 - m1) + (d2 - d1);
  }

  case DayCountConvention::ThirtyE360: {
    int y1 = start.year(), m1 = static_cast<int>(start.month()),
        d1 = static_cast<int>(start.day());
    int y2 = end.year(), m2 = static_cast<int>(end.month()),
        d2 = static_cast<int>(end.day());

    if (d1 == 31)
      d1 = 30;
    if (d2 == 31)
      d2 = 30;

    return 360LL * (y2 - y1) + 30LL * (m2 - m1) + (d2 - d1);
  }

  case DayCountConvention::ThirtyE360ISDA: {
    int y1 = start.year(), m1 = static_cast<int>(start.month()),
        d1 = static_cast<int>(start.day());
    int y2 = end.year(), m2 = static_cast<int>(end.month()),
        d2 = static_cast<int>(end.day());

    if (start.is_end_of_month())
      d1 = 30;
    if (end.is_end_of_month() && !(m2 == 2 && end == end.end_of_month()))
      d2 = 30;

    return 360LL * (y2 - y1) + 30LL * (m2 - m1) + (d2 - d1);
  }

  case DayCountConvention::OneOne:
    return end - start;
  }
  return end - start;
}

double DayCounter::year_fraction(Date start, Date end, Date ref_start,
                                 Date ref_end) const noexcept {
  if (start == end)
    return 0.0;
  if (start > end)
    return -year_fraction(end, start, ref_end, ref_start);

  switch (conv_) {
  case DayCountConvention::Actual360:
    return static_cast<double>(end - start) / 360.0;

  case DayCountConvention::Actual365Fixed:
    return static_cast<double>(end - start) / 365.0;

  case DayCountConvention::Actual365NoLeap:
    return static_cast<double>(day_count(start, end)) / 365.0;

  case DayCountConvention::ActualActualISDA: {
    int y1 = start.year();
    int y2 = end.year();
    if (y1 == y2) {
      double dib = Date::is_leap_year(y1) ? 366.0 : 365.0;
      return static_cast<double>(end - start) / dib;
    }

    double sum = 0.0;
    Date start_next_y(y1 + 1, 1, 1);
    double dib1 = Date::is_leap_year(y1) ? 366.0 : 365.0;
    sum += static_cast<double>(start_next_y - start) / dib1;

    for (int y = y1 + 1; y < y2; ++y) {
      sum += 1.0;
    }

    Date start_y2(y2, 1, 1);
    double dib2 = Date::is_leap_year(y2) ? 366.0 : 365.0;
    sum += static_cast<double>(end - start_y2) / dib2;
    return sum;
  }

  case DayCountConvention::ActualActualICMA: {
    Date r_start = ref_start.is_valid() ? ref_start : start;
    Date r_end = ref_end.is_valid() ? ref_end : end;
    int64_t ref_days = r_end - r_start;
    if (ref_days <= 0)
      ref_days = end - start;

    double f = events_per_year(freq_);
    if (f <= 0.0)
      f = 1.0;

    return static_cast<double>(end - start) /
           (f * static_cast<double>(ref_days));
  }

  case DayCountConvention::Thirty360US:
  case DayCountConvention::ThirtyE360:
  case DayCountConvention::ThirtyE360ISDA:
    return static_cast<double>(day_count(start, end)) / 360.0;

  case DayCountConvention::OneOne:
    return 1.0;
  }
  return static_cast<double>(end - start) / 365.0;
}

std::string_view DayCounter::name() const noexcept {
  switch (conv_) {
  case DayCountConvention::Actual360:
    return "Actual/360";
  case DayCountConvention::Actual365Fixed:
    return "Actual/365 Fixed";
  case DayCountConvention::Actual365NoLeap:
    return "Actual/365 No Leap";
  case DayCountConvention::ActualActualISDA:
    return "Actual/Actual ISDA";
  case DayCountConvention::ActualActualICMA:
    return "Actual/Actual ICMA";
  case DayCountConvention::Thirty360US:
    return "30/360 US";
  case DayCountConvention::ThirtyE360:
    return "30E/360";
  case DayCountConvention::ThirtyE360ISDA:
    return "30E/360 ISDA";
  case DayCountConvention::OneOne:
    return "1/1";
  }
  return "Unknown";
}

std::string to_string(DayCountConvention conv) {
  switch (conv) {
  case DayCountConvention::Actual360:
    return "Actual360";
  case DayCountConvention::Actual365Fixed:
    return "Actual365Fixed";
  case DayCountConvention::Actual365NoLeap:
    return "Actual365NoLeap";
  case DayCountConvention::ActualActualISDA:
    return "ActualActualISDA";
  case DayCountConvention::ActualActualICMA:
    return "ActualActualICMA";
  case DayCountConvention::Thirty360US:
    return "Thirty360US";
  case DayCountConvention::ThirtyE360:
    return "ThirtyE360";
  case DayCountConvention::ThirtyE360ISDA:
    return "ThirtyE360ISDA";
  case DayCountConvention::OneOne:
    return "OneOne";
  }
  return "Unknown";
}

std::optional<DayCountConvention>
parse_day_count_convention(std::string_view text) {
  std::string clean;
  clean.reserve(text.size());
  for (char c : text) {
    if (c != '/' && c != '-' && c != '_' && c != ' ') {
      clean.push_back(
          static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
  }

  if (clean == "ACT360" || clean == "ACTUAL360" || clean == "A360") {
    return DayCountConvention::Actual360;
  }
  if (clean == "ACT365" || clean == "ACTUAL365" || clean == "ACTUAL365FIXED" ||
      clean == "A365F" || clean == "A365") {
    return DayCountConvention::Actual365Fixed;
  }
  if (clean == "ACT365NL" || clean == "ACTUAL365NOLEAP") {
    return DayCountConvention::Actual365NoLeap;
  }
  if (clean == "ACTACT" || clean == "ACTUALACTUAL" || clean == "ACTACTISDA" ||
      clean == "ACTUALACTUALISDA") {
    return DayCountConvention::ActualActualISDA;
  }
  if (clean == "ACTACTICMA" || clean == "ACTUALACTUALICMA" ||
      clean == "ACTACTBOND") {
    return DayCountConvention::ActualActualICMA;
  }
  if (clean == "30360" || clean == "30360US" || clean == "THIRTY360US" ||
      clean == "BONDBASIS") {
    return DayCountConvention::Thirty360US;
  }
  if (clean == "30E360" || clean == "30360E" || clean == "THIRTYE360" ||
      clean == "EUROBONDBASIS" || clean == "ISMA30360") {
    return DayCountConvention::ThirtyE360;
  }
  if (clean == "30E360ISDA" || clean == "THIRTYE360ISDA") {
    return DayCountConvention::ThirtyE360ISDA;
  }
  if (clean == "11" || clean == "ONEONE") {
    return DayCountConvention::OneOne;
  }

  return std::nullopt;
}

std::optional<DayCounter> DayCounter::parse(std::string_view text) {
  auto conv = parse_day_count_convention(text);
  if (conv) {
    return DayCounter(*conv);
  }
  return std::nullopt;
}

std::ostream &operator<<(std::ostream &os, DayCountConvention conv) {
  return os << to_string(conv);
}

std::ostream &operator<<(std::ostream &os, const DayCounter &dc) {
  return os << dc.name();
}

} // namespace alm::time
