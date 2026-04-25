#pragma once
#include "Common.hpp"

namespace xva {

/**
 * Post-processes raw MC path values into regulatory/XVA exposure profiles.
 *
 * Supports:
 *  - EE  : Expected Exposure  E[max(V,0)]
 *  - ENE : Expected Negative Exposure  E[min(V,0)]
 *  - EPE : Effective Positive Exposure (BCBS definition)
 *  - PFE : Potential Future Exposure at confidence level alpha
 *  - IM  : Initial Margin via SIMM-like proxy (for MVA)
 */
class ExposureCalculator {
public:
    explicit ExposureCalculator(Real collateralThreshold = 0.0,
                                Real minimumTransferAmt  = 0.0);

    // Compute exposure metrics from simulation result (modifies result in place)
    void compute(SimulationResult& result) const;

    // Compute SIMM-like initial margin proxy profile
    // Returns IM[timeStep]
    std::vector<Real> computeInitialMargin(
        const SimulationResult& result,
        Real imMultiplier = 1.4) const;     // Basel multiplier

    // Effective EPE (for CVA capital)
    Real effectiveEPE(const SimulationResult& result) const;

    // Alpha multiplier (regulatory, default 1.4)
    void setAlpha(Real alpha) { alpha_ = alpha; }

private:
    Real collateralThreshold_;
    Real minimumTransferAmt_;
    Real alpha_ = 1.4;
    Real pfeCL_ = 0.95;  // PFE confidence level

    // Apply collateral (netting + CSA)
    std::vector<Real> applyCollateral(const std::vector<Real>& values) const;
};

} // namespace xva
