#pragma once
#include "Common.hpp"
#include <string>

namespace xva {

// Build a uniform time grid from 0 to T with N steps
inline TimeGrid makeUniformGrid(Date T, std::size_t N) {
    TimeGrid g;
    g.times.resize(N + 1);
    for (std::size_t i = 0; i <= N; ++i)
        g.times[i] = T * i / N;
    return g;
}

// Build a grid with given step size dt
inline TimeGrid makeGrid(Date T, Date dt) {
    TimeGrid g;
    for (Date t = 0.0; t <= T + 1e-9; t += dt)
        g.times.push_back(t);
    return g;
}

} // namespace xva
