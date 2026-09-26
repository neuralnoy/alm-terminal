#include "time/calendar.h"

namespace alm::time {

Date nth_weekday(int n, std::chrono::weekday wd, unsigned month, int year) {
  Date first_day(year, month, 1);
  int first_iso = static_cast<int>(first_day.weekday().iso_encoding());
  int target_iso = static_cast<int>(wd.iso_encoding());
  int day_offset = (target_iso - first_iso + 7) % 7;
  int d = 1 + day_offset + (n - 1) * 7;
  return Date(year, month, static_cast<unsigned>(d));
}

Date last_weekday(std::chrono::weekday wd, unsigned month, int year) {
  Date last_day = Date(year, month, 1).end_of_month();
  int last_iso = static_cast<int>(last_day.weekday().iso_encoding());
  int target_iso = static_cast<int>(wd.iso_encoding());
  int day_offset = (last_iso - target_iso + 7) % 7;
  int d = static_cast<int>(last_day.day()) - day_offset;
  return Date(year, month, static_cast<unsigned>(d));
}

// -------------------------------------------------------------
// Calendar Base
// -------------------------------------------------------------

Calendar::Calendar(std::shared_ptr<const Impl> impl) : impl_{std::move(impl)} {}

bool Calendar::is_weekend(Date date) const {
  if (impl_)
    return impl_->is_weekend(date);
  return date.is_weekend();
}

bool Calendar::is_business_day(Date date) const {
  if (!added_holidays_.empty() && added_holidays_.contains(date))
    return false;
  if (!removed_holidays_.empty() && removed_holidays_.contains(date))
    return !is_weekend(date);
  if (!impl_)
    return !date.is_weekend();
  return impl_->is_business_day(date);
}

std::string_view Calendar::name() const {
  if (impl_)
    return impl_->name();
  return "NullCalendar";
}

bool Calendar::is_end_of_month(Date date) const {
  if (!is_business_day(date))
    return false;
  Date next = date + 1;
  while (!is_business_day(next)) {
    ++next;
  }
  return next.month() != date.month();
}

Date Calendar::end_of_month(Date date) const {
  Date eom = date.end_of_month();
  while (!is_business_day(eom)) {
    --eom;
  }
  return eom;
}

Date Calendar::adjust(Date date, BusinessDayConvention conv) const {
  if (conv == BusinessDayConvention::Unadjusted)
    return date;

  if (conv == BusinessDayConvention::Following) {
    Date d = date;
    while (!is_business_day(d)) {
      ++d;
    }
    return d;
  }

  if (conv == BusinessDayConvention::ModifiedFollowing) {
    Date d1 = date;
    while (!is_business_day(d1)) {
      ++d1;
    }
    if (d1.month() != date.month()) {
      Date d2 = date;
      while (!is_business_day(d2)) {
        --d2;
      }
      return d2;
    }
    return d1;
  }

  if (conv == BusinessDayConvention::Preceding) {
    Date d = date;
    while (!is_business_day(d)) {
      --d;
    }
    return d;
  }

  if (conv == BusinessDayConvention::ModifiedPreceding) {
    Date d1 = date;
    while (!is_business_day(d1)) {
      --d1;
    }
    if (d1.month() != date.month()) {
      Date d2 = date;
      while (!is_business_day(d2)) {
        ++d2;
      }
      return d2;
    }
    return d1;
  }

  if (conv == BusinessDayConvention::HalfMonthModifiedFollowing) {
    Date d1 = date;
    while (!is_business_day(d1)) {
      ++d1;
    }
    if (d1.month() != date.month() || (date.day() <= 15 && d1.day() > 15)) {
      Date d2 = date;
      while (!is_business_day(d2)) {
        --d2;
      }
      return d2;
    }
    return d1;
  }

  if (conv == BusinessDayConvention::Nearest) {
    if (is_business_day(date))
      return date;
    Date d_fwd = date;
    while (!is_business_day(d_fwd))
      ++d_fwd;
    Date d_back = date;
    while (!is_business_day(d_back))
      --d_back;

    int64_t fwd_diff = d_fwd - date;
    int64_t back_diff = date - d_back;
    if (back_diff < fwd_diff)
      return d_back;
    return d_fwd;
  }

  return date;
}

Date Calendar::advance(Date date, int n, TimeUnit unit,
                       BusinessDayConvention conv, bool end_of_month) const {
  if (unit == TimeUnit::Days) {
    if (n > 0) {
      Date d = date;
      while (n > 0) {
        ++d;
        if (is_business_day(d))
          --n;
      }
      return d;
    } else if (n < 0) {
      Date d = date;
      while (n < 0) {
        --d;
        if (is_business_day(d))
          ++n;
      }
      return d;
    }
    return adjust(date, conv);
  }

  Date target = alm::time::advance(date, Period(n, unit), end_of_month);
  return adjust(target, conv);
}

Date Calendar::advance(Date date, const Period &period,
                       BusinessDayConvention conv, bool end_of_month) const {
  return advance(date, period.length(), period.units(), conv, end_of_month);
}

int64_t Calendar::business_days_between(Date from, Date to, bool include_first,
                                        bool include_last) const {
  if (from == to) {
    if (include_first && include_last && is_business_day(from))
      return 1;
    return 0;
  }
  if (from > to) {
    return -business_days_between(to, from, include_last, include_first);
  }

  int64_t count = 0;
  Date cur = from;
  if (include_first && is_business_day(cur))
    ++count;
  ++cur;
  while (cur < to) {
    if (is_business_day(cur))
      ++count;
    ++cur;
  }
  if (include_last && is_business_day(to))
    ++count;
  return count;
}

std::vector<Date> Calendar::holiday_list(Date from, Date to) const {
  std::vector<Date> res;
  if (from > to)
    return res;
  Date cur = from;
  while (cur <= to) {
    if (is_holiday(cur)) {
      res.push_back(cur);
    }
    ++cur;
  }
  return res;
}

void Calendar::add_holiday(Date date) {
  added_holidays_.insert(date);
  removed_holidays_.erase(date);
}

void Calendar::remove_holiday(Date date) {
  removed_holidays_.insert(date);
  added_holidays_.erase(date);
}

bool Calendar::operator==(const Calendar &other) const noexcept {
  if (name() != other.name())
    return false;
  return added_holidays_ == other.added_holidays_ &&
         removed_holidays_ == other.removed_holidays_;
}

// -------------------------------------------------------------
// Concrete Implementations
// -------------------------------------------------------------

namespace {

class NullCalendarImpl : public Calendar::Impl {
public:
  std::string_view name() const override { return "NullCalendar"; }
  bool is_business_day(Date) const override { return true; }
  bool is_weekend(Date) const override { return false; }
};

class WeekendsOnlyImpl : public Calendar::Impl {
public:
  std::string_view name() const override { return "WeekendsOnly"; }
  bool is_business_day(Date date) const override { return !date.is_weekend(); }
};

class TargetImpl : public Calendar::Impl {
public:
  std::string_view name() const override { return "TARGET"; }

