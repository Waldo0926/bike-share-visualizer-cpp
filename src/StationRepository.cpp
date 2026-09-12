#include "bikeviz/StationRepository.hpp"

#include <algorithm>
#include <cctype>

namespace bikeviz {

namespace {

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return result;
}

}  // namespace

StationRepository::StationRepository(std::vector<Station> stations)
    : stations_(std::move(stations)) {}

std::optional<Station> StationRepository::findById(const std::string& id) const {
    auto it = std::find_if(stations_.begin(), stations_.end(),
                            [&](const Station& s) { return s.id == id; });
    if (it == stations_.end()) return std::nullopt;
    return *it;
}

std::optional<Station> StationRepository::findByName(const std::string& name) const {
    const std::string target = toLower(name);
    auto it = std::find_if(stations_.begin(), stations_.end(), [&](const Station& s) {
        return toLower(s.name) == target;
    });
    if (it == stations_.end()) return std::nullopt;
    return *it;
}

StationSummary StationRepository::summary() const {
    StationSummary summary;
    summary.totalStations = static_cast<int>(stations_.size());
    for (const Station& s : stations_) {
        summary.totalDocks += s.totalDocks;
        summary.docksInService += s.docksInService;
        if (s.isInService()) {
            ++summary.inServiceStations;
        }
    }
    summary.notInServiceStations = summary.totalStations - summary.inServiceStations;
    return summary;
}

}  // namespace bikeviz
