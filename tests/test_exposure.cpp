#include "simulation/MonteCarloEngine.hpp"
#include "simulation/ExposureCalculator.hpp"
#include "instruments/InterestRateSwap.hpp"
#include "models/HullWhiteModel.hpp"
#include "utils/DateUtils.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace xva;

YieldCurve flatCurve(double r, int n=30) {
    YieldCurve c;
    for (int i = 0; i <= n; ++i) {
        c.tenors.push_back(i); c.discountFactors.push_back(std::exp(-r*i));
    }
    return c;
}

void test_ee_nonnegative() {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);

    SwapSchedule s;
    s.startDate=0; s.endDate=5; s.notional=1e6;
    s.fixedFreq=1; s.floatFreq=2; s.dayCountConvention="ACT365";
    InterestRateSwap tmp(s,true);
    s.fixedRate = tmp.parRate(curve);
    InterestRateSwap atm(s,true);

    MCConfig cfg; cfg.numPaths=2000; cfg.numThreads=2; cfg.seed=1;
    MonteCarloEngine mc(cfg, hw);
    auto grid = makeGrid(5.0, 1.0/12.0);
    auto res  = mc.run(atm, grid);

    for (auto v : res.EE)  assert(v >= -1e-9);
    for (auto v : res.ENE) assert(v <=  1e-9);
    std::cout << "PASS: test_ee_nonnegative\n";
}

void test_ee_decays_at_maturity() {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);

    SwapSchedule s;
    s.startDate=0; s.endDate=3; s.notional=1e6;
    s.fixedFreq=1; s.floatFreq=2; s.dayCountConvention="ACT365";
    InterestRateSwap tmp(s,true);
    s.fixedRate = tmp.parRate(curve);
    InterestRateSwap atm(s,true);

    MCConfig cfg; cfg.numPaths=2000; cfg.numThreads=2; cfg.seed=7;
    MonteCarloEngine mc(cfg, hw);
    auto grid = makeGrid(3.0, 1.0/12.0);
    auto res  = mc.run(atm, grid);

    // EE at maturity should be ~0
    assert(res.EE.back() < 1000.0);
    std::cout << "PASS: test_ee_decays_at_maturity (EE(T)=" << res.EE.back() << ")\n";
}

int main() {
    test_ee_nonnegative();
    test_ee_decays_at_maturity();
    std::cout << "\nAll exposure tests passed!\n";
    return 0;
}
