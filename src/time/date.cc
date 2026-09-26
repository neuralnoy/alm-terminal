#include "time/date.h"

namespace alm::time {

Date::Date(std::chrono::year y, std::chrono::month m, std::chrono::day d)
    : tp_{std::chrono::year_month_day{y, m, d}} {}

Date::Date(int year, unsigned month, unsigned day)
    : Date(std::chrono::year{year}, std::chrono::month{month},
           std::chrono::day{day}) {}

Date::Date(std::chrono::sys_days tp) : tp_{tp} {}

int Date::year() const {
  return static_cast<int>(std::chrono::year_month_day{tp_}.year());
}

unsigned Date::month() const {
  return static_cast<unsigned>(std::chrono::year_month_day{tp_}.month());
}

unsigned Date::day() const {
  return static_cast<unsigned>(std::chrono::year_month_day{tp_}.day());
}

} // namespace alm::time
