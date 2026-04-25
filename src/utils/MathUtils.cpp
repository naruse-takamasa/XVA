#include "utils/MathUtils.hpp"
#include <cmath>
#include <stdexcept>

namespace xva::math {

// Beasley-Springer-Moro inverse normal CDF
Real normInvCDF(Real p) {
    if (p <= 0.0 || p >= 1.0)
        throw std::domain_error("normInvCDF: p must be in (0,1)");

    static const Real a[] = {
        2.50662823884, -18.61500062529, 41.39119773534, -25.44106049637
    };
    static const Real b[] = {
        -8.47351093090, 23.08336743743, -21.06224101826, 3.13082909833
    };
    static const Real c[] = {
        0.3374754822726147, 0.9761690190917186, 0.1607979714918209,
        0.0276438810333863, 0.0038405729373609, 0.0003951896511349,
        0.0000321767881768, 0.0000002888167364, 0.0000003960315187
    };

    Real y = p - 0.5;
    if (std::abs(y) < 0.42) {
        Real r = y * y;
        Real x = y * (((a[3]*r + a[2])*r + a[1])*r + a[0]) /
                     ((((b[3]*r + b[2])*r + b[1])*r + b[0])*r + 1.0);
        return x;
    }
    Real r = (y < 0) ? p : 1.0 - p;
    r = std::log(-std::log(r));
    Real x = c[0] + r*(c[1] + r*(c[2] + r*(c[3] + r*(c[4] + r*(c[5] + r*(c[6] + r*(c[7] + r*c[8])))))));
    return (y < 0) ? -x : x;
}

} // namespace xva::math
