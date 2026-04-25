#include "simulation/ExposureCalculator.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "utils/MathUtils.hpp"

namespace xva {

ExposureCalculator::ExposureCalculator(Real threshold, Real mta)
    : collateralThreshold_(threshold), minimumTransferAmt_(mta) {}

void ExposureCalculator::compute(SimulationResult& result) const {
    // Profiles already computed in MC engine; apply collateral if needed
    if (collateralThreshold_ > 0.0 || minimumTransferAmt_ > 0.0) {
        std::size_t N = result.timeGrid.size();
        std::size_t P = result.numPaths;

        for (std::size_t i = 0; i < N; ++i) {
            Real ee = 0.0, ene = 0.0;
            for (std::size_t p = 0; p < P; ++p) {
                Real v = result.pathValues[p][i];
                // Simple threshold collateral
                Real collat = std::max(0.0, std::abs(v) - collateralThreshold_);
                if (collat < minimumTransferAmt_) collat = 0.0;
                Real vNet = (v > 0) ? std::max(0.0, v - collat) : std::min(0.0, v + collat);
                ee += std::max(vNet, 0.0);
                ene += std::min(vNet, 0.0);
            }
            result.EE[i] = ee / static_cast<Real>(P);
            result.ENE[i] = ene / static_cast<Real>(P);
        }
    }
}

// SIMM-proxy: IM ≈ imMultiplier * EE * sqrt(10/250) * sqrt(252/dt)
// Simplified: IM proportional to volatility of exposure
std::vector<Real> ExposureCalculator::computeInitialMargin(const SimulationResult& result,
                                                           Real imMultiplier) const {
    std::size_t N = result.timeGrid.size();
    std::size_t P = result.numPaths;
    std::vector<Real> im(N, 0.0);

    for (std::size_t i = 0; i < N; ++i) {
        // IM proxy: standard deviation of path values * multiplier
        Real mean = 0.0;
        for (std::size_t p = 0; p < P; ++p) mean += result.pathValues[p][i];
        mean /= static_cast<Real>(P);

        Real var = 0.0;
        for (std::size_t p = 0; p < P; ++p) {
            Real d = result.pathValues[p][i] - mean;
            var += d * d;
        }
        var /= static_cast<Real>(P - 1);

        // SIMM-like: IM = multiplier * sigma * sqrt(MPOR/1yr)
        // MPOR = 10 business days ≈ 10/252 years
        im[i] = imMultiplier * std::sqrt(var) * std::sqrt(10.0 / 252.0);
    }
    return im;
}

Real ExposureCalculator::effectiveEPE(const SimulationResult& result) const {
    return result.EEPE.empty() ? 0.0 : result.EEPE[0];
}

}  // namespace xva
