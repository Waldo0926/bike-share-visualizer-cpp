#include "bikeviz/TripSimulator.hpp"

#include <cmath>

namespace bikeviz {

std::optional<TripEndpoints> pickRandomTripEndpoints(
    const StationRepository& repository,
    const MapProjection& projection,
    const RandomIndexFn& randomIndex,
    int minPixelSeparation,
    int maxDiagonalSkew,
    int maxAttempts) {
    const auto& stations = repository.all();
    if (stations.size() < 2) return std::nullopt;

    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        std::size_t i = randomIndex(stations.size());
        std::size_t j = randomIndex(stations.size());
        if (i == j) continue;

        const PixelPoint a = projection.toPixel(stations[i].latitude, stations[i].longitude);
        const PixelPoint b = projection.toPixel(stations[j].latitude, stations[j].longitude);

        const int dx = a.x - b.x;
        const int dy = a.y - b.y;

        // Same visual-separation heuristic the original used before
        // animating a route between two points: reject pairs that are
        // too close together, or whose horizontal/vertical spread is too
        // lopsided to look like a real diagonal trip on the map.
        if (std::abs(dy - dx) < maxDiagonalSkew && std::abs(dy) > minPixelSeparation &&
            std::abs(dx) > minPixelSeparation) {
            return TripEndpoints{stations[i], stations[j]};
        }
    }
    return std::nullopt;
}

double estimateTripDistance(const Station& start,
                             const Station& end,
                             const MapProjection& projection,
                             double unitDivisor) {
    // The original computed this inline with the same two literals its
    // drawing code used (1460 and 1960), so the distance metric and the
    // map calibration were silently coupled by copy-paste. Here the
    // scale factors come from the projection itself, which means
    // recalibrating the map cannot leave the distance metric behind.
    const double dLon = std::fabs(start.longitude - end.longitude) * projection.scaleX();
    const double dLat = std::fabs(start.latitude - end.latitude) * projection.scaleY();
    return (dLon + dLat) / unitDivisor;
}

}  // namespace bikeviz
