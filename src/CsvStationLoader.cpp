#include "bikeviz/CsvStationLoader.hpp"

#include <cctype>
#include <fstream>
#include <limits>
#include <sstream>

namespace bikeviz {

namespace {

std::string trim(const std::string& s) {
    std::size_t begin = 0;
    std::size_t end = s.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(s[begin]))) ++begin;
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(begin, end - begin);
}

// Parses `s` as a base-10 integer, accepting the value only if the
// *entire* string was consumed by the conversion.
//
// std::stoi silently ignores trailing garbage after a valid prefix, so
// std::stoi("12.3") returns 12 rather than failing, and std::stoi("12abc")
// returns 12 as well. Checking `pos == s.size()` is what makes this a
// real validator instead of a best-effort prefix parse.
bool parseInt(const std::string& s, int& out) {
    if (s.empty()) return false;
    try {
        std::size_t pos = 0;
        const long value = std::stol(s, &pos);
        if (pos != s.size()) return false;
        if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
            return false;
        }
        out = static_cast<int>(value);
        return true;
    } catch (...) {
        return false;
    }
}

// Same full-consumption discipline for floating-point fields. This is
// what rejects malformed values like "1..2" or "41.9.5" that a naive
// "digits and at most one dot" character check would wave through:
// std::stod("1..2") happily returns 1.0 and stops at the second dot,
// which the old looksNumeric()-then-stod() pairing could not detect
// because looksNumeric() never re-checked what stod() actually consumed.
bool parseDouble(const std::string& s, double& out) {
    if (s.empty()) return false;
    try {
        std::size_t pos = 0;
        const double value = std::stod(s, &pos);
        if (pos != s.size()) return false;
        out = value;
        return true;
    } catch (...) {
        return false;
    }
}

// Header-row detection only needs to know "does this look like a number
// at all", so it reuses the same strict parser rather than keeping a
// second, looser numeric check that could disagree with it.
bool isFullyNumeric(const std::string& s) {
    double unused;
    return parseDouble(s, unused);
}

}  // namespace

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    current += '"';
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                current += c;
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ',') {
                fields.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
    }
    fields.push_back(current);
    return fields;
}

CsvLoadResult loadStationsFromCsv(const std::string& path) {
    CsvLoadResult result;

    std::ifstream file(path);
    if (!file.is_open()) {
        result.warnings.push_back("could not open file: " + path);
        return result;
    }
    result.fileOpened = true;

    std::string line;
    int lineNumber = 0;
    bool checkedForHeader = false;

    while (std::getline(file, line)) {
        ++lineNumber;

        // Strip a UTF-8 BOM if present on the very first line.
        if (lineNumber == 1 && line.size() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF) {
            line = line.substr(3);
        }

        // Also tolerate a trailing '\r' from CRLF line endings.
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (trim(line).empty()) continue;

        std::vector<std::string> fields = splitCsvLine(line);

        if (!checkedForHeader) {
            checkedForHeader = true;
            if (!fields.empty() && !isFullyNumeric(trim(fields[0]))) {
                result.warnings.push_back("line " + std::to_string(lineNumber) +
                                           ": detected header row, skipped");
                continue;
            }
        }

        if (fields.size() < 8) {
            result.warnings.push_back("line " + std::to_string(lineNumber) +
                                       ": expected at least 8 columns, got " +
                                       std::to_string(fields.size()) + ", skipped");
            continue;
        }

        Station station;
        station.id = trim(fields[0]);
        station.name = trim(fields[1]);
        station.address = trim(fields[2]);

        bool ok = true;
        ok = ok && parseInt(trim(fields[3]), station.totalDocks);
        ok = ok && parseInt(trim(fields[4]), station.docksInService);
        station.status = trim(fields[5]);
        ok = ok && parseDouble(trim(fields[6]), station.latitude);
        ok = ok && parseDouble(trim(fields[7]), station.longitude);

        if (!ok || station.id.empty()) {
            result.warnings.push_back("line " + std::to_string(lineNumber) +
                                       ": non-numeric or missing field, skipped");
            continue;
        }

        result.stations.push_back(std::move(station));
    }

    return result;
}

}  // namespace bikeviz
