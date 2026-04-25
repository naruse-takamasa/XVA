#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

#include "Common.hpp"
#include "instruments/InterestRateSwap.hpp"
#include "models/HullWhiteModel.hpp"
#include "simulation/ExposureCalculator.hpp"
#include "simulation/MonteCarloEngine.hpp"
#include "utils/DateUtils.hpp"
#include "utils/Logger.hpp"
#include "xva/XVAAggregator.hpp"

using namespace xva;

// ── Helper: build a flat yield curve ─────────────────────────────────────────
YieldCurve makeFlatCurve(Real rate, int maxYears = 30) {
    YieldCurve c;
    for (int y = 0; y <= maxYears; ++y) {
        c.tenors.push_back(static_cast<Real>(y));
        c.discountFactors.push_back(std::exp(-rate * y));
    }
    return c;
}

// ── Helper: build a flat credit curve (constant hazard rate) ─────────────────
CreditCurve makeFlatCreditCurve(const std::string& name, Real hazardRate, int maxYears = 30) {
    CreditCurve cc;
    cc.name = name;
    for (int y = 0; y <= maxYears; ++y) {
        cc.tenors.push_back(static_cast<Real>(y));
        cc.survivalProbs.push_back(std::exp(-hazardRate * y));
    }
    return cc;
}

int main() {
    Logger::instance().setLevel(LogLevel::INFO);

    LOG_INFO("=== XVA Calculator – IRS Example ===");

    // ── 1. Market data ───────────────────────────────────────────────────────
    Real riskFreeRate = 0.03;  // 3% flat
    YieldCurve curve = makeFlatCurve(riskFreeRate);

    // ── 2. Hull-White model parameters ───────────────────────────────────────
    Real hwA = 0.05;      // mean-reversion speed
    Real hwSigma = 0.01;  // short-rate volatility
    HullWhiteModel hwModel(hwA, hwSigma, curve);

    // ── 3. Interest Rate Swap ─────────────────────────────────────────────────
    SwapSchedule sched;
    sched.startDate = 0.0;  // starts today
    sched.endDate = 5.0;    // 5-year swap
    sched.notional = 1'000'000.0;
    sched.fixedFreq = 1;  // annual fixed payments
    sched.floatFreq = 2;  // semi-annual floating

    // Set fixed rate = par rate (at-the-money swap)
    sched.fixedRate = hwModel.initialForwardRate(0.0);  // approx par
    // More precisely, compute par rate:
    sched.dayCountConvention = "ACT365";

    InterestRateSwap swap(sched, /*payFixed=*/true);

    // Log par rate
    Real par = swap.parRate(curve);
    sched.fixedRate = par;
    InterestRateSwap atmSwap(sched, true);

    LOG_INFO("Par rate: ", std::fixed, std::setprecision(4), par * 100.0, "%");

    // ── 4. Time grid (monthly, 5 years) ───────────────────────────────────────
    TimeGrid grid = makeGrid(5.0, 1.0 / 12.0);

    // ── 5. Monte Carlo simulation ─────────────────────────────────────────────
    MCConfig mcCfg;
    mcCfg.numPaths = 5'000;
    mcCfg.useAntithetic = true;
    mcCfg.numThreads = 4;
    mcCfg.seed = 42;

    MonteCarloEngine mc(mcCfg, hwModel);
    SimulationResult simResult = mc.run(atmSwap, grid);

    // Print exposure profile (first 12 months)
    std::cout << "\n--- Exposure Profile (EE / PFE@95%) ---\n";
    std::cout << std::setw(8) << "t(yr)" << std::setw(14) << "EE" << std::setw(14) << "PFE(95%)"
              << "\n";
    for (std::size_t i = 0; i < grid.size() && i <= 12; ++i) {
        std::cout << std::fixed << std::setprecision(3) << std::setw(8) << grid[i] << std::setw(14)
                  << simResult.EE[i] << std::setw(14) << simResult.PFE[i] << "\n";
    }

    // ── 6. XVA configuration ──────────────────────────────────────────────────
    XVAConfig xvaCfg;

    // Credit curves (100 bps counterparty, 50 bps own)
    xvaCfg.counterpartyCurve = makeFlatCreditCurve("Counterparty", 0.01);
    xvaCfg.ownCurve = makeFlatCreditCurve("Self", 0.005);
    xvaCfg.counterpartyRecovery = 0.40;
    xvaCfg.ownRecovery = 0.40;

    // Funding spreads
    xvaCfg.fundingBorrowSpread = 0.005;  // 50 bps
    xvaCfg.fundingLendSpread = 0.002;    // 20 bps

    // IM funding
    xvaCfg.imFundingSpread = 0.003;  // 30 bps
    xvaCfg.imMultiplier = 1.4;

    // No collateral for this example
    xvaCfg.collateralThreshold = 0.0;

    // ── 7. Compute XVA ────────────────────────────────────────────────────────
    XVAAggregator aggregator(xvaCfg);
    XVAResult xva = aggregator.compute(simResult, curve);

    aggregator.printReport(xva, std::cout);

    // ── 8. Sensitivity (CS01 example) ─────────────────────────────────────────
    CVACalculator cvaCalc(xvaCfg.counterpartyRecovery, xvaCfg.counterpartyCurve);
    Real cs01 = cvaCalc.cs01(simResult, curve);
    std::cout << "CVA CS01 (per bp): " << std::fixed << std::setprecision(4) << cs01 << "\n\n";

    return 0;
}
