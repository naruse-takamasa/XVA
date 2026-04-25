#include "instruments/InterestRateSwap.hpp"

#include <cmath>
#include <stdexcept>

namespace xva {

InterestRateSwap::InterestRateSwap(const SwapSchedule& sched, bool payFixed)
    : sched_(sched), payFixed_(payFixed) {
    buildSchedule();
}

void InterestRateSwap::buildSchedule() {
    // Build fixed payment dates
    Real fixedPeriod = 1.0 / sched_.fixedFreq;
    for (Real d = sched_.startDate + fixedPeriod; d <= sched_.endDate + 1e-9; d += fixedPeriod)
        fixedDates_.push_back(d);

    // Build float payment dates
    Real floatPeriod = 1.0 / sched_.floatFreq;
    for (Real d = sched_.startDate + floatPeriod; d <= sched_.endDate + 1e-9; d += floatPeriod)
        floatDates_.push_back(d);
}

Real InterestRateSwap::accrualFactor(Date t1, Date t2) const {
    // ACT/360, ACT/365, or 30/360 – default ACT/365
    if (sched_.dayCountConvention == "ACT360") return (t2 - t1) * 365.0 / 360.0;
    return t2 - t1;  // ACT/365 (year fraction already in years)
}

// NPV of fixed leg at time t with short rate rt
Real InterestRateSwap::fixedLegNPV(Date t, Real rt, const HullWhiteModel& m) const {
    Real npvFixed = 0.0;
    Real prevDate = sched_.startDate;
    for (Date payDate : fixedDates_) {
        if (payDate <= t) {
            prevDate = payDate;
            continue;
        }
        Real alpha = accrualFactor(prevDate, payDate);
        Real pv = m.bondPrice(t, payDate, rt) * sched_.fixedRate * alpha * sched_.notional;
        npvFixed += pv;
        prevDate = payDate;
    }
    return npvFixed;
}

// NPV of floating leg at time t with short rate rt
// Float leg ≈ P(t, T_start) - P(t, T_end) + accrued if within period
Real InterestRateSwap::floatLegNPV(Date t, Real rt, const HullWhiteModel& m) const {
    Real npvFloat = 0.0;
    Real prevDate = sched_.startDate;
    for (Date payDate : floatDates_) {
        if (payDate <= t) {
            prevDate = payDate;
            continue;
        }
        // Forward rate for this period
        Real df_start = (prevDate > t) ? m.bondPrice(t, prevDate, rt) : 1.0;
        Real df_end = m.bondPrice(t, payDate, rt);
        Real alpha = accrualFactor(prevDate, payDate);
        // Floating coupon PV = (df_start/df_end - 1) * df_end * N
        //                    = (df_start - df_end) * N
        npvFloat += (df_start - df_end) * sched_.notional;
        prevDate = payDate;
    }
    return npvFloat;
}

// NPV at time t: positive = asset (for payer swap: float - fixed)
Real InterestRateSwap::npv(Date t, Real rt, const HullWhiteModel& model) const {
    if (t >= sched_.endDate) return 0.0;

    Real fixed = fixedLegNPV(t, rt, model);
    Real flt = floatLegNPV(t, rt, model);

    // Payer swap (pay fixed): NPV = float - fixed
    // Receiver swap         : NPV = fixed - float
    return payFixed_ ? (flt - fixed) : (fixed - flt);
}

// Par rate: rate that makes NPV = 0 at t=0
Real InterestRateSwap::parRate(const YieldCurve& curve) const {
    // Annuity
    Real annuity = 0.0;
    Real prevDate = sched_.startDate;
    for (Date payDate : fixedDates_) {
        Real alpha = accrualFactor(prevDate, payDate);
        annuity += alpha * curve.df(payDate);
        prevDate = payDate;
    }
    // Float leg value
    Real dfStart = curve.df(sched_.startDate);
    Real dfEnd = curve.df(sched_.endDate);
    Real floatPV = (dfStart - dfEnd) * sched_.notional;

    if (std::abs(annuity) < 1e-12) throw std::runtime_error("IRS: zero annuity");
    return floatPV / (annuity * sched_.notional);
}

}  // namespace xva
