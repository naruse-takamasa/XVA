#include <cassert>
#include <cmath>
#include <iostream>

#include "instruments/InterestRateSwap.hpp"
#include "models/HullWhiteModel.hpp"

using namespace xva;

YieldCurve flatCurve(double r, int n = 30) {
    YieldCurve c;
    for (int i = 0; i <= n; ++i) {
        c.tenors.push_back(i);
        c.discountFactors.push_back(std::exp(-r * i));
    }
    return c;
}

void test_par_swap_zero_npv() {
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

    // At t=0, r=initial forward rate, NPV should be ~0
    Real r0 = hw.initialForwardRate(0.0);
    Real npv = atm.npv(0.0, r0, hw);
    std::cout << "ATM swap NPV at t=0: " << npv << " (should be ~0)\n";
    assert(std::abs(npv) < 1000.0);  // within $1000 on $1M notional
    std::cout << "PASS: test_par_swap_zero_npv\n";
}

void test_payer_receiver_symmetry() {
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
    Real npv_pay = payer.npv(0.0, r0, hw);
    Real npv_rec = receiver.npv(0.0, r0, hw);

    std::cout << "Payer NPV: " << npv_pay << ", Receiver NPV: " << npv_rec << "\n";
    assert(std::abs(npv_pay + npv_rec) < 1e-6);
    std::cout << "PASS: test_payer_receiver_symmetry\n";
}

int main() {
    test_par_swap_zero_npv();
    test_payer_receiver_symmetry();
    std::cout << "\nAll IRS tests passed!\n";
    return 0;
}
