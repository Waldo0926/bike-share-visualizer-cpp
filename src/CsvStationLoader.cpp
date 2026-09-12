#include "bikeviz/CsvStationLoader.hpp"

#include <cctype>
#include <fstream>
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

bool looksNumeric(const std::string& s) {
    if (s.empty()) return false;
    std::size_t i = 0;
    if (s[i] == '+' || s[i] == '-') ++i;
    if (i >= s.size()) return false;
    bool sawDigit = false;
    for (; i < s.size(); ++i) {
        if (std::isdigit(static_cast<unsigned char>(s[i]))) {
            sawDigit = true;
        } else if (s[i] == '.') {
            continue;
        } else {
            return false;
        }
    }
    return sawDigit;
}

bool parseInt(const std::string& s, int& out) {
    if (!looksNumeric(s)) return false;
    try {
        out = std::stoi(s);
        return true;
    } catch (...) {
        return false;
    }
}

bool parseDouble(const std::string& s, double& out) {
    if (!looksNumeric(s)) return false;
    try {
        out = std::stod(s);
        return true;
    } catch (...) {
        return false;
    }
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
            if (!fields.empty() && !looksNumeric(trim(fields[0]))) {
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
