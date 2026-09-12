#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "bikeviz/BitmapImage.hpp"
#include "bikeviz/CsvStationLoader.hpp"
#include "bikeviz/MapProjection.hpp"
#include "bikeviz/StationRepository.hpp"
#include "bikeviz/TripSimulator.hpp"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void requireNear(double actual, double expected, double tolerance, const std::string& context) {
    require(std::fabs(actual - expected) <= tolerance,
            context + ": expected ~" + std::to_string(expected) + ", got " + std::to_string(actual));
}

// Uses std::filesystem rather than a hardcoded "/tmp" so the suite also
// runs on Windows, where TMPDIR does not exist and "/tmp" is not a
// writable path.
std::string tempPath(const std::string& name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

std::string writeTempFile(const std::string& name, const std::string& content) {
    const std::string path = tempPath(name);
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("could not create temp file at " + path);
    }
    out << content;
    out.close();
    return path;
}

// ---- splitCsvLine -----------------------------------------------------

void testSplitCsvLineHandlesQuotedCommas() {
    const std::vector<std::string> fields =
        bikeviz::splitCsvLine(R"csv(246,Ashland Ave & Belle Plaine Ave,Ashland Ave,11,11,In Service,41.956057,-87.668835,"(41.956057, -87.668835)")csv");
    require(fields.size() == 9, "expected 9 fields, got " + std::to_string(fields.size()));
    require(fields[0] == "246", "field 0 should be the station id");
    require(fields[8] == "(41.956057, -87.668835)", "quoted field should keep its embedded comma");
}

// ---- CsvStationLoader ---------------------------------------------------

void testLoaderSkipsHeaderRow() {
    const std::string csv =
        "Station ID,Station Name,Address,Total Docks,Docks in Service,Status,Latitude,Longitude\n"
        "246,Ashland Ave,Ashland Ave,11,11,In Service,41.956057,-87.668835\n"
        "514,Ridge Blvd,Ridge Blvd,11,9,Not In Service,42.019276,-87.68452\n";
    const std::string path = writeTempFile("bikeviz_header_test.csv", csv);

    bikeviz::CsvLoadResult result = bikeviz::loadStationsFromCsv(path);
    require(result.stations.size() == 2, "header row must not be counted as a station");
    require(!result.warnings.empty(), "skipping the header should be reported as a warning");
    require(result.stations[0].id == "246", "first parsed station should be id 246");
    require(result.stations[1].status == "Not In Service", "status column should parse verbatim");
    std::remove(path.c_str());
}

void testLoaderSkipsMalformedRows() {
    const std::string csv =
        "1,Station A,Addr A,10,10,In Service,41.0,-87.0\n"
        "not-a-number,Station B,Addr B,oops,5,In Service,41.1,-87.1\n"
        "3,Station C,Addr C,5,5,In Service,41.2,-87.2\n"
        "4,Too,Few,Columns\n";
    const std::string path = writeTempFile("bikeviz_malformed_test.csv", csv);

    bikeviz::CsvLoadResult result = bikeviz::loadStationsFromCsv(path);
    require(result.stations.size() == 2, "malformed rows must be skipped, not crash the loader");
    require(result.warnings.size() == 2, "each skipped row should produce exactly one warning");
    std::remove(path.c_str());
}

void testLoaderReportsMissingFile() {
    bikeviz::CsvLoadResult result = bikeviz::loadStationsFromCsv(tempPath("definitely-not-here.csv"));
    require(result.stations.empty(), "missing file should yield no stations");
    require(!result.warnings.empty(), "missing file should be reported as a warning, not thrown");
    require(!result.fileOpened, "fileOpened must be false when the path cannot be read");
}

void testLoaderDistinguishesEmptyFileFromMissingFile() {
    // A readable file with no usable rows is a different failure from a
    // path that does not exist, and callers need to tell them apart.
    const std::string path = writeTempFile("bikeviz_empty_test.csv", "\n\n");
    bikeviz::CsvLoadResult result = bikeviz::loadStationsFromCsv(path);
    require(result.fileOpened, "fileOpened must be true for a readable but empty file");
    require(result.stations.empty(), "an empty file should yield no stations");
    std::remove(path.c_str());
}

// ---- MapProjection ------------------------------------------------------