  bool is_business_day(Date date) const override {
    if (date.is_weekend())
      return false;

    int y = date.year();
    unsigned m = date.month();
    unsigned d = date.day();

    // New Year's Day
    if (m == 1 && d == 1)
      return false;
    // Labour Day
    if (m == 5 && d == 1)
      return false;
    // Christmas Day
    if (m == 12 && d == 25)
      return false;
    // Boxing Day
    if (m == 12 && d == 26)
      return false;

    Date easter = easter_sunday(y);
    // Good Friday
    if (date == easter - 2)
      return false;
    // Easter Monday
    if (date == easter + 1)
      return false;

    return true;
  }
};

class UnitedStatesImpl : public Calendar::Impl {
public:
  explicit UnitedStatesImpl(UnitedStates::Market market) : market_{market} {}

  std::string_view name() const override {
    switch (market_) {
    case UnitedStates::Market::Settlement:
      return "US Settlement";
    case UnitedStates::Market::GovernmentBond:
      return "US Government Bond";
    case UnitedStates::Market::NYSE:
      return "NYSE";
    case UnitedStates::Market::SOFR:
      return "US SOFR";
    }
    return "UnitedStates";
  }

  bool is_business_day(Date date) const override {
    if (date.is_weekend())
      return false;

    int y = date.year();
    unsigned m = date.month();
    unsigned d = date.day();
    auto wd = date.weekday();

    // New Year's Day (observed)
    if ((m == 1 && d == 1) || (m == 1 && d == 2 && wd == std::chrono::Monday) ||
        (m == 12 && d == 31 && wd == std::chrono::Friday &&
         market_ != UnitedStates::Market::Settlement)) {
      return false;
    }

    // Martin Luther King Jr. Day (3rd Mon of Jan, from 1983)
    if (y >= 1983 && date == nth_weekday(3, std::chrono::Monday, 1, y)) {
      return false;
    }

    // Presidents' Day (3rd Mon of Feb)
    if (date == nth_weekday(3, std::chrono::Monday, 2, y)) {
      return false;
    }

    // Good Friday (NYSE, Government Bond, SOFR)
    if (market_ != UnitedStates::Market::Settlement) {
      Date easter = easter_sunday(y);
      if (date == easter - 2)
        return false;
    }

    // Memorial Day (last Mon of May)
    if (date == last_weekday(std::chrono::Monday, 5, y)) {
      return false;
    }

    // Juneteenth (from 2021)
    if (y >= 2021) {
      if ((m == 6 && d == 19) ||
          (m == 6 && d == 20 && wd == std::chrono::Monday) ||
          (m == 6 && d == 18 && wd == std::chrono::Friday)) {
        return false;
      }
    }

    // Independence Day (Jul 4, observed)
    if ((m == 7 && d == 4) || (m == 7 && d == 5 && wd == std::chrono::Monday) ||
        (m == 7 && d == 3 && wd == std::chrono::Friday)) {
      return false;
    }

    // Labor Day (1st Mon of Sep)
    if (date == nth_weekday(1, std::chrono::Monday, 9, y)) {
      return false;
    }

    // Columbus Day (2nd Mon of Oct - Gov Bond, Settlement, SOFR)
    if (market_ != UnitedStates::Market::NYSE) {
      if (date == nth_weekday(2, std::chrono::Monday, 10, y)) {
        return false;
      }
    }

    // Veterans Day (Nov 11 observed - Gov Bond, Settlement, SOFR)
    if (market_ != UnitedStates::Market::NYSE) {
      if ((m == 11 && d == 11) ||
          (m == 11 && d == 12 && wd == std::chrono::Monday) ||
          (m == 11 && d == 10 && wd == std::chrono::Friday)) {
        return false;
      }
    }

    // Thanksgiving Day (4th Thu of Nov)
    if (date == nth_weekday(4, std::chrono::Thursday, 11, y)) {
      return false;
    }

    // Christmas Day (Dec 25 observed)
    if ((m == 12 && d == 25) ||
        (m == 12 && d == 26 && wd == std::chrono::Monday) ||
        (m == 12 && d == 24 && wd == std::chrono::Friday)) {
      return false;
    }

    return true;
  }

private:
  UnitedStates::Market market_;
};

class UnitedKingdomImpl : public Calendar::Impl {
public:
  explicit UnitedKingdomImpl(UnitedKingdom::Market market) : market_{market} {}

