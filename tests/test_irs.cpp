#include <gtest/gtest.h>

#include "instruments/InterestRateSwap.hpp"
#include "models/HullWhiteModel.hpp"
#include "test_helpers.hpp"

using namespace xva;

TEST(IRS, ParSwapZeroNPV) {
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

    Real r0 = hw.initialForwardRate(0.0);
    EXPECT_NEAR(atm.npv(0.0, r0, hw), 0.0, 1000.0);  // within $1000 on $1M notional
}

TEST(IRS, PayerReceiverSymmetry) {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);

    SwapSchedule s;
    s.startDate = 0;
    s.endDate = 5;
    s.notional = 1e6;
    s.fixedFreq = 1;
    s.floatFreq = 2;
    s.fixedRate = 0.03;
    s.dayCountConvention = "ACT365";

    InterestRateSwap payer(s, true);
    InterestRateSwap receiver(s, false);
    Real r0 = hw.initialForwardRate(0.0);

    EXPECT_NEAR(payer.npv(0.0, r0, hw) + receiver.npv(0.0, r0, hw), 0.0, 1e-6);
}