void testProjectionMatchesOriginalCalibration() {
    bikeviz::MapProjection projection;
    // Reproduces the exact worked values from the original formula:
    // x = (87.8 + longitude) * 1460, y = (42.1 - latitude) * 1960
    const bikeviz::PixelPoint p = projection.toPixel(41.956057, -87.668835);
    const int expectedX = static_cast<int>((87.8 + -87.668835) * 1460.0);
    const int expectedY = static_cast<int>((42.1 - 41.956057) * 1960.0);
    require(p.x == expectedX, "x pixel should match the original affine transform");
    require(p.y == expectedY, "y pixel should match the original affine transform");
}

void testProjectionIsConfigurable() {
    bikeviz::MapProjection projection(0.0, 0.0, 100.0, 100.0);
    const bikeviz::PixelPoint origin = projection.toPixel(0.0, 0.0);
    require(origin.x == 0 && origin.y == 0, "origin should map to pixel (0, 0)");
    const bikeviz::PixelPoint moved = projection.toPixel(-1.0, 1.0);
    require(moved.x == 100 && moved.y == 100, "custom scale should be honoured");
}

// ---- StationRepository ----------------------------------------------------

std::vector<bikeviz::Station> sampleStations() {
    return {
        {"1", "Alpha", "Addr 1", 10, 10, "In Service", 41.0, -87.0},
        {"2", "Beta", "Addr 2", 8, 4, "Not In Service", 41.1, -87.1},
        {"3", "Gamma", "Addr 3", 12, 12, "In Service", 41.2, -87.2},
    };
}

void testFindByIdAndName() {
    bikeviz::StationRepository repo(sampleStations());
    require(repo.findById("2")->name == "Beta", "findById should return the matching station");
    require(!repo.findById("missing").has_value(), "findById should return nullopt for unknown id");
    require(repo.findByName("gamma")->id == "3", "findByName should be case-insensitive");
    require(!repo.findByName("delta").has_value(), "findByName should return nullopt for unknown name");
}

void testSummaryCounts() {
    bikeviz::StationRepository repo(sampleStations());
    bikeviz::StationSummary summary = repo.summary();
    require(summary.totalStations == 3, "total station count");
    require(summary.inServiceStations == 2, "in-service station count");
    require(summary.notInServiceStations == 1, "not-in-service station count");
    require(summary.totalDocks == 30, "total docks sum");
    require(summary.docksInService == 26, "docks-in-service sum");
}

// ---- TripSimulator ---------------------------------------------------------

void testTripDistanceMatchesOriginalFormula() {
    bikeviz::Station a{"1", "A", "", 0, 0, "In Service", 41.956057, -87.668835};
    bikeviz::Station b{"2", "B", "", 0, 0, "In Service", 42.019276, -87.68452};
    bikeviz::MapProjection projection;
    const double distance = bikeviz::estimateTripDistance(a, b, projection);
    const double expected =
        (std::fabs(-87.668835 - -87.68452) * 1460.0 + std::fabs(41.956057 - 42.019276) * 1960.0) / 5.0;
    requireNear(distance, expected, 1e-6, "trip distance formula");
}

void testTripDistanceFollowsProjectionScale() {
    // The distance metric must be derived from the projection's scale
    // factors, not from its own copy of the two magic numbers.
    bikeviz::Station a{"1", "A", "", 0, 0, "In Service", 0.0, 0.0};
    bikeviz::Station b{"2", "B", "", 0, 0, "In Service", 1.0, 1.0};
    bikeviz::MapProjection doubled(0.0, 0.0, 2.0, 4.0);
    const double distance = bikeviz::estimateTripDistance(a, b, doubled, /*unitDivisor=*/1.0);
    requireNear(distance, 6.0, 1e-9, "distance should use the projection's own scale factors");
}

void testTripSimulatorRespectsSeparationHeuristic() {
    // Station "1" sits exactly at the projection's origin (pixel 0,0).
    // Station "2" sits at pixel (150, 140): both axes are more than
    // minPixelSeparation (100) apart, and the diagonal skew |dy-dx| is
    // only 10, comfortably under the default threshold of 30. Station
    // "1b" sits one pixel away from "1" and must always be rejected.
    std::vector<bikeviz::Station> stations = {
        {"1", "Origin-A", "", 0, 0, "In Service", 42.1, -87.8},
        {"1b", "Too-close", "", 0, 0, "In Service", 42.0995, -87.7995},
        {"2", "Diagonal-B", "", 0, 0, "In Service", 42.02857142857143, -87.6972602739726},
    };
    bikeviz::StationRepository repo(stations);
    bikeviz::MapProjection projection;

    // Deterministic "random" source that always returns station 0 then 2.
    int call = 0;
    bikeviz::RandomIndexFn fixedIndex = [&](std::size_t) -> std::size_t {
        const std::size_t values[] = {0, 2};
        return values[call++ % 2];
    };

    auto trip = bikeviz::pickRandomTripEndpoints(repo, projection, fixedIndex);
    require(trip.has_value(), "a suitable pair should be found");
    require(trip->start.id == "1" && trip->end.id == "2", "should pick the fixed indices");
}

