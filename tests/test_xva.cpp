#include "xva/XVAAggregator.hpp"
#include "simulation/MonteCarloEngine.hpp"
#include "instruments/InterestRateSwap.hpp"
#include "models/HullWhiteModel.hpp"
#include "utils/DateUtils.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace xva;

YieldCurve flatCurve(double r, int n=30) {
    YieldCurve c;
    for (int i = 0; i <= n; ++i) {
        c.tenors.push_back(i); c.discountFactors.push_back(std::exp(-r*i));
    }
    return c;
}

CreditCurve flatCC(const std::string& n, double h, int yrs=30) {
    CreditCurve cc; cc.name=n;
    for (int i = 0; i <= yrs; ++i) {
        cc.tenors.push_back(i);
        cc.survivalProbs.push_back(std::exp(-h*i));
    }
    return cc;
}

void test_cva_negative() {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);

    SwapSchedule s;
    s.startDate=0; s.endDate=5; s.notional=1e6;
    s.fixedFreq=1; s.floatFreq=2; s.dayCountConvention="ACT365";
    InterestRateSwap tmp(s,true);
    s.fixedRate = tmp.parRate(curve);
    InterestRateSwap atm(s,true);

    MCConfig cfg; cfg.numPaths=3000; cfg.numThreads=2; cfg.seed=42;
    MonteCarloEngine mc(cfg, hw);
    auto grid = makeGrid(5.0, 1.0/12.0);
    auto res  = mc.run(atm, grid);

    XVAConfig xcfg;
    xcfg.counterpartyCurve  = flatCC("CP", 0.01);
    xcfg.ownCurve           = flatCC("Self", 0.005);
    xcfg.counterpartyRecovery = 0.4;
    xcfg.ownRecovery          = 0.4;
    xcfg.fundingBorrowSpread  = 0.005;
    xcfg.fundingLendSpread    = 0.002;
    xcfg.imFundingSpread      = 0.003;
    xcfg.imMultiplier         = 1.4;

    XVAAggregator agg(xcfg);
    XVAResult r = agg.compute(res, curve);

    std::cout << "CVA=" << r.CVA << " DVA=" << r.DVA
              << " FVA=" << r.FVA << " MVA=" << r.MVA << "\n";

    // CVA should be negative (cost), DVA positive (benefit)
    assert(r.CVA < 0);
    assert(r.DVA > 0);
    assert(r.MVA < 0);

    std::cout << "PASS: test_cva_negative\n";
}

int main() {
    test_cva_negative();
    std::cout << "\nAll XVA tests passed!\n";
    return 0;
}
