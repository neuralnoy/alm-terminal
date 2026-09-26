#include "time/schedule.h"

#include <algorithm>
#include <stdexcept>

namespace alm::time {

Schedule::Schedule(std::vector<Date> dates, std::vector<Date> unadjusted_dates,
                   std::vector<uint8_t> is_regular, Calendar calendar,
                   BusinessDayConvention convention)
    : dates_{std::move(dates)}, unadjusted_dates_{std::move(unadjusted_dates)},
      is_regular_{std::move(is_regular)}, calendar_{std::move(calendar)},
      convention_{convention} {}

SchedulePeriod Schedule::period(size_t index) const {
  if (index >= periods_count()) {
    throw std::out_of_range("Period index out of range in Schedule");
  }
  return SchedulePeriod{dates_[index], dates_[index + 1],
                        unadjusted_dates_[index], unadjusted_dates_[index + 1],
                        static_cast<bool>(is_regular_[index])};
}

ScheduleBuilder &ScheduleBuilder::from(Date effective_date) {
  effective_date_ = effective_date;
  return *this;
}

ScheduleBuilder &ScheduleBuilder::to(Date termination_date) {
  termination_date_ = termination_date;
  return *this;
}

ScheduleBuilder &ScheduleBuilder::tenor(const Period &tenor) {
  tenor_ = tenor;
  return *this;
}

ScheduleBuilder &ScheduleBuilder::frequency(Frequency freq) {
  tenor_ = Period::from_frequency(freq);
  return *this;
}

ScheduleBuilder &ScheduleBuilder::calendar(Calendar cal) {
  calendar_ = std::move(cal);
  return *this;
}

ScheduleBuilder &ScheduleBuilder::convention(BusinessDayConvention conv) {
  convention_ = conv;
  return *this;
}

ScheduleBuilder &
ScheduleBuilder::termination_convention(BusinessDayConvention conv) {
  termination_convention_ = conv;
  return *this;
}

ScheduleBuilder &ScheduleBuilder::rule(DateGenerationRule rule) {
  rule_ = rule;
  return *this;
}

ScheduleBuilder &ScheduleBuilder::end_of_month(bool eom) {
  end_of_month_ = eom;
  return *this;
}

ScheduleBuilder &ScheduleBuilder::first_date(Date d) {
  first_date_ = d;
  return *this;
}

ScheduleBuilder &ScheduleBuilder::next_to_last_date(Date d) {
  next_to_last_date_ = d;
  return *this;
}

Schedule ScheduleBuilder::build() const {
  if (effective_date_ > termination_date_) {
    throw std::invalid_argument(
        "Effective date must be before or equal to termination date");
  }

  BusinessDayConvention term_conv =
      termination_convention_.value_or(convention_);

  if (effective_date_ == termination_date_) {
    Date adj = calendar_.adjust(effective_date_, term_conv);
    return Schedule({adj}, {effective_date_}, {}, calendar_, convention_);
  }

  if (rule_ == DateGenerationRule::Zero) {
    std::vector<Date> unadj = {effective_date_, termination_date_};
    std::vector<Date> adj = {calendar_.adjust(effective_date_, convention_),
                             calendar_.adjust(termination_date_, term_conv)};
    std::vector<uint8_t> reg = {1};
    return Schedule(std::move(adj), std::move(unadj), std::move(reg), calendar_,
                    convention_);
  }

  if (tenor_.length() <= 0) {
    throw std::invalid_argument("Tenor length must be positive");
  }

  std::vector<Date> unadjusted;
  unadjusted.reserve(16);

  if (rule_ == DateGenerationRule::Backward) {
    unadjusted.push_back(termination_date_);
    Date seed = termination_date_;

    if (next_to_last_date_.has_value()) {
      if (*next_to_last_date_ >= termination_date_ ||
          *next_to_last_date_ <= effective_date_) {
        throw std::invalid_argument(
            "next_to_last_date must be between effective and termination date");
      }
      unadjusted.push_back(*next_to_last_date_);
      seed = *next_to_last_date_;
    }

    Date exit_date = first_date_.value_or(effective_date_);
    int periods = 1;
    while (true) {
      Date temp = advance(seed, -(tenor_ * periods), end_of_month_);
      if (temp <= exit_date) {
        break;
      }
      unadjusted.push_back(temp);
      ++periods;
    }

    if (first_date_.has_value()) {
      if (*first_date_ <= effective_date_ ||
          *first_date_ >= termination_date_) {
        throw std::invalid_argument(
            "first_date must be between effective and termination date");
      }
      unadjusted.push_back(*first_date_);
    }

    unadjusted.push_back(effective_date_);
    std::reverse(unadjusted.begin(), unadjusted.end());

  } else if (rule_ == DateGenerationRule::Forward) {
    unadjusted.push_back(effective_date_);
    Date seed = effective_date_;

    if (first_date_.has_value()) {
      if (*first_date_ <= effective_date_ ||
          *first_date_ >= termination_date_) {
        throw std::invalid_argument(
            "first_date must be between effective and termination date");
      }
      unadjusted.push_back(*first_date_);
      seed = *first_date_;
    }

    Date exit_date = next_to_last_date_.value_or(termination_date_);
    int periods = 1;
    while (true) {
      Date temp = advance(seed, tenor_ * periods, end_of_month_);
      if (temp >= exit_date) {
        break;
      }
      unadjusted.push_back(temp);
      ++periods;
    }

    if (next_to_last_date_.has_value()) {
      if (*next_to_last_date_ >= termination_date_ ||
          *next_to_last_date_ <= effective_date_) {
        throw std::invalid_argument(
            "next_to_last_date must be between effective and termination date");
      }
      unadjusted.push_back(*next_to_last_date_);
    }

    unadjusted.push_back(termination_date_);

  } else if (rule_ == DateGenerationRule::ThirdWednesday) {
    // Generate 3rd Wednesday of every tenor step
    Date cur = effective_date_;
    unadjusted.push_back(cur);
    while (cur < termination_date_) {
      Date next_month = cur.add_months(
          tenor_.units() == TimeUnit::Months ? tenor_.length() : 3);
      Date third_wed = nth_weekday(3, std::chrono::Wednesday,
                                   next_month.month(), next_month.year());
      if (third_wed >= termination_date_) {
        break;
      }
      if (third_wed > cur) {
        unadjusted.push_back(third_wed);
        cur = third_wed;
      } else {
        cur = next_month;
      }
    }
    unadjusted.push_back(termination_date_);

  } else if (rule_ == DateGenerationRule::Twentieth ||
             rule_ == DateGenerationRule::TwentiethIMM) {
    Date cur = effective_date_;
    unadjusted.push_back(cur);

    if (rule_ == DateGenerationRule::TwentiethIMM) {
      int m = cur.month();
      int rem = m % 3;
      int months_to_add = (rem == 0) ? 0 : (3 - rem);
      Date next_imm(cur.year(), cur.month(), 20);
      next_imm = next_imm.add_months(months_to_add);
      if (next_imm <= cur) {
        next_imm = next_imm.add_months(3);
      }
      cur = next_imm;
    } else {
      Date next_twentieth(cur.year(), cur.month(), 20);
      if (next_twentieth <= cur) {
        int step_months =
            tenor_.units() == TimeUnit::Months ? tenor_.length() : 1;
        next_twentieth = next_twentieth.add_months(step_months);
      }
      cur = next_twentieth;
    }

    if (cur < termination_date_) {
      unadjusted.push_back(cur);
    }

    int step_months =
        (rule_ == DateGenerationRule::TwentiethIMM)
            ? 3
            : (tenor_.units() == TimeUnit::Months ? tenor_.length() : 1);

    while (cur < termination_date_) {
      Date next_month = cur.add_months(step_months);
      Date twentieth(next_month.year(), next_month.month(), 20);
      if (twentieth >= termination_date_) {
        break;
      }
      if (twentieth > cur) {
        unadjusted.push_back(twentieth);
        cur = twentieth;
      } else {
        cur = next_month;
      }
    }
    unadjusted.push_back(termination_date_);
  }

  // Deduplicate in case any dates coincide
  auto last = std::unique(unadjusted.begin(), unadjusted.end());
  unadjusted.erase(last, unadjusted.end());

  size_t n = unadjusted.size();
  std::vector<Date> adjusted;
  adjusted.reserve(n);

  for (size_t i = 0; i < n; ++i) {
    if (i == 0) {
      adjusted.push_back(calendar_.adjust(unadjusted[i], convention_));
    } else if (i == n - 1) {
      adjusted.push_back(calendar_.adjust(unadjusted[i], term_conv));
    } else {
      adjusted.push_back(calendar_.adjust(unadjusted[i], convention_));
    }
  }

  // Regularity check
  std::vector<uint8_t> is_regular;
  if (n > 1) {
    is_regular.reserve(n - 1);
    for (size_t i = 0; i < n - 1; ++i) {
      Date expected = advance(unadjusted[i], tenor_, end_of_month_);
      is_regular.push_back(expected == unadjusted[i + 1] ? 1 : 0);
    }
  }

  return Schedule(std::move(adjusted), std::move(unadjusted),
                  std::move(is_regular), calendar_, convention_);
}

std::string to_string(DateGenerationRule rule) {
  switch (rule) {
  case DateGenerationRule::Backward:
    return "Backward";
  case DateGenerationRule::Forward:
    return "Forward";
  case DateGenerationRule::Zero:
    return "Zero";
  case DateGenerationRule::ThirdWednesday:
    return "ThirdWednesday";
  case DateGenerationRule::Twentieth:
    return "Twentieth";
  case DateGenerationRule::TwentiethIMM:
    return "TwentiethIMM";
  }
  return "Unknown";
}

} // namespace alm::time
