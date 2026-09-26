#include "time/conventions.h"

#include <cctype>

namespace alm::time {

MarketConvention::MarketConvention(
    std::string name, Calendar calendar, DayCounter day_counter,
    BusinessDayConvention business_day_convention, int settlement_days,
    Frequency payment_frequency, bool end_of_month)
    : name_{std::move(name)}, calendar_{std::move(calendar)},
      day_counter_{day_counter}, bdc_{business_day_convention},
      settlement_days_{settlement_days}, payment_freq_{payment_frequency},
      end_of_month_{end_of_month} {}

Date MarketConvention::settlement_date(Date trade_date) const {
  if (settlement_days_ <= 0) {
    return adjust(trade_date);
  }
  return calendar_.advance(trade_date, settlement_days_, TimeUnit::Days, bdc_);
}

Date MarketConvention::adjust(Date date) const {
  return calendar_.adjust(date, bdc_);
}

MarketConvention MarketConvention::sofr_ois() {
  return MarketConvention(
      "USD SOFR OIS", UnitedStates(UnitedStates::Market::GovernmentBond),
      DayCounter::actual_360(), BusinessDayConvention::ModifiedFollowing, 2,
      Frequency::Annual, false);
}

MarketConvention MarketConvention::estr_ois() {
  return MarketConvention("EUR ESTR OIS", Target(), DayCounter::actual_360(),
                          BusinessDayConvention::ModifiedFollowing, 2,
                          Frequency::Annual, false);
}

MarketConvention MarketConvention::sonia_ois() {
  return MarketConvention(
      "GBP SONIA OIS", UnitedKingdom(UnitedKingdom::Market::Settlement),
      DayCounter::actual_365_fixed(), BusinessDayConvention::ModifiedFollowing,
      0, Frequency::Annual, false);
}

MarketConvention MarketConvention::euribor_3m() {
  return MarketConvention("EUR EURIBOR 3M", Target(), DayCounter::actual_360(),
                          BusinessDayConvention::ModifiedFollowing, 2,
                          Frequency::Quarterly, false);
}

MarketConvention MarketConvention::euribor_6m() {
  return MarketConvention("EUR EURIBOR 6M", Target(), DayCounter::actual_360(),
                          BusinessDayConvention::ModifiedFollowing, 2,
                          Frequency::SemiAnnual, false);
}

MarketConvention MarketConvention::us_treasury() {
  return MarketConvention(
      "USD Treasury", UnitedStates(UnitedStates::Market::GovernmentBond),
      DayCounter::actual_actual_icma(Frequency::SemiAnnual),
      BusinessDayConvention::Following, 1, Frequency::SemiAnnual, false);
}

MarketConvention MarketConvention::german_bund() {
  return MarketConvention("EUR German Bund", Target(),
                          DayCounter::actual_actual_icma(Frequency::Annual),
                          BusinessDayConvention::Following, 2,
                          Frequency::Annual, false);
}

MarketConvention MarketConvention::uk_gilt() {
  return MarketConvention(
      "GBP UK Gilt", UnitedKingdom(UnitedKingdom::Market::Settlement),
      DayCounter::actual_actual_icma(Frequency::SemiAnnual),
      BusinessDayConvention::Following, 1, Frequency::SemiAnnual, false);
}

std::optional<MarketConvention>
MarketConvention::lookup(std::string_view name) {
  std::string clean;
  clean.reserve(name.size());
  for (char c : name) {
    if (c != ' ' && c != '_' && c != '-') {
      clean.push_back(
          static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
  }

  if (clean == "SOFR" || clean == "USDSOFR" || clean == "USDSOFROIS" ||
      clean == "SOFROIS") {
    return sofr_ois();
  }
  if (clean == "ESTR" || clean == "EURESTR" || clean == "ESTROIS" ||
      clean == "EURESTROIS") {
    return estr_ois();
  }
  if (clean == "SONIA" || clean == "GBPSONIA" || clean == "SONIAOIS") {
    return sonia_ois();
  }
  if (clean == "EURIBOR3M" || clean == "EURIBOR") {
    return euribor_3m();
  }
  if (clean == "EURIBOR6M") {
    return euribor_6m();
  }
  if (clean == "USTREASURY" || clean == "TREASURY" || clean == "USGOV") {
    return us_treasury();
  }
  if (clean == "GERMANBUND" || clean == "BUND") {
    return german_bund();
  }
  if (clean == "UKGILT" || clean == "GILT") {
    return uk_gilt();
  }

  return std::nullopt;
}

} // namespace alm::time
