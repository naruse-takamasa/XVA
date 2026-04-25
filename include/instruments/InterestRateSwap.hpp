#pragma once
#include "Common.hpp"
#include "models/HullWhiteModel.hpp"

namespace xva {

enum class SwapLeg { Fixed, Floating };

struct SwapSchedule {
    Date        startDate;        // years from today
    Date        endDate;
    Real        fixedRate;
    Real        notional;
    int         fixedFreq;        // payments per year (e.g. 1=annual, 2=semi)
    int         floatFreq;
    std::string dayCountConvention;  // "ACT360", "ACT365", "30360"
};

/**
 * Vanilla Interest Rate Swap (pay-fixed / receive-float or vice versa)
 *
 * NPV under Hull-White: analytical via bond prices
 */
class InterestRateSwap {
public:
    // payFixed=true  -> we pay fixed, receive floating  (payer swap)
    // payFixed=false -> we receive fixed, pay floating  (receiver swap)
    InterestRateSwap(const SwapSchedule& sched, bool payFixed = true);

    // Analytical NPV at time t given short rate r(t)
    Real npv(Date t, Real rt, const HullWhiteModel& model) const;

    // NPV at time 0 (market)
    Real parRate(const YieldCurve& curve) const;

    // Schedule accessors
    const SwapSchedule& schedule() const { return sched_; }

    // Accrual factor (day count)
    Real accrualFactor(Date t1, Date t2) const;

private:
    SwapSchedule sched_;
    bool         payFixed_;

    std::vector<Date> fixedDates_;    // payment dates for fixed leg
    std::vector<Date> floatDates_;    // payment dates for float leg

    void buildSchedule();
    Real fixedLegNPV (Date t, Real rt, const HullWhiteModel& m) const;
    Real floatLegNPV (Date t, Real rt, const HullWhiteModel& m) const;
};

} // namespace xva