  std::string_view name() const override {
    switch (market_) {
    case UnitedKingdom::Market::Settlement:
      return "UK Settlement";
    case UnitedKingdom::Market::Exchange:
      return "LSE";
    case UnitedKingdom::Market::Metals:
      return "LME";
    }
    return "UnitedKingdom";
  }

  bool is_business_day(Date date) const override {
    if (date.is_weekend())
      return false;

    int y = date.year();
    unsigned m = date.month();
    unsigned d = date.day();
    auto wd = date.weekday();

    // New Year's Day (observed)
    if ((m == 1 && d == 1) ||
        (m == 1 && (d == 2 || d == 3) && wd == std::chrono::Monday)) {
      return false;
    }

    Date easter = easter_sunday(y);
    // Good Friday
    if (date == easter - 2)
      return false;
    // Easter Monday
    if (date == easter + 1)
      return false;

    // Early May Bank Holiday (1st Mon of May)
    if (date == nth_weekday(1, std::chrono::Monday, 5, y))
      return false;

    // Spring Bank Holiday (last Mon of May)
    if (date == last_weekday(std::chrono::Monday, 5, y))
      return false;

    // Summer Bank Holiday (last Mon of Aug)
    if (date == last_weekday(std::chrono::Monday, 8, y))
      return false;

    // Christmas Day (observed)
    if ((m == 12 && d == 25) ||
        (m == 12 && (d == 27) &&
         (wd == std::chrono::Monday || wd == std::chrono::Tuesday))) {
      return false;
    }

    // Boxing Day (observed)
    if ((m == 12 && d == 26) ||
        (m == 12 && (d == 28) &&
         (wd == std::chrono::Monday || wd == std::chrono::Tuesday))) {
      return false;
    }

    return true;
  }

private:
  UnitedKingdom::Market market_;
};

class SwitzerlandImpl : public Calendar::Impl {
public:
  explicit SwitzerlandImpl(Switzerland::Market market) : market_{market} {}