void testTripSimulatorGivesUpOnImpossibleDataset() {
    // Every station sits at the exact same coordinate, so no pair can
    // ever satisfy the minimum-separation heuristic. The simulator must
    // return nullopt rather than looping forever.
    std::vector<bikeviz::Station> stations = {
        {"1", "A", "", 0, 0, "In Service", 41.0, -87.0},
        {"2", "B", "", 0, 0, "In Service", 41.0, -87.0},
    };
    bikeviz::StationRepository repo(stations);
    bikeviz::MapProjection projection;
    std::size_t counter = 0;
    bikeviz::RandomIndexFn alwaysFirstThenSecond = [&counter](std::size_t upperBound) -> std::size_t {
        return (counter++) % upperBound;
    };

    auto trip = bikeviz::pickRandomTripEndpoints(repo, projection, alwaysFirstThenSecond,
                                                  /*minPixelSeparation=*/100, /*maxDiagonalSkew=*/30,
                                                  /*maxAttempts=*/50);
    require(!trip.has_value(), "impossible dataset should report failure instead of hanging");
}

// ---- BitmapImage -----------------------------------------------------------

void testBitmapRoundTrip() {
    bikeviz::BitmapImage image(4, 3);
    image.drawMarker(1, 1, bikeviz::RgbColor{200, 20, 20}, /*radius=*/0);

    const std::string path = tempPath("bikeviz_test_output.bmp");
    image.save(path);

    bikeviz::BitmapImage reloaded = bikeviz::BitmapImage::loadFromFile(path);
    require(reloaded.width() == 4 && reloaded.height() == 3, "round-tripped bitmap should keep its size");
    const bikeviz::RgbColor pixel = reloaded.pixelAt(1, 1);
    require(pixel.r == 200 && pixel.g == 20 && pixel.b == 20, "round-tripped marker colour should survive save/load");
    std::remove(path.c_str());
}

void testBitmapClipsOutOfBoundsMarkers() {
    bikeviz::BitmapImage image(4, 3);
    // Should not throw or corrupt memory even though this is off-canvas.
    image.drawMarker(-5, 100, bikeviz::RgbColor{1, 2, 3}, /*radius=*/3);
    require(image.width() == 4 && image.height() == 3, "out-of-bounds marker must be clipped, not crash");
}

// ---- Integration tests against the real bundled dataset -------------------
//
// These run on the actual 588-station Chicago CSV and the original
// coursework's map image, so the whole pipeline is exercised on real
// input on every platform CI builds for. The expected numbers are the
// ones the original MFC program displayed in its statistics panel
// (9835 docks in service, 416 out of service), minus the phantom
// "station" it created from the CSV header row.

void testRealDatasetLoadsExpectedTotals() {
    const std::string csv = std::string(BIKEVIZ_DATA_DIR) + "/chicago_bike_stations.csv";
    bikeviz::CsvLoadResult result = bikeviz::loadStationsFromCsv(csv);

    require(result.stations.size() == 588,
            "real dataset should yield exactly 588 stations, got " +
                std::to_string(result.stations.size()));
    require(result.warnings.size() == 1,
            "the only warning should be the skipped header row, got " +
                std::to_string(result.warnings.size()));

    bikeviz::StationRepository repo(std::move(result.stations));
    bikeviz::StationSummary summary = repo.summary();

    require(summary.totalDocks == 10251, "total docks should be 10251");
    require(summary.docksInService == 9835,
            "docks in service should match the original program's 9835");
    require(summary.totalDocks - summary.docksInService == 416,
            "out-of-service docks should match the original program's 416");
    require(summary.inServiceStations == 588, "every station in this export is In Service");
    require(summary.notInServiceStations == 0,
            "the original's single 'not in service' station was the CSV header, not real data");

    // Spot-check a known record end to end.
    auto station = repo.findById("246");
    require(station.has_value(), "station 246 should exist in the real dataset");
    require(station->name == "Ashland Ave & Belle Plaine Ave", "station 246 name");
    require(station->totalDocks == 11, "station 246 dock count");
}

