#include "xva/XVAAggregator.hpp"

#include <iomanip>
#include <iostream>
#include <memory>

#include "utils/Logger.hpp"

namespace xva {

XVAAggregator::XVAAggregator(const XVAConfig& config) : config_(config) {
    expCalc_ = std::make_unique<ExposureCalculator>(config_.collateralThreshold,
                                                    config_.minimumTransferAmt);

    if (config_.computeCVA)
        cvaCalc_ = std::make_unique<CVACalculator>(config_.counterpartyRecovery,
                                                   config_.counterpartyCurve);

    if (config_.computeDVA)
        dvaCalc_ = std::make_unique<DVACalculator>(config_.ownRecovery, config_.ownCurve);

    if (config_.computeFVA)
        fvaCalc_ =
            std::make_unique<FVACalculator>(config_.fundingBorrowSpread, config_.fundingLendSpread);

    if (config_.computeMVA) mvaCalc_ = std::make_unique<MVACalculator>(config_.imFundingSpread);
}

XVAResult XVAAggregator::compute(const SimulationResult& result, const YieldCurve& disc) {
    // Apply collateral / CSA
    SimulationResult res = result;
    expCalc_->compute(res);

    XVAResult xva;

    if (config_.computeCVA && cvaCalc_) {
        xva.CVA = cvaCalc_->compute(res, disc);
        LOG_INFO("CVA = ", std::fixed, std::setprecision(4), xva.CVA);
    }
    if (config_.computeDVA && dvaCalc_) {
        xva.DVA = dvaCalc_->compute(res, disc);
        LOG_INFO("DVA = ", std::fixed, std::setprecision(4), xva.DVA);
    }
    if (config_.computeFVA && fvaCalc_) {
        xva.FVA = fvaCalc_->compute(res, disc);
        LOG_INFO("FVA = ", std::fixed, std::setprecision(4), xva.FVA);
    }
    if (config_.computeMVA && mvaCalc_) {
        auto imProfile = expCalc_->computeInitialMargin(res, config_.imMultiplier);
        xva.MVA = mvaCalc_->compute(res, imProfile, disc);
        LOG_INFO("MVA = ", std::fixed, std::setprecision(4), xva.MVA);
    }

    LOG_INFO("Total XVA = ", std::fixed, std::setprecision(4), xva.total());
    return xva;
}

void XVAAggregator::printReport(const XVAResult& r, std::ostream& os) const {
    os << "\n========================================\n";
    os << "           XVA Report\n";
    os << "========================================\n";
    os << std::fixed << std::setprecision(6);
    os << "  CVA  : " << std::setw(14) << r.CVA << "\n";
    os << "  DVA  : " << std::setw(14) << r.DVA << "\n";
    os << "  FVA  : " << std::setw(14) << r.FVA << "\n";
    os << "  MVA  : " << std::setw(14) << r.MVA << "\n";
    os << "----------------------------------------\n";
    os << "  Total: " << std::setw(14) << r.total() << "\n";
    os << "========================================\n\n";
}

}  // namespace xva
