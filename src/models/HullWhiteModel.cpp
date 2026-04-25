#include "models/HullWhiteModel.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "utils/MathUtils.hpp"

namespace xva {

HullWhiteModel::HullWhiteModel(Real a, Real sigma, const YieldCurve& curve)
    : a_(a), sigma_(sigma), curve_(curve) {
    if (a <= 0) throw std::invalid_argument("HW: mean-reversion a must be > 0");
    if (sigma <= 0) throw std::invalid_argument("HW: sigma must be > 0");
}

// B(t,T) = (1 - exp(-a*(T-t))) / a
Real HullWhiteModel::B(Date t, Date T) const {
    if (T <= t) return 0.0;
    return (1.0 - std::exp(-a_ * (T - t))) / a_;
}

// ln A(t,T) = ln(P(0,T)/P(0,t)) + B(t,T)*f(0,t) - sigma^2/(4a)*B(t,T)^2*(1-exp(-2at))
Real HullWhiteModel::A(Date t, Date T) const {
    Real df0t = curve_.df(t);
    Real df0T = curve_.df(T);
    Real f0t = initialForwardRate(t);
    Real b = B(t, T);
    Real lnA = std::log(df0T / df0t) + b * f0t -
               (sigma_ * sigma_) / (4.0 * a_) * b * b * (1.0 - std::exp(-2.0 * a_ * t));
    return lnA;
}

// P(t,T) = exp(A(t,T) - B(t,T)*r(t))
Real HullWhiteModel::bondPrice(Date t, Date T, Real rt) const {
    if (T <= t) return 1.0;
    return std::exp(A(t, T) - B(t, T) * rt);
}

// f(0,t) = -d ln P(0,t) / dt  (numerical derivative)
Real HullWhiteModel::initialForwardRate(Date t) const {
    const Real h = 1e-5;
    Real df1 = curve_.df(std::max(t - h, 0.0));
    Real df2 = curve_.df(t + h);
    return -std::log(df2 / df1) / (2.0 * h);
}

// Var[r(t)] = sigma^2/(2a) * (1 - exp(-2at))
Real HullWhiteModel::variance(Date t) const {
    return (sigma_ * sigma_) / (2.0 * a_) * (1.0 - std::exp(-2.0 * a_ * t));
}

// theta(t) = d f(0,t)/dt + a*f(0,t) + sigma^2/(2a)*(1-exp(-2at))
Real HullWhiteModel::theta(Date t) const {
    const Real h = 1e-5;
    Real f0 = initialForwardRate(std::max(t - h, 0.0));
    Real f1 = initialForwardRate(t + h);
    Real dfdt = (f1 - f0) / (2.0 * h);
    return dfdt + a_ * initialForwardRate(t) +
           (sigma_ * sigma_) / a_ * (1.0 - std::exp(-2.0 * a_ * t)) * 0.5;
}

// Exact Gaussian discretisation of Hull-White:
// r(t+dt) = r(t)*e^{-a*dt} + [f(0,t+dt) - f(0,t)*e^{-a*dt}]
//           + sigma^2/(2a)*(1-e^{-a*dt})^2  +  sigma*sqrt((1-e^{-2a*dt})/(2a))*Z
Matrix HullWhiteModel::simulatePaths(const TimeGrid& grid, std::size_t numPaths,
                                     std::mt19937_64& rng) const {
    std::size_t N = grid.size();
    Matrix paths(numPaths, std::vector<Real>(N, 0.0));

    Real r0 = initialForwardRate(0.0);
    std::normal_distribution<Real> norm(0.0, 1.0);

    for (std::size_t p = 0; p < numPaths; ++p) {
        paths[p][0] = r0;
        for (std::size_t i = 1; i < N; ++i) {
            Real t = grid[i - 1];
            Real t1 = grid[i];
            Real dt = t1 - t;
            Real e_a = std::exp(-a_ * dt);

            // Exact conditional mean
            Real f0 = initialForwardRate(t);
            Real f1 = initialForwardRate(t1);
            Real convex = (sigma_ * sigma_) / (2.0 * a_) * (1.0 - e_a) * (1.0 - e_a);
            Real mean_r = paths[p][i - 1] * e_a + (f1 - f0 * e_a) + convex;

            // Exact conditional variance
            Real var_r = (sigma_ * sigma_) / (2.0 * a_) * (1.0 - e_a * e_a);

            paths[p][i] = mean_r + std::sqrt(var_r) * norm(rng);
        }
    }
    return paths;
}

}  // namespace xva
