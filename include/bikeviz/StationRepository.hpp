#pragma once

#include <optional>
#include <string>
#include <vector>

#include "bikeviz/Station.hpp"

namespace bikeviz {

// Aggregate counts over a set of stations, equivalent to the five
// dialog fields the original program filled in after importing a CSV
// (in-service / not-in-service station and dock counts, plus the total).
struct StationSummary {
    int totalStations = 0;
    int inServiceStations = 0;
    int notInServiceStations = 0;
    int totalDocks = 0;
    int docksInService = 0;
};

// In-memory lookup over a loaded station list. The original program
// re-implemented "find by ID" and "find by name" as two separate,
// near-duplicate loops directly inside two different button handlers;
// this class gives both a single, tested home.
class StationRepository {
public:
    explicit StationRepository(std::vector<Station> stations);

    const std::vector<Station>& all() const { return stations_; }
    std::size_t size() const { return stations_.size(); }

    std::optional<Station> findById(const std::string& id) const;

    // Case-insensitive exact match on station name.
    std::optional<Station> findByName(const std::string& name) const;

    StationSummary summary() const;

private:
    std::vector<Station> stations_;
};

}  // namespace bikeviz
