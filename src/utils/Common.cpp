#include "Common.hpp"

#include <stdexcept>

#include "utils/MathUtils.hpp"

namespace xva {

// ── YieldCurve ───────────────────────────────────────────────────────────────

Real YieldCurve::df(Date t) const {
    if (tenors.empty()) throw std::runtime_error("YieldCurve: empty");
    return math::interp(tenors, discountFactors, t);
}

Real YieldCurve::fwdRate(Date t1, Date t2) const {
    if (t2 <= t1) throw std::invalid_argument("fwdRate: t2 must be > t1");
    Real df1 = df(t1);
    Real df2 = df(t2);
    return std::log(df1 / df2) / (t2 - t1);
}

// ── CreditCurve ──────────────────────────────────────────────────────────────

Real CreditCurve::survProb(Date t) const {
    if (tenors.empty()) return 1.0;
    return math::interp(tenors, survivalProbs, t);
}

Real CreditCurve::hazardRate(Date t) const {
    const Real dt = 1e-4;
    Real q1 = survProb(t);
    Real q2 = survProb(t + dt);
    if (q1 < 1e-12) return 0.0;
    return -std::log(q2 / q1) / dt;
}

}  // namespace xva
