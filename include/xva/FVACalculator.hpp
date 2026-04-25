#pragma once
#include "Common.hpp"

namespace xva {

/**
 * Funding Valuation Adjustment
 *
 * FVA = FCA + FBA   (Funding Cost + Funding Benefit)
 *
 * FCA = -s_f * sum_i [ EE(t_i)  * D(t_i) * dt_i ]   (cost of funding assets)
 * FBA = +s_f * sum_i [ ENE(t_i) * D(t_i) * dt_i ]   (benefit of funding liab)
 *
 * where s_f = funding spread (over risk-free)
 *
 * Simplified model: symmetric funding spread.
 * Extended: asymmetric FCA/FBA spreads supported.
 */
class FVACalculator {
public:
    // Symmetric funding spread
    explicit FVACalculator(Real fundingSpread);

    // Asymmetric: borrowing vs lending spread
    FVACalculator(Real borrowingSpread, Real lendingSpread);

    Real compute(const SimulationResult& result, const YieldCurve& discountCurve) const;

    Real computeFCA(const SimulationResult& result, const YieldCurve& discountCurve) const;

    Real computeFBA(const SimulationResult& result, const YieldCurve& discountCurve) const;

private:
    Real borrowingSpread_;
    Real lendingSpread_;
};

}  // namespace xva