  std::string_view name() const override { return "Switzerland"; }

  bool is_business_day(Date date) const override {
    if (date.is_weekend())
      return false;

    int y = date.year();
    unsigned m = date.month();
    unsigned d = date.day();

    // New Year's Day & Berchtoldstag
    if (m == 1 && (d == 1 || d == 2))
      return false;

    Date easter = easter_sunday(y);
    // Good Friday
    if (date == easter - 2)
      return false;
    // Easter Monday
    if (date == easter + 1)
      return false;
    // Ascension Day (Easter + 39)
    if (date == easter + 39)
      return false;
    // Whit Monday (Easter + 50)
    if (date == easter + 50)
      return false;

    // Swiss National Day
    if (m == 8 && d == 1)
      return false;

    // Christmas & St. Stephen
    if (m == 12 && (d == 25 || d == 26))
      return false;

    return true;
  }

private:
  Switzerland::Market market_;
};

class JapanImpl : public Calendar::Impl {
public:
  std::string_view name() const override { return "Japan"; }

  bool is_business_day(Date date) const override {
    if (date.is_weekend())
      return false;

    int y = date.year();
    unsigned m = date.month();
    unsigned d = date.day();
    auto wd = date.weekday();

    // New Year Bank Holidays (Jan 1, 2, 3)
    if (m == 1 && (d >= 1 && d <= 3))
      return false;

    // Coming of Age Day (2nd Mon of Jan)
    if (date == nth_weekday(2, std::chrono::Monday, 1, y))
      return false;

    // National Foundation Day (Feb 11, observed)
    if ((m == 2 && d == 11) || (m == 2 && d == 12 && wd == std::chrono::Monday))
      return false;

    // Emperor's Birthday (Feb 23, observed, from 2020)
    if (y >= 2020 && ((m == 2 && d == 23) ||
                      (m == 2 && d == 24 && wd == std::chrono::Monday)))
      return false;

    // Vernal Equinox (~Mar 20/21)
    int vernal_d = 20; // standard approximation
    if ((m == 3 && d == static_cast<unsigned>(vernal_d)) ||
        (m == 3 && d == static_cast<unsigned>(vernal_d + 1) &&
         wd == std::chrono::Monday)) {
      return false;
    }

    // Showa Day (Apr 29, observed)
    if ((m == 4 && d == 29) || (m == 4 && d == 30 && wd == std::chrono::Monday))
      return false;

    // Golden Week (May 3, 4, 5)
    if (m == 5 && (d >= 3 && d <= 5))
      return false;
    // If any of May 3, 4, 5 was Sunday, May 6 is observed holiday
    if (m == 5 && d == 6 &&
        (wd == std::chrono::Monday || wd == std::chrono::Tuesday ||
         wd == std::chrono::Wednesday)) {
      Date may3(y, 5, 3), may4(y, 5, 4), may5(y, 5, 5);
      if (may3.weekday() == std::chrono::Sunday ||
          may4.weekday() == std::chrono::Sunday ||
          may5.weekday() == std::chrono::Sunday) {
        return false;
      }
    }

    // Marine Day (3rd Mon of Jul)
    if (date == nth_weekday(3, std::chrono::Monday, 7, y))
      return false;

    // Mountain Day (Aug 11, observed)
    if ((m == 8 && d == 11) || (m == 8 && d == 12 && wd == std::chrono::Monday))
      return false;

    // Respect for the Aged Day (3rd Mon of Sep)
    if (date == nth_weekday(3, std::chrono::Monday, 9, y))
      return false;

    // Autumnal Equinox (~Sep 23, observed)
    int autumn_d = 23;
    if ((m == 9 && d == static_cast<unsigned>(autumn_d)) ||
        (m == 9 && d == static_cast<unsigned>(autumn_d + 1) &&
         wd == std::chrono::Monday)) {
      return false;
    }

    // Sports Day (2nd Mon of Oct)
    if (date == nth_weekday(2, std::chrono::Monday, 10, y))
      return false;

    // Culture Day (Nov 3, observed)
    if ((m == 11 && d == 3) || (m == 11 && d == 4 && wd == std::chrono::Monday))
      return false;

    // Labor Thanksgiving Day (Nov 23, observed)
    if ((m == 11 && d == 23) ||
        (m == 11 && d == 24 && wd == std::chrono::Monday))
      return false;

    // Year-end bank holiday (Dec 31)
    if (m == 12 && d == 31)
      return false;

    return true;
  }
};

class JointCalendarImpl : public Calendar::Impl {
public:
  JointCalendarImpl(std::vector<Calendar> calendars, JointCalendar::Rule rule)
      : calendars_{std::move(calendars)}, rule_{rule} {}

