#include "xva/MVACalculator.hpp"

#include <cmath>
#include <vector>

namespace xva {

MVACalculator::MVACalculator(Real spread) : imFundingSpread_(spread) {}

// MVA = -s_im * sum_i IM(t_i) * D(t_i) * dt_i
Real MVACalculator::compute(const SimulationResult& result, const std::vector<Real>& imProfile,
                            const YieldCurve& disc) const {
    Real mva = 0.0;
    std::size_t N = result.timeGrid.size();
    std::size_t imN = imProfile.size();

    for (std::size_t i = 1; i < N && i < imN; ++i) {
        Real dt = result.timeGrid[i] - result.timeGrid[i - 1];
        Real df = disc.df(result.timeGrid[i]);
        mva += imProfile[i] * df * dt;
    }
    return -imFundingSpread_ * mva;
}

Real MVACalculator::fundingSpreadSensitivity(const SimulationResult& result,
                                             const std::vector<Real>& imProfile,
                                             const YieldCurve& disc) const {
    // Linear: dMVA/ds = MVA / s
    Real mva = compute(result, imProfile, disc);
    return mva / imFundingSpread_ * 0.0001;  // per bp
}

}  // namespace xva
