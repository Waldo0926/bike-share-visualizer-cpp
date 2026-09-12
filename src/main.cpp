#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "bikeviz/BitmapImage.hpp"
#include "bikeviz/CsvStationLoader.hpp"
#include "bikeviz/MapProjection.hpp"
#include "bikeviz/StationRepository.hpp"
#include "bikeviz/TripSimulator.hpp"

namespace {

void printUsage() {
    std::cout <<
        "bikeviz - city bike-share station CLI\n\n"
        "Usage:\n"
        "  bikeviz --csv <stations.csv> [options]\n\n"
        "Options:\n"
        "  --csv <path>        Station data CSV (required)\n"
        "  --find-id <id>      Print details for a station by exact ID\n"
        "  --find-name <name>  Print details for a station by exact name\n"
        "  --simulate          Simulate a random two-station trip\n"
        "  --map <path>        Source map bitmap (e.g. legacy/assets/map.bmp)\n"
        "  --out <path>        Render all stations onto --map and save here\n";
}

void printStation(const bikeviz::Station& s) {
    std::cout << "  id:               " << s.id << "\n"
              << "  name:             " << s.name << "\n"
              << "  address:          " << s.address << "\n"
              << "  total docks:      " << s.totalDocks << "\n"
              << "  docks in service: " << s.docksInService << "\n"
              << "  status:           " << s.status << "\n"
              << "  latitude:         " << s.latitude << "\n"
              << "  longitude:        " << s.longitude << "\n";
}

int run(int argc, char** argv) {
    std::string csvPath;
    std::optional<std::string> findId;
    std::optional<std::string> findName;
    std::optional<std::string> mapPath;
    std::optional<std::string> outPath;
    bool simulate = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto next = [&]() -> std::string {
            if (i + 1 >= argc) {
                throw std::runtime_error("missing value after " + arg);
            }
            return argv[++i];
        };

        if (arg == "--csv") {
            csvPath = next();
        } else if (arg == "--find-id") {
            findId = next();
        } else if (arg == "--find-name") {
            findName = next();
        } else if (arg == "--map") {
            mapPath = next();
        } else if (arg == "--out") {
            outPath = next();
        } else if (arg == "--simulate") {
            simulate = true;
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            std::cerr << "unknown argument: " << arg << "\n";
            printUsage();
            return 1;
        }
    }

    if (csvPath.empty()) {
        printUsage();
        return 1;
    }

    bikeviz::CsvLoadResult loadResult = bikeviz::loadStationsFromCsv(csvPath);
    for (const std::string& warning : loadResult.warnings) {
        std::cerr << "warning: " << warning << "\n";
    }

    if (!loadResult.fileOpened) {
        std::cerr << "error: could not read " << csvPath << "\n";
        return 1;
    }
    if (loadResult.stations.empty()) {
        std::cerr << "error: " << csvPath << " contained no usable station rows\n";
        return 1;
    }

    bikeviz::StationRepository repository(std::move(loadResult.stations));
    bikeviz::MapProjection projection;

    const bikeviz::StationSummary summary = repository.summary();
    std::cout << "Loaded " << summary.totalStations << " stations\n"
              << "  in service:     " << summary.inServiceStations << "\n"
              << "  not in service: " << summary.notInServiceStations << "\n"
              << "  total docks:    " << summary.totalDocks << "\n"
              << "  docks in service: " << summary.docksInService << "\n";

    if (findId) {
        std::cout << "\nLooking up station by id '" << *findId << "':\n";
        if (auto station = repository.findById(*findId)) {
            printStation(*station);
        } else {
            std::cout << "  not found\n";
        }
    }

    if (findName) {
        std::cout << "\nLooking up station by name '" << *findName << "':\n";
        if (auto station = repository.findByName(*findName)) {
            printStation(*station);
        } else {
            std::cout << "  not found\n";
        }
    }

    if (simulate) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        auto randomIndex = [](std::size_t upperBound) {
            return static_cast<std::size_t>(std::rand()) % upperBound;
        };
        std::cout << "\nSimulating a random trip:\n";
        if (auto trip = bikeviz::pickRandomTripEndpoints(repository, projection, randomIndex)) {
            const double distance = bikeviz::estimateTripDistance(trip->start, trip->end, projection);
            std::cout << "  from: " << trip->start.name << " (" << trip->start.id << ")\n"
                      << "  to:   " << trip->end.name << " (" << trip->end.id << ")\n"
                      << "  estimated distance: " << distance << "\n";
        } else {
            std::cout << "  could not find a suitable pair of stations\n";
        }
    }

    if (mapPath && outPath) {
        std::cout << "\nRendering " << repository.size() << " stations onto " << *mapPath << "...\n";
        bikeviz::BitmapImage image = bikeviz::BitmapImage::loadFromFile(*mapPath);
        int offMap = 0;
        for (const bikeviz::Station& station : repository.all()) {
            const bikeviz::PixelPoint p = projection.toPixel(station.latitude, station.longitude);
            if (!image.drawMarker(p.x, p.y, bikeviz::RgbColor{154, 82, 44})) {
                ++offMap;
                std::cerr << "  note: " << station.name << " (" << station.id
                          << ") projects outside the map area at pixel (" << p.x << ", " << p.y
                          << ")\n";
            }
        }
        image.save(*outPath);
        std::cout << "  saved to " << *outPath << " (" << repository.size() - offMap
                  << " drawn, " << offMap << " outside the map area)\n";
    } else if (mapPath || outPath) {
        std::cerr << "\nnote: rendering needs both --map and --out; skipping the render step\n";
    }

    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
