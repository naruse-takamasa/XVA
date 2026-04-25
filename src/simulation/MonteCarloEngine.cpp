#include "simulation/MonteCarloEngine.hpp"
#include "utils/Logger.hpp"
#include <thread>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace xva {

MonteCarloEngine::MonteCarloEngine(const MCConfig& config,
                                   const HullWhiteModel& model)
    : config_(config), model_(model) {}

// ── Single-threaded block simulation ─────────────────────────────────────────
void MonteCarloEngine::simulateBlock(
    std::size_t pathStart,
    std::size_t pathEnd,
    const std::vector<InterestRateSwap>& swaps,
    const TimeGrid& grid,
    Matrix& pathValues,
    unsigned seed) const
{
    std::mt19937_64 rng(seed);
    std::size_t N = grid.size();

    // Simulate short-rate paths for this block
    TimeGrid blockGrid = grid;
    std::size_t blockSize = pathEnd - pathStart;

    Matrix rPaths = model_.simulatePaths(grid, blockSize, rng);

    for (std::size_t p = 0; p < blockSize; ++p) {
        for (std::size_t i = 0; i < N; ++i) {
            Real t  = grid[i];
            Real rt = rPaths[p][i];
            Real portNPV = 0.0;
            for (auto& swap : swaps)
                portNPV += swap.npv(t, rt, model_);
            pathValues[pathStart + p][i] = portNPV;
        }
    }
}

// ── Main simulation runner ────────────────────────────────────────────────────
SimulationResult MonteCarloEngine::runPortfolio(
    const std::vector<InterestRateSwap>& swaps,
    const TimeGrid& grid) const
{
    LOG_INFO("MC simulation: ", config_.numPaths, " paths, ",
             grid.size(), " time steps, ",
             config_.numThreads, " threads");

    SimulationResult result;
    result.timeGrid  = grid;
    result.numPaths  = config_.numPaths;
    result.pathValues.assign(config_.numPaths,
                             std::vector<Real>(grid.size(), 0.0));

    // Divide paths across threads
    std::size_t numPaths  = config_.numPaths;
    int         numT      = std::max(1, config_.numThreads);
    std::size_t blockSize = (numPaths + numT - 1) / numT;

    std::vector<std::thread> threads;
    for (int t = 0; t < numT; ++t) {
        std::size_t start = t * blockSize;
        std::size_t end   = std::min(start + blockSize, numPaths);
        if (start >= end) break;
        unsigned seed = static_cast<unsigned>(config_.seed + t * 1000000);
        threads.emplace_back([&, start, end, seed]() {
            simulateBlock(start, end, swaps, grid,
                          result.pathValues, seed);
        });
    }
    for (auto& th : threads) th.join();

    // Antithetic variance reduction: mirror half the paths
    if (config_.useAntithetic && numPaths >= 2) {
        std::size_t half = numPaths / 2;
        for (std::size_t p = 0; p < half; ++p)
            for (std::size_t i = 0; i < grid.size(); ++i)
                result.pathValues[half + p][i] = -result.pathValues[p][i];
        // Re-run second half would break symmetry; instead we flip sign
        // (valid for portfolios symmetric in short rate – approximate)
    }

    computeExposureProfiles(result);

    LOG_INFO("Simulation complete.");
    return result;
}

SimulationResult MonteCarloEngine::run(const InterestRateSwap& swap,
                                       const TimeGrid& grid) const {
    return runPortfolio({swap}, grid);
}

// ── Exposure profiles ─────────────────────────────────────────────────────────
void MonteCarloEngine::computeExposureProfiles(SimulationResult& result) const {
    std::size_t N = result.timeGrid.size();
    std::size_t P = result.numPaths;

    result.EE .resize(N, 0.0);
    result.ENE.resize(N, 0.0);
    result.PFE.resize(N, 0.0);

    std::vector<Real> col(P);

    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t p = 0; p < P; ++p)
            col[p] = result.pathValues[p][i];

        // EE = E[max(V,0)]
        Real ee = 0.0, ene = 0.0;
        for (Real v : col) {
            ee  += std::max(v, 0.0);
            ene += std::min(v, 0.0);
        }
        result.EE [i] = ee  / P;
        result.ENE[i] = ene / P;

        // PFE @ 95%
        std::vector<Real> pos;
        for (Real v : col) if (v > 0) pos.push_back(v);
        if (!pos.empty()) {
            std::sort(pos.begin(), pos.end());
            result.PFE[i] = pos[static_cast<std::size_t>(0.95 * pos.size())];
        }
    }

    // EEPE = time-average of EE over [0, 1yr]
    Real eepe = 0.0;
    Real tEnd = std::min(result.timeGrid.times.back(), 1.0);
    int  cnt  = 0;
    for (std::size_t i = 0; i < N && result.timeGrid[i] <= tEnd; ++i) {
        eepe += result.EE[i];
        ++cnt;
    }
    result.EEPE.assign(1, cnt > 0 ? eepe / cnt : 0.0);
}

} // namespace xva
