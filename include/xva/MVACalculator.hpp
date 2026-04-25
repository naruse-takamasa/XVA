#pragma once
#include "Common.hpp"

namespace xva {

/**
 * Margin Valuation Adjustment
 *
 * MVA = -s_im * sum_i [ IM(t_i) * D(t_i) * dt_i ]
 *
 *  s_im : funding cost of posting initial margin (IM)
 *  IM   : Initial Margin profile (from ExposureCalculator)
 *
 * IM proxy: SIMM-inspired delta/vega sensitivity-based schedule approach.
 * The IM is computed as a function of the expected exposure profile
 * scaled by a regulatory multiplier.
 *
 * Reference: ISDA SIMM methodology, BCBS/IOSCO margin rules.
 */
class MVACalculator {
public:
    explicit MVACalculator(Real imFundingSpread);

    Real compute(const SimulationResult& result,
                 const std::vector<Real>& imProfile,
                 const YieldCurve&        discountCurve) const;

    // Sensitivity: MVA change per 1bp shift in funding spread
    Real fundingSpreadSensitivity(const SimulationResult& result,
                                  const std::vector<Real>& imProfile,
                                  const YieldCurve&        discountCurve) const;

private:
    Real imFundingSpread_;
};

} // namespace xva
