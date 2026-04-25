cmake_minimum_required(VERSION 3.16)
project(XVACalculator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# Find packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# ── Library sources ──────────────────────────────────────────────────────────
set(LIB_SOURCES
    src/utils/Common.cpp
    src/models/HullWhiteModel.cpp
    src/instruments/InterestRateSwap.cpp
    src/simulation/MonteCarloEngine.cpp
    src/simulation/ExposureCalculator.cpp
    src/xva/CVACalculator.cpp
    src/xva/DVACalculator.cpp
    src/xva/FVACalculator.cpp
    src/xva/MVACalculator.cpp
    src/xva/XVAAggregator.cpp
    src/utils/DateUtils.cpp
    src/utils/MathUtils.cpp
    src/utils/Logger.cpp
)

add_library(xva_lib STATIC ${LIB_SOURCES})
target_link_libraries(xva_lib Threads::Threads)

# ── Main executable ──────────────────────────────────────────────────────────
add_executable(xva_calculator src/main.cpp)
target_link_libraries(xva_calculator xva_lib)

# ── Tests ────────────────────────────────────────────────────────────────────
enable_testing()

add_executable(test_irs        tests/test_irs.cpp)
add_executable(test_hw_model   tests/test_hw_model.cpp)
add_executable(test_exposure   tests/test_exposure.cpp)
add_executable(test_xva        tests/test_xva.cpp)

foreach(test_target test_irs test_hw_model test_exposure test_xva)
    target_link_libraries(${test_target} xva_lib)
    add_test(NAME ${test_target} COMMAND ${test_target})
endforeach()

# ── Install ──────────────────────────────────────────────────────────────────
install(TARGETS xva_lib DESTINATION lib)
install(TARGETS xva_calculator DESTINATION bin)
install(DIRECTORY include/ DESTINATION include/xva)

# ── Packaging (CPack) ─────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME "xva")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "TGZ")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
