#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace xva {

// ── Primitive aliases ────────────────────────────────────────────────────────
using Real = double;
using Date = double;  // time in years from today
using Matrix = std::vector<std::vector<Real>>;

// ── Date / time helpers ──────────────────────────────────────────────────────
struct TimeGrid {
    std::vector<Date> times;  // in years
    std::size_t size() const { return times.size(); }
    Date operator[](std::size_t i) const { return times[i]; }
};

// ── Cashflow ─────────────────────────────────────────────────────────────────
struct Cashflow {
    Date payDate;
    Real amount;
};

// ── Market data ──────────────────────────────────────────────────────────────
struct YieldCurve {
    std::vector<Date> tenors;  // years
    std::vector<Real> discountFactors;

    Real df(Date t) const;  // interpolated discount factor
    Real fwdRate(Date t1, Date t2) const;
};

// ── Credit curve ─────────────────────────────────────────────────────────────
struct CreditCurve {
    std::string name;
    std::vector<Date> tenors;
    std::vector<Real> survivalProbs;  // Q(tau > t)

    Real survProb(Date t) const;
    Real hazardRate(Date t) const;
};

// ── Simulation result ─────────────────────────────────────────────────────────
struct SimulationResult {
    TimeGrid timeGrid;
    std::size_t numPaths;
    // [path][timeStep] -> NPV of portfolio
    Matrix pathValues;

    // Derived exposure profiles
    std::vector<Real> EE;    // Expected Exposure
    std::vector<Real> ENE;   // Expected Negative Exposure
    std::vector<Real> PFE;   // Potential Future Exposure (95%)
    std::vector<Real> EEPE;  // Effective Expected Positive Exposure
};

// ── XVA result ────────────────────────────────────────────────────────────────
struct XVAResult {
    Real CVA = 0.0;  // Credit Valuation Adjustment
    Real DVA = 0.0;  // Debit Valuation Adjustment
    Real FVA = 0.0;  // Funding Valuation Adjustment
    Real MVA = 0.0;  // Margin Valuation Adjustment
    Real total() const { return CVA + DVA + FVA + MVA; }
};

}  // namespace xva
