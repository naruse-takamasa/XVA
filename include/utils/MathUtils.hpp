#pragma once
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "Common.hpp"

namespace xva::math {

// Standard normal CDF (Abramowitz & Stegun approximation)
inline Real normCDF(Real x) {
    static const Real a1 = 0.254829592;
    static const Real a2 = -0.284496736;
    static const Real a3 = 1.421413741;
    static const Real a4 = -1.453152027;
    static const Real a5 = 1.061405429;
    static const Real p = 0.3275911;
    Real sign = (x < 0) ? -1.0 : 1.0;
    x = std::abs(x) / std::sqrt(2.0);
    Real t = 1.0 / (1.0 + p * x);
    Real y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);
    return 0.5 * (1.0 + sign * y);
}

// Inverse normal CDF (Beasley-Springer-Moro approximation)
Real normInvCDF(Real p);

// Compute quantile of a sorted vector
inline Real quantile(std::vector<Real> v, Real alpha) {
    std::sort(v.begin(), v.end());
    std::size_t idx = static_cast<std::size_t>(alpha * v.size());
    return v[std::min(idx, v.size() - 1)];
}

// Linear interpolation
inline Real interp(const std::vector<Real>& xs, const std::vector<Real>& ys, Real x) {
    if (x <= xs.front()) return ys.front();
    if (x >= xs.back()) return ys.back();
    auto it = std::lower_bound(xs.begin(), xs.end(), x);
    std::size_t i = std::distance(xs.begin(), it) - 1;
    Real t = (x - xs[i]) / (xs[i + 1] - xs[i]);
    return ys[i] + t * (ys[i + 1] - ys[i]);
}

// Trapezoidal integration
inline Real trapz(const std::vector<Real>& x, const std::vector<Real>& y) {
    Real s = 0.0;
    for (std::size_t i = 1; i < x.size(); ++i) s += 0.5 * (y[i] + y[i - 1]) * (x[i] - x[i - 1]);
    return s;
}

}  // namespace xva::math
