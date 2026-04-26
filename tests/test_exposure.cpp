#include <gtest/gtest.h>

#include "instruments/InterestRateSwap.hpp"
#include "models/HullWhiteModel.hpp"
#include "simulation/ExposureCalculator.hpp"
#include "simulation/MonteCarloEngine.hpp"
#include "test_helpers.hpp"
#include "utils/DateUtils.hpp"

using namespace xva;

namespace {
SimulationResult runAtmSwap(double endDate, double dt, int numPaths, int seed) {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);

    SwapSchedule s;
    s.startDate = 0;
    s.endDate = endDate;
    s.notional = 1e6;
    s.fixedFreq = 1;
    s.floatFreq = 2;
    s.dayCountConvention = "ACT365";
    InterestRateSwap tmp(s, true);
    s.fixedRate = tmp.parRate(curve);
    InterestRateSwap atm(s, true);

    MCConfig cfg;
    cfg.numPaths = numPaths;
    cfg.numThreads = 2;
    cfg.seed = seed;
    MonteCarloEngine mc(cfg, hw);
    return mc.run(atm, makeGrid(endDate, dt));
}
}  // namespace

TEST(Exposure, EENonNegative) {
    auto res = runAtmSwap(5.0, 1.0 / 12.0, 2000, 1);
    for (auto v : res.EE) EXPECT_GE(v, -1e-9);
    for (auto v : res.ENE) EXPECT_LE(v, 1e-9);
}

TEST(Exposure, EEDecaysAtMaturity) {
    auto res = runAtmSwap(3.0, 1.0 / 12.0, 2000, 7);
    EXPECT_LT(res.EE.back(), 1000.0);
}
