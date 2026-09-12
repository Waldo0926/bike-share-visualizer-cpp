#pragma once

#include <functional>
#include <optional>

#include "bikeviz/MapProjection.hpp"
#include "bikeviz/Station.hpp"
#include "bikeviz/StationRepository.hpp"

namespace bikeviz {

struct TripEndpoints {
    Station start;
    Station end;
};

// Random-number source abstraction so trip selection is deterministic
// in tests instead of depending on std::rand()/time-seeded state.
using RandomIndexFn = std::function<std::size_t(std::size_t exclusiveUpperBound)>;

// Reproduces the "pick two random stations to simulate a rider's trip"
// feature, with two corrections over the original:
//   1. the original always sampled indices with `rand() % 589`, so it
//      silently misbehaved on any dataset that wasn't exactly 589 rows;
//      this version samples over the actual station count.
//   2. the original's retry loop had no iteration cap, so an unlucky
//      dataset (or a bug in the separation heuristic) could spin
//      forever; this version gives up after a bounded number of
//      attempts and reports failure instead of hanging.
std::optional<TripEndpoints> pickRandomTripEndpoints(
    const StationRepository& repository,
    const MapProjection& projection,
    const RandomIndexFn& randomIndex,
    int minPixelSeparation = 100,
    int maxDiagonalSkew = 30,
    int maxAttempts = 2000);

// The same "Manhattan-style" distance estimate the original used to
// label a simulated trip: each GPS delta scaled into map pixels by the
// projection, summed, then divided by a fixed factor.
//
// Note that this is a grid-distance proxy in scaled map units, not a
// true great-circle or road distance — the original dialog labelled the
// field "KM", which happens to land in a plausible range for Chicago
// but is not a real kilometre conversion. It is kept as-is because
// reproducing the coursework's metric is the point; `unitDivisor` is
// exposed so a caller can recalibrate it rather than edit the formula.
double estimateTripDistance(const Station& start,
                             const Station& end,
                             const MapProjection& projection,
                             double unitDivisor = 5.0);

}  // namespace bikeviz
