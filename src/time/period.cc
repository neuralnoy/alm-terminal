#include "time/period.h"

#include <cctype>
#include <charconv>
#include <iostream>
#include <stdexcept>

namespace alm::time {

Period Period::from_frequency(Frequency freq) {
  switch (freq) {
  case Frequency::Annual:
    return Period::years(1);
  case Frequency::SemiAnnual:
    return Period::months(6);
  case Frequency::EveryFourthMonth:
    return Period::months(4);
  case Frequency::Quarterly:
    return Period::months(3);
  case Frequency::Bimonthly:
    return Period::months(2);
  case Frequency::Monthly:
    return Period::months(1);
  case Frequency::EveryFourthWeek:
    return Period::weeks(4);
  case Frequency::Biweekly:
    return Period::weeks(2);
  case Frequency::Weekly:
    return Period::weeks(1);
  case Frequency::Daily:
    return Period::days(1);
  case Frequency::Once:
    return Period::days(0);
  case Frequency::NoFrequency:
  case Frequency::OtherFrequency:
    return Period::days(0);
  }
  return Period::days(0);
}

std::optional<Frequency> Period::frequency() const noexcept {
  Period norm = normalize();
  if (norm.units_ == TimeUnit::Years && norm.length_ == 1)
    return Frequency::Annual;
  if (norm.units_ == TimeUnit::Months) {
    if (norm.length_ == 6)
      return Frequency::SemiAnnual;
    if (norm.length_ == 4)
      return Frequency::EveryFourthMonth;
    if (norm.length_ == 3)
      return Frequency::Quarterly;
    if (norm.length_ == 2)
      return Frequency::Bimonthly;
    if (norm.length_ == 1)
      return Frequency::Monthly;
    if (norm.length_ == 12)
      return Frequency::Annual;
  }
  if (norm.units_ == TimeUnit::Weeks) {
    if (norm.length_ == 4)
      return Frequency::EveryFourthWeek;
    if (norm.length_ == 2)
      return Frequency::Biweekly;
    if (norm.length_ == 1)
      return Frequency::Weekly;
  }
  if (norm.units_ == TimeUnit::Days) {
    if (norm.length_ == 1)
      return Frequency::Daily;
    if (norm.length_ == 0)
      return Frequency::Once;
  }
  return std::nullopt;
}

Period Period::normalize() const noexcept {
  if (length_ == 0) {
    return Period(0, TimeUnit::Days);
  }
  if (units_ == TimeUnit::Months && length_ % 12 == 0) {
    return Period(length_ / 12, TimeUnit::Years);
  }
  if (units_ == TimeUnit::Days && length_ % 7 == 0) {
    return Period(length_ / 7, TimeUnit::Weeks);
  }
  return *this;
}

Period Period::operator+(const Period &other) const {
  if (length_ == 0)
    return other;
  if (other.length_ == 0)
    return *this;

  if (units_ == other.units_) {
    return Period(length_ + other.length_, units_).normalize();
  }

  // Months and Years are compatible
  if ((units_ == TimeUnit::Months || units_ == TimeUnit::Years) &&
      (other.units_ == TimeUnit::Months || other.units_ == TimeUnit::Years)) {
    int m1 = (units_ == TimeUnit::Years) ? length_ * 12 : length_;
    int m2 =
        (other.units_ == TimeUnit::Years) ? other.length_ * 12 : other.length_;
    return Period(m1 + m2, TimeUnit::Months).normalize();
  }

  // Days and Weeks are compatible
  if ((units_ == TimeUnit::Days || units_ == TimeUnit::Weeks) &&
      (other.units_ == TimeUnit::Days || other.units_ == TimeUnit::Weeks)) {
    int d1 = (units_ == TimeUnit::Weeks) ? length_ * 7 : length_;
    int d2 =
        (other.units_ == TimeUnit::Weeks) ? other.length_ * 7 : other.length_;
    return Period(d1 + d2, TimeUnit::Days).normalize();
  }

  throw std::invalid_argument(
      "Cannot add incompatible periods (e.g. Days and Months)");
}

Period Period::operator-(const Period &other) const { return *this + (-other); }

bool Period::operator==(const Period &other) const noexcept {
  if (length_ == 0 && other.length_ == 0)
    return true;
  if (units_ == other.units_)
    return length_ == other.length_;

  if ((units_ == TimeUnit::Months || units_ == TimeUnit::Years) &&
      (other.units_ == TimeUnit::Months || other.units_ == TimeUnit::Years)) {
    int m1 = (units_ == TimeUnit::Years) ? length_ * 12 : length_;
    int m2 =
        (other.units_ == TimeUnit::Years) ? other.length_ * 12 : other.length_;
    return m1 == m2;
  }

  if ((units_ == TimeUnit::Days || units_ == TimeUnit::Weeks) &&
      (other.units_ == TimeUnit::Days || other.units_ == TimeUnit::Weeks)) {
    int d1 = (units_ == TimeUnit::Weeks) ? length_ * 7 : length_;
    int d2 =
        (other.units_ == TimeUnit::Weeks) ? other.length_ * 7 : other.length_;
    return d1 == d2;
  }

  return false;
}

std::partial_ordering Period::operator<=>(const Period &other) const noexcept {
  if (*this == other)
    return std::partial_ordering::equivalent;

  // Approximate days comparison for partial ordering
  auto to_approx_days = [](const Period &p) -> double {
    switch (p.units_) {
    case TimeUnit::Days:
      return p.length_;
    case TimeUnit::Weeks:
      return p.length_ * 7.0;
    case TimeUnit::Months:
      return p.length_ * (365.25 / 12.0);
    case TimeUnit::Years:
      return p.length_ * 365.25;
    }
    return 0.0;
  };

  double d1 = to_approx_days(*this);
  double d2 = to_approx_days(other);
  if (d1 < d2)
    return std::partial_ordering::less;
  if (d1 > d2)
    return std::partial_ordering::greater;
  return std::partial_ordering::equivalent;
}

std::string Period::to_string() const {
  char unit_char = 'D';
  switch (units_) {
  case TimeUnit::Days:
    unit_char = 'D';
    break;
  case TimeUnit::Weeks:
    unit_char = 'W';
    break;
  case TimeUnit::Months:
    unit_char = 'M';
    break;
  case TimeUnit::Years:
    unit_char = 'Y';
    break;
  }
  return std::format("{}{}", length_, unit_char);
}

std::optional<Period> Period::parse(std::string_view text) {
  if (text.empty())
    return std::nullopt;

  // Check special market aliases
  if (text == "ON" || text == "on" || text == "O/N")
    return Period::days(1);
  if (text == "TN" || text == "tn" || text == "T/N")
    return Period::days(1);
  if (text == "SN" || text == "sn" || text == "S/N")
    return Period::days(1);

  // Parse [number][D|W|M|Y]
  int len = 0;
  auto res = std::from_chars(text.data(), text.data() + text.size() - 1, len);
  if (res.ec != std::errc{}) {
    // Might be single char unit like "D" without number => defaults to 1? Or
    // not
    return std::nullopt;
  }

  char u =
      static_cast<char>(std::toupper(static_cast<unsigned char>(text.back())));
  switch (u) {
  case 'D':
    return Period(len, TimeUnit::Days);
  case 'W':
    return Period(len, TimeUnit::Weeks);
  case 'M':
    return Period(len, TimeUnit::Months);
  case 'Y':
    return Period(len, TimeUnit::Years);
  default:
    return std::nullopt;
  }
}

std::ostream &operator<<(std::ostream &os, const Period &p) {
  return os << p.to_string();
}

} // namespace alm::time
