#pragma once
#include <cmath>
#include <string>

#include "Common.hpp"

namespace xva {

inline YieldCurve flatCurve(double r, int n = 30) {
    YieldCurve c;
    for (int i = 0; i <= n; ++i) {
        c.tenors.push_back(i);
        c.discountFactors.push_back(std::exp(-r * i));
    }
    return c;
}

inline CreditCurve flatCreditCurve(const std::string& name, double hazardRate, int yrs = 30) {
    CreditCurve cc;
    cc.name = name;
    for (int i = 0; i <= yrs; ++i) {
        cc.tenors.push_back(i);
        cc.survivalProbs.push_back(std::exp(-hazardRate * i));
    }
    return cc;
}

}  // namespace xva