  std::string_view name() const override { return "JointCalendar"; }

  bool is_business_day(Date date) const override {
    if (calendars_.empty())
      return !date.is_weekend();

    if (rule_ == JointCalendar::Rule::JoinHolidays) {
      for (const auto &cal : calendars_) {
        if (!cal.is_business_day(date))
          return false;
      }
      return true;
    } else {
      for (const auto &cal : calendars_) {
        if (cal.is_business_day(date))
          return true;
      }
      return false;
    }
  }

  bool is_weekend(Date date) const override {
    for (const auto &cal : calendars_) {
      if (cal.is_weekend(date))
        return true;
    }
    return false;
  }

private:
  std::vector<Calendar> calendars_;
  JointCalendar::Rule rule_;
};

} // namespace

NullCalendar::NullCalendar() {
  static const auto inst = std::make_shared<NullCalendarImpl>();
  impl_ = inst;
}

WeekendsOnly::WeekendsOnly() {
  static const auto inst = std::make_shared<WeekendsOnlyImpl>();
  impl_ = inst;
}

Target::Target() {
  static const auto inst = std::make_shared<TargetImpl>();
  impl_ = inst;
}

UnitedStates::UnitedStates(Market market) {
  static const std::shared_ptr<const UnitedStatesImpl> instances[] = {
      std::make_shared<UnitedStatesImpl>(Market::Settlement),
      std::make_shared<UnitedStatesImpl>(Market::GovernmentBond),
      std::make_shared<UnitedStatesImpl>(Market::NYSE),
      std::make_shared<UnitedStatesImpl>(Market::SOFR)};
  impl_ = instances[static_cast<size_t>(market)];
}

UnitedKingdom::UnitedKingdom(Market market) {
  static const std::shared_ptr<const UnitedKingdomImpl> instances[] = {
      std::make_shared<UnitedKingdomImpl>(Market::Settlement),
      std::make_shared<UnitedKingdomImpl>(Market::Exchange),
      std::make_shared<UnitedKingdomImpl>(Market::Metals)};
  impl_ = instances[static_cast<size_t>(market)];
}

Switzerland::Switzerland(Market market) {
  static const std::shared_ptr<const SwitzerlandImpl> instances[] = {
      std::make_shared<SwitzerlandImpl>(Market::Settlement),
      std::make_shared<SwitzerlandImpl>(Market::SIX)};
  impl_ = instances[static_cast<size_t>(market)];
}

Japan::Japan() {
  static const auto inst = std::make_shared<JapanImpl>();
  impl_ = inst;
}

JointCalendar::JointCalendar(const Calendar &c1, const Calendar &c2, Rule rule)
    : Calendar(std::make_shared<JointCalendarImpl>(
          std::vector<Calendar>{c1, c2}, rule)) {}

JointCalendar::JointCalendar(const std::vector<Calendar> &calendars, Rule rule)
    : Calendar(std::make_shared<JointCalendarImpl>(calendars, rule)) {}

} // namespace alm::time
