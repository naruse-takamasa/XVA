#include "xva/FVACalculator.hpp"
#include <cmath>

namespace xva {

FVACalculator::FVACalculator(Real spread)
    : borrowingSpread_(spread), lendingSpread_(spread) {}

FVACalculator::FVACalculator(Real borrow, Real lend)
    : borrowingSpread_(borrow), lendingSpread_(lend) {}

// FCA = -s_borrow * sum_i EE(t_i) * D(t_i) * dt_i
Real FVACalculator::computeFCA(const SimulationResult& result,
                                const YieldCurve& disc) const {
    Real fca = 0.0;
    std::size_t N = result.timeGrid.size();
    for (std::size_t i = 1; i < N; ++i) {
        Real dt = result.timeGrid[i] - result.timeGrid[i - 1];
        Real df = disc.df(result.timeGrid[i]);
        fca    += result.EE[i] * df * dt;
    }
    return -borrowingSpread_ * fca;
}

// FBA = +s_lend * sum_i |ENE(t_i)| * D(t_i) * dt_i
Real FVACalculator::computeFBA(const SimulationResult& result,
                                const YieldCurve& disc) const {
    Real fba = 0.0;
    std::size_t N = result.timeGrid.size();
    for (std::size_t i = 1; i < N; ++i) {
        Real dt = result.timeGrid[i] - result.timeGrid[i - 1];
        Real df = disc.df(result.timeGrid[i]);
        fba    += (-result.ENE[i]) * df * dt;   // ENE < 0, flip
    }
    return lendingSpread_ * fba;
}

Real FVACalculator::compute(const SimulationResult& result,
                             const YieldCurve& disc) const {
    return computeFCA(result, disc) + computeFBA(result, disc);
}

} // namespace xva
