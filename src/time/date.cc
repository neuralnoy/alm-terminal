#include "time/date.h"

#include <charconv>
#include <iostream>

namespace alm::time {

Date Date::today() noexcept {
  const auto now = std::chrono::system_clock::now();
  return Date{std::chrono::floor<std::chrono::days>(now)};
}

std::string Date::to_string() const {
  const auto ymd = to_year_month_day();
  return std::format("{:04d}-{:02d}-{:02d}", static_cast<int>(ymd.year()),
                     static_cast<unsigned>(ymd.month()),
                     static_cast<unsigned>(ymd.day()));
}

std::optional<Date> Date::parse(std::string_view text) {
  if (text.size() == 10 && text[4] == '-' && text[7] == '-') {
    int y = 0;
    unsigned m = 0;
    unsigned d = 0;
    const auto res_y = std::from_chars(text.data(), text.data() + 4, y);
    const auto res_m = std::from_chars(text.data() + 5, text.data() + 7, m);
    const auto res_d = std::from_chars(text.data() + 8, text.data() + 10, d);
    if (res_y.ec == std::errc{} && res_m.ec == std::errc{} &&
        res_d.ec == std::errc{}) {
      if (is_valid_date(y, m, d)) {
        return Date(y, m, d);
      }
    }
  } else if (text.size() == 8) {
    int y = 0;
    unsigned m = 0;
    unsigned d = 0;
    const auto res_y = std::from_chars(text.data(), text.data() + 4, y);
    const auto res_m = std::from_chars(text.data() + 4, text.data() + 6, m);
    const auto res_d = std::from_chars(text.data() + 6, text.data() + 8, d);
    if (res_y.ec == std::errc{} && res_m.ec == std::errc{} &&
        res_d.ec == std::errc{}) {
      if (is_valid_date(y, m, d)) {
        return Date(y, m, d);
      }
    }
  }
  return std::nullopt;
}

std::ostream &operator<<(std::ostream &os, const Date &date) {
  return os << date.to_string();
}

} // namespace alm::time
