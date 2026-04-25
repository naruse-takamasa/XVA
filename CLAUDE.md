# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure and build (Release)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4

# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4

# Run main executable
./build/xva_calculator

# Run all tests
cd build && ctest --output-on-failure

# Run a single test executable
./build/test_irs
./build/test_hw_model
./build/test_exposure
./build/test_xva
```

No external dependencies — only C++17 standard library is required.

## Architecture Overview

This is a C++17 XVA (Valuation Adjustments) pricing engine for Interest Rate Swaps using a Hull-White 1-factor short-rate model. The data flow is:

**Market Data → HW Model → MC Simulation → Exposure Profiles → XVA Components**

### Layer Structure

1. **Market Data** (`include/Common.hpp`): Core types — `YieldCurve`, `CreditCurve`, `TimeGrid`, `SimulationResult`, `XVAResult`. All dates are `double` (years from today). Matrix is `vector<vector<double>>` indexed as `[path][timeStep]`.

2. **Model** (`include/models/HullWhiteModel.hpp`): Hull-White 1-factor SDE `dr = [θ(t) - a·r]dt + σ·dW`. Provides analytical bond prices via `P(t,T) = exp(A(t,T) - B(t,T)·r(t))` and Euler-Maruyama path simulation.

3. **Instrument** (`include/instruments/InterestRateSwap.hpp`): Vanilla payer/receiver IRS. NPV is computed analytically using HW bond prices at each simulation step via `npv(t, rt, model)`. `parRate(curve)` computes the at-the-money rate.

4. **Simulation** (`include/simulation/MonteCarloEngine.hpp`): Splits paths across `std::thread` workers. Uses antithetic variance reduction by default. Produces a `SimulationResult` with per-path NPV matrices and exposure profiles.

5. **Exposure** (`include/simulation/ExposureCalculator.hpp`): Post-simulation statistics — EE (expected exposure), ENE (expected negative exposure), PFE (95th percentile), EEPE (effective EPE for regulatory capital). Also computes SIMM-proxy Initial Margin.

6. **XVA** (`include/xva/`): Four independent calculators:
   - `CVACalculator`: `-(1-R_c) · Σ EE(tᵢ) · ΔQ_c(tᵢ) · D(tᵢ)`
   - `DVACalculator`: `(1-R_o) · Σ |ENE(tᵢ)| · ΔQ_o(tᵢ) · D(tᵢ)`
   - `FVACalculator`: FCA (funding cost on EE) + FBA (funding benefit on ENE)
   - `MVACalculator`: Cost of posting SIMM-proxy IM over the trade life
   - `XVAAggregator`: Orchestrates all four; configure via `XVAConfig` to enable/disable components.

### Key Design Decisions

- Analytical HW bond pricing is used inside the simulation hot path (not numerical) for speed.
- Each MC thread owns its own RNG seed to avoid contention; antithetic paths are generated within each thread.
- `YieldCurve::df()` and `CreditCurve::survProb()` interpolate lazily — no pre-computed grids.
- `SimulationResult` pre-allocates all path matrices before simulation starts.

### Test Coverage

| Test file | What it validates |
|-----------|-------------------|
| `tests/test_irs.cpp` | IRS NPV, par rate, payer/receiver symmetry |
| `tests/test_hw_model.cpp` | HW bond prices, path generation |
| `tests/test_exposure.cpp` | EE, PFE, IM computation |
| `tests/test_xva.cpp` | CVA, DVA, FVA, MVA values |
