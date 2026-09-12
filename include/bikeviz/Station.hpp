#pragma once

#include <string>

namespace bikeviz {

// A single bike-share dock station, mirroring the columns of the
// Chicago Divvy bike station export used by the original coursework:
// Station ID, Station Name, Address, Total Docks, Docks in Service,
// Status, Latitude, Longitude.
struct Station {
    std::string id;
    std::string name;
    std::string address;
    int totalDocks = 0;
    int docksInService = 0;
    std::string status;
    double latitude = 0.0;
    double longitude = 0.0;

    bool isInService() const { return status == "In Service"; }
};

}  // namespace bikeviz
