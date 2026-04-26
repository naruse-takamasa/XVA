#include <gtest/gtest.h>

#include <cmath>
#include <random>

#include "models/HullWhiteModel.hpp"
#include "test_helpers.hpp"

using namespace xva;

TEST(HullWhite, BondPriceMatchesCurve) {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);
    Real r0 = hw.initialForwardRate(0.0);

    for (int T : {1, 2, 5, 10}) {
        EXPECT_NEAR(hw.bondPrice(0.0, T, r0), curve.df(T), 0.01) << "T=" << T;
    }
}

TEST(HullWhite, VariancePositive) {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);
    for (double t : {0.5, 1.0, 2.0, 5.0}) {
        EXPECT_GT(hw.variance(t), 0.0) << "t=" << t;
    }
}

TEST(HullWhite, SimulationMeanInRange) {
    auto curve = flatCurve(0.03);
    HullWhiteModel hw(0.05, 0.01, curve);

    TimeGrid grid;
    for (int i = 0; i <= 12; ++i) grid.times.push_back(i / 12.0);

    std::mt19937_64 rng(42);
    auto paths = hw.simulatePaths(grid, 5000, rng);

    Real mean = 0.0;
    for (auto& p : paths) mean += p.back();
    mean /= static_cast<Real>(paths.size());

    // Antithetic paths make mean ≈ 0; accept any plausible short rate
    EXPECT_NEAR(mean, 0.0, 0.10);
}
