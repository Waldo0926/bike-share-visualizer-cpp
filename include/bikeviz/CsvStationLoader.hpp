#pragma once

#include <string>
#include <vector>

#include "bikeviz/Station.hpp"

namespace bikeviz {

// Result of loading a station CSV file: the successfully parsed stations
// plus a human-readable warning for every line that had to be skipped
// (malformed rows, non-numeric fields, a detected header row, ...).
//
// `fileOpened` distinguishes "the path was wrong" from "the file was
// readable but contained no usable rows" — two failures a caller
// usually wants to report differently, and which a plain empty vector
// would conflate.
struct CsvLoadResult {
    std::vector<Station> stations;
    std::vector<std::string> warnings;
    bool fileOpened = false;
};

// Parses a single CSV line into raw fields, honouring double-quoted
// fields that may themselves contain commas (e.g. the "(lat, lon)"
// Location column in the original dataset). This is intentionally a
// small, dependency-free parser rather than a full RFC 4180 implementation.
std::vector<std::string> splitCsvLine(const std::string& line);

// Loads stations from a CSV file at `path`.
//
// The original coursework program read the file's first line as if it
// were already a data row, so the header ("Station ID,Station Name,...")
// was silently counted as a 589th "station" with every numeric field
// parsed as zero. This loader detects a header row (first field is not
// numeric) and skips it, and skips any row that does not have at least
// the eight expected columns or whose numeric columns fail to parse,
// recording a warning for each instead of silently producing garbage data.
CsvLoadResult loadStationsFromCsv(const std::string& path);

}  // namespace bikeviz