void testRealMapRendersEveryStation() {
    const std::string csv = std::string(BIKEVIZ_DATA_DIR) + "/chicago_bike_stations.csv";
    const std::string mapPath = std::string(BIKEVIZ_DATA_DIR) + "/map.bmp";

    bikeviz::CsvLoadResult loaded = bikeviz::loadStationsFromCsv(csv);
    bikeviz::StationRepository repo(std::move(loaded.stations));
    bikeviz::MapProjection projection;

    bikeviz::BitmapImage image = bikeviz::BitmapImage::loadFromFile(mapPath);
    require(image.width() == 365 && image.height() == 785,
            "the original map image should be 365x785");

    // 587 of the 588 stations project inside the calibrated map area.
    // The one exception is station 398 (Rainbow Beach), the easternmost
    // station in the export, which lands at x = 365 on a 365-pixel-wide
    // image — roughly 50 metres past the right edge of the map the
    // original coursework calibrated against. The original program drew
    // it into a Win32 DC that clipped it away without saying anything;
    // this test documents the discrepancy instead of hiding it.
    int drawn = 0;
    int offMap = 0;
    std::vector<std::string> offMapNames;
    for (const bikeviz::Station& s : repo.all()) {
        const bikeviz::PixelPoint p = projection.toPixel(s.latitude, s.longitude);
        if (image.drawMarker(p.x, p.y, bikeviz::RgbColor{154, 82, 44})) {
            ++drawn;
        } else {
            ++offMap;
            offMapNames.push_back(s.id + " " + s.name);
        }
    }
    require(drawn == 587, "587 stations should project inside the map, got " + std::to_string(drawn));
    require(offMap == 1, "exactly one station should fall outside, got " + std::to_string(offMap));
    require(offMapNames.size() == 1 && offMapNames[0] == "398 Rainbow Beach",
            "the off-map station should be 398 Rainbow Beach");

    const std::string out = tempPath("bikeviz_real_render.bmp");
    image.save(out);
    bikeviz::BitmapImage reloaded = bikeviz::BitmapImage::loadFromFile(out);
    require(reloaded.width() == 365 && reloaded.height() == 785,
            "rendered output should keep the map's dimensions");
    std::remove(out.c_str());
}

}  // namespace

int main() {
    const std::pair<const char*, void (*)()> tests[] = {
        {"splitCsvLine handles quoted commas", testSplitCsvLineHandlesQuotedCommas},
        {"loader skips header row", testLoaderSkipsHeaderRow},
        {"loader skips malformed rows", testLoaderSkipsMalformedRows},
        {"loader reports missing file", testLoaderReportsMissingFile},
        {"loader distinguishes empty file from missing file", testLoaderDistinguishesEmptyFileFromMissingFile},
        {"projection matches original calibration", testProjectionMatchesOriginalCalibration},
        {"projection is configurable", testProjectionIsConfigurable},
        {"findById and findByName", testFindByIdAndName},
        {"summary counts", testSummaryCounts},
        {"trip distance matches original formula", testTripDistanceMatchesOriginalFormula},
        {"trip distance follows projection scale", testTripDistanceFollowsProjectionScale},
        {"trip simulator respects separation heuristic", testTripSimulatorRespectsSeparationHeuristic},
        {"trip simulator gives up on impossible dataset", testTripSimulatorGivesUpOnImpossibleDataset},
        {"bitmap round trip", testBitmapRoundTrip},
        {"bitmap clips out-of-bounds markers", testBitmapClipsOutOfBoundsMarkers},
        {"real dataset loads expected totals", testRealDatasetLoadsExpectedTotals},
        {"real map renders every station", testRealMapRendersEveryStation},
    };

    int failures = 0;
    for (const auto& [name, fn] : tests) {
        try {
            fn();
            std::cout << "[PASS] " << name << "\n";
        } catch (const std::exception& e) {
            std::cout << "[FAIL] " << name << ": " << e.what() << "\n";
            ++failures;
        }
    }

    std::cout << "\n" << (sizeof(tests) / sizeof(tests[0])) << " tests, " << failures << " failed\n";
    return failures == 0 ? 0 : 1;
}
