#include "xva/DVACalculator.hpp"

#include <cmath>

namespace xva {

DVACalculator::DVACalculator(Real ownRecovery, const CreditCurve& ownCurve)
    : ownRecovery_(ownRecovery), ownCurve_(ownCurve) {}

// DVA = (1-R_o) * sum_i |ENE(t_i)| * [Q_o(t_{i-1}) - Q_o(t_i)] * D(t_i)
Real DVACalculator::compute(const SimulationResult& result, const YieldCurve& disc) const {
    Real dva = 0.0;
    std::size_t N = result.timeGrid.size();
    Real lgd = 1.0 - ownRecovery_;

    for (std::size_t i = 1; i < N; ++i) {
        Date t_prev = result.timeGrid[i - 1];
        Date t_curr = result.timeGrid[i];

        Real ene = result.ENE[i];  // negative number
        Real df = disc.df(t_curr);
        Real dq = ownCurve_.survProb(t_prev) - ownCurve_.survProb(t_curr);

        dva += lgd * (-ene) * dq * df;  // ENE is negative; flip sign
    }
    return dva;  // positive: benefit to us
}

}  // namespace xva
