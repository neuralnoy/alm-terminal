#include "time/business_day.h"
#include "time/calendar.h"

#include <cctype>
#include <iostream>

namespace alm::time {

std::string to_string(BusinessDayConvention conv) {
  switch (conv) {
  case BusinessDayConvention::Following:
    return "Following";
  case BusinessDayConvention::ModifiedFollowing:
    return "ModifiedFollowing";
  case BusinessDayConvention::Preceding:
    return "Preceding";
  case BusinessDayConvention::ModifiedPreceding:
    return "ModifiedPreceding";
  case BusinessDayConvention::Unadjusted:
    return "Unadjusted";
  case BusinessDayConvention::HalfMonthModifiedFollowing:
    return "HalfMonthModifiedFollowing";
  case BusinessDayConvention::Nearest:
    return "Nearest";
  }
  return "Unknown";
}

std::optional<BusinessDayConvention>
parse_business_day_convention(std::string_view text) {
  std::string clean;
  clean.reserve(text.size());
  for (char c : text) {
    if (c != '-' && c != '_' && c != ' ') {
      clean.push_back(
          static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
  }

  if (clean == "FOLLOWING" || clean == "F") {
    return BusinessDayConvention::Following;
  }
  if (clean == "MODIFIEDFOLLOWING" || clean == "MODFOLLOWING" ||
      clean == "MF") {
    return BusinessDayConvention::ModifiedFollowing;
  }
  if (clean == "PRECEDING" || clean == "P") {
    return BusinessDayConvention::Preceding;
  }
  if (clean == "MODIFIEDPRECEDING" || clean == "MODPRECEDING" ||
      clean == "MP") {
    return BusinessDayConvention::ModifiedPreceding;
  }
  if (clean == "UNADJUSTED" || clean == "U") {
    return BusinessDayConvention::Unadjusted;
  }
  if (clean == "HALFMONTHMODIFIEDFOLLOWING" || clean == "HMMF") {
    return BusinessDayConvention::HalfMonthModifiedFollowing;
  }
  if (clean == "NEAREST" || clean == "N") {
    return BusinessDayConvention::Nearest;
  }

  return std::nullopt;
}

std::ostream &operator<<(std::ostream &os, BusinessDayConvention conv) {
  return os << to_string(conv);
}

Date adjust(Date date, BusinessDayConvention conv, const Calendar &calendar) {
  return calendar.adjust(date, conv);
}

} // namespace alm::time
