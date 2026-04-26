#include <gtest/gtest.h>

#include "instruments/InterestRateSwap.hpp"
#include "models/HullWhiteModel.hpp"
#include "simulation/MonteCarloEngine.hpp"
#include "test_helpers.hpp"
#include "utils/DateUtils.hpp"
#include "xva/XVAAggregator.hpp"

using namespace xva;

TEST(XVA, CVANegativeDVAPositiveMVANegative) {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);

    SwapSchedule s;
    s.startDate = 0;
    s.endDate = 5;
    s.notional = 1e6;
    s.fixedFreq = 1;
    s.floatFreq = 2;
    s.dayCountConvention = "ACT365";
    InterestRateSwap tmp(s, true);
    s.fixedRate = tmp.parRate(curve);
    InterestRateSwap atm(s, true);

    MCConfig cfg;
    cfg.numPaths = 3000;
    cfg.numThreads = 2;
    cfg.seed = 42;
    MonteCarloEngine mc(cfg, hw);
    auto res = mc.run(atm, makeGrid(5.0, 1.0 / 12.0));

    XVAConfig xcfg;
    xcfg.counterpartyCurve = flatCreditCurve("CP", 0.01);
    xcfg.ownCurve = flatCreditCurve("Self", 0.005);
    xcfg.counterpartyRecovery = 0.4;
    xcfg.ownRecovery = 0.4;
    xcfg.fundingBorrowSpread = 0.005;
    xcfg.fundingLendSpread = 0.002;
    xcfg.imFundingSpread = 0.003;
    xcfg.imMultiplier = 1.4;

    XVAResult r = XVAAggregator(xcfg).compute(res, curve);

    EXPECT_LT(r.CVA, 0.0);  // credit cost is negative
    EXPECT_GT(r.DVA, 0.0);  // own default benefit is positive
    EXPECT_LT(r.MVA, 0.0);  // margin funding cost is negative
}
