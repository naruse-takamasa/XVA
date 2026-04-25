#pragma once
#include "Common.hpp"

namespace xva {

/**
 * DVA = (1 - R_o) * sum_i [ ENE(t_i) * (Q_o(t_{i-1}) - Q_o(t_i)) * D(t_i) ]
 *
 *  R_o : own recovery rate
 *  Q_o : own survival probability
 *  ENE : Expected Negative Exposure (from counterparty's perspective)
 *
 * Note: DVA reduces CVA (bilateral adjustment). Sign convention:
 *       positive DVA = benefit to us.
 */
class DVACalculator {
public:
    DVACalculator(Real ownRecoveryRate, const CreditCurve& ownCurve);

    Real compute(const SimulationResult& result, const YieldCurve& discountCurve) const;

private:
    Real ownRecovery_;
    CreditCurve ownCurve_;
};

}  // namespace xva
