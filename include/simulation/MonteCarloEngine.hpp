#pragma once
#include <functional>
#include <vector>

#include "Common.hpp"
#include "instruments/InterestRateSwap.hpp"
#include "models/HullWhiteModel.hpp"

namespace xva {

struct MCConfig {
    std::size_t numPaths = 10'000;
    std::size_t numTimeSteps = 100;
    std::size_t seed = 42;
    bool useAntithetic = true;  // variance reduction
    bool useQuasiMC = false;    // Sobol (future)
    int numThreads = 4;         // parallel simulation
};

/**
 * Monte Carlo Engine
 *
 * Generates short-rate paths under Hull-White and re-prices the IRS
 * at each time step to produce an exposure profile.
 */
class MonteCarloEngine {
public:
    MonteCarloEngine(const MCConfig& config, const HullWhiteModel& model);

    // Run simulation for a single swap
    SimulationResult run(const InterestRateSwap& swap, const TimeGrid& grid) const;

    // Run simulation for a portfolio of swaps (netting set)
    SimulationResult runPortfolio(const std::vector<InterestRateSwap>& swaps,
                                  const TimeGrid& grid) const;

private:
    MCConfig config_;
    HullWhiteModel model_;

    // Compute EE, ENE, PFE profiles from raw path values
    void computeExposureProfiles(SimulationResult& result) const;

    // Single-threaded simulation block
    void simulateBlock(std::size_t pathStart, std::size_t pathEnd,
                       const std::vector<InterestRateSwap>& swaps, const TimeGrid& grid,
                       Matrix& pathValues, unsigned seed) const;
};

}  // namespace xva
