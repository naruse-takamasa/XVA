#pragma once
#include "Common.hpp"

namespace xva {

/**
 * CVA = (1 - R_c) * sum_i [ EE(t_i) * (Q_c(t_{i-1}) - Q_c(t_i)) * D(t_i) ]
 *
 *  R_c : counterparty recovery rate
 *  Q_c : counterparty survival probability
 *  D   : risk-free discount factor
 *  EE  : Expected Exposure
 */
class CVACalculator {
public:
    CVACalculator(Real recoveryRate, const CreditCurve& counterpartyCurve);

    Real compute(const SimulationResult& result,
                 const YieldCurve&       discountCurve) const;

    // Unilateral CVA (no DVA offset)
    Real computeUnilateral(const SimulationResult& result,
                           const YieldCurve&       discountCurve) const;

    // CVA sensitivity to parallel shift in credit spread (CS01)
    Real cs01(const SimulationResult& result,
              const YieldCurve&       discountCurve,
              Real bpShift = 0.0001) const;

private:
    Real        recoveryRate_;
    CreditCurve cpCurve_;
};

} // namespace xva
