#include "xva/CVACalculator.hpp"
#include "utils/MathUtils.hpp"
#include <cmath>

namespace xva {

CVACalculator::CVACalculator(Real recoveryRate, const CreditCurve& cpCurve)
    : recoveryRate_(recoveryRate), cpCurve_(cpCurve) {}

// CVA = (1-R) * sum_i EE(t_i) * [Q(t_{i-1}) - Q(t_i)] * D(t_i)
Real CVACalculator::compute(const SimulationResult& result,
                             const YieldCurve& disc) const {
    return computeUnilateral(result, disc);
}

Real CVACalculator::computeUnilateral(const SimulationResult& result,
                                       const YieldCurve& disc) const {
    Real cva = 0.0;
    std::size_t N = result.timeGrid.size();
    Real lgd = 1.0 - recoveryRate_;

    for (std::size_t i = 1; i < N; ++i) {
        Date t_prev = result.timeGrid[i - 1];
        Date t_curr = result.timeGrid[i];

        Real ee    = result.EE[i];
        Real df    = disc.df(t_curr);
        Real dq    = cpCurve_.survProb(t_prev) - cpCurve_.survProb(t_curr);

        cva += lgd * ee * dq * df;
    }
    return -cva;  // CVA is a cost: negative for party holding long exposure
}

// CS01: bump-and-reprice on credit spread
Real CVACalculator::cs01(const SimulationResult& result,
                          const YieldCurve& disc,
                          Real bpShift) const {
    // Build bumped curve: Q_bumped(t) = Q(t) * exp(-bpShift * t)
    CreditCurve bumped = cpCurve_;
    for (std::size_t i = 0; i < bumped.survivalProbs.size(); ++i) {
        Real t = bumped.tenors[i];
        bumped.survivalProbs[i] *= std::exp(-bpShift * t);
    }
    CVACalculator bumpedCalc(recoveryRate_, bumped);
    Real cva0    = compute(result, disc);
    Real cvaBump = bumpedCalc.compute(result, disc);
    return (cvaBump - cva0) / (bpShift / 0.0001);  // per bp
}

} // namespace xva
