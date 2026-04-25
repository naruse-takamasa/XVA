#pragma once
#include "Common.hpp"
#include <random>

namespace xva {

/**
 * Hull-White 1-factor model
 *
 *   dr(t) = [theta(t) - a*r(t)] dt + sigma dW(t)
 *
 *  a     : mean-reversion speed
 *  sigma : short-rate volatility
 *  theta : time-dependent drift (calibrated to initial yield curve)
 */
class HullWhiteModel {
public:
    HullWhiteModel(Real a, Real sigma, const YieldCurve& curve);

    // Analytical bond price P(t, T) given r(t)
    Real bondPrice(Date t, Date T, Real rt) const;

    // Analytical forward rate f(0, t)
    Real initialForwardRate(Date t) const;

    // B(t,T) function in HW
    Real B(Date t, Date T) const;

    // A(t,T) function in HW (log-normal part)
    Real A(Date t, Date T) const;

    // Variance of r(t)
    Real variance(Date t) const;

    // Simulate short-rate paths on a time grid (Euler-Maruyama)
    // Returns [path][timeStep] matrix of short rates
    Matrix simulatePaths(const TimeGrid& grid,
                         std::size_t     numPaths,
                         std::mt19937_64& rng) const;

    // Accessors
    Real a()     const { return a_; }
    Real sigma() const { return sigma_; }

private:
    Real        a_;
    Real        sigma_;
    YieldCurve  curve_;

    Real theta(Date t) const;  // theta(t) from initial curve
};

} // namespace xva
