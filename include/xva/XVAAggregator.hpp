#pragma once
#include "Common.hpp"
#include "xva/CVACalculator.hpp"
#include "xva/DVACalculator.hpp"
#include "xva/FVACalculator.hpp"
#include "xva/MVACalculator.hpp"
#include "simulation/ExposureCalculator.hpp"
#include <memory>
#include <iostream>

namespace xva {

struct XVAConfig {
    // Credit
    Real        counterpartyRecovery = 0.40;
    Real        ownRecovery          = 0.40;
    CreditCurve counterpartyCurve;
    CreditCurve ownCurve;

    // Funding
    Real        fundingBorrowSpread  = 0.0050;  // 50 bps
    Real        fundingLendSpread    = 0.0020;  // 20 bps

    // Margin
    Real        imFundingSpread      = 0.0030;  // 30 bps
    Real        imMultiplier         = 1.4;     // Basel multiplier

    // Collateral / CSA
    Real        collateralThreshold  = 0.0;
    Real        minimumTransferAmt   = 0.0;

    // Flags
    bool        computeCVA = true;
    bool        computeDVA = true;
    bool        computeFVA = true;
    bool        computeMVA = true;
};

/**
 * Orchestrates CVA / DVA / FVA / MVA calculation from a SimulationResult.
 */
class XVAAggregator {
public:
    explicit XVAAggregator(const XVAConfig& config);

    // Compute all enabled XVA components
    XVAResult compute(const SimulationResult& result,
                      const YieldCurve&       discountCurve);

    // Detailed breakdown (prints to stream)
    void printReport(const XVAResult& result,
                     std::ostream& os = std::cout) const;

private:
    XVAConfig config_;

    std::unique_ptr<CVACalculator> cvaCalc_;
    std::unique_ptr<DVACalculator> dvaCalc_;
    std::unique_ptr<FVACalculator> fvaCalc_;
    std::unique_ptr<MVACalculator> mvaCalc_;
    std::unique_ptr<ExposureCalculator> expCalc_;
};

} // namespace xva
