# Modernisation notes

The original program (preserved in [`../legacy/`](../legacy/)) worked — it
was graded, and it really did import a 588-station dataset, plot it on a
city map, answer station queries and animate a simulated trip.
Revisiting it in 2026 was about finding what an early-undergraduate MFC
dialog class tends to get wrong once you are no longer racing a course
deadline, and fixing each thing properly rather than patching around it.

Handily, the original submission's report included screenshots of the
running program, so several of the issues below can be checked against
what the program itself displayed rather than taken on trust.

## 1. The header row was silently counted as a station

`OnBnClickedButton1()` opened the CSV and read straight into the parsing
loop:

```cpp
CString str = NULL;
f.ReadString(str);
int i = 0;
while (str != "" && i++ < 589)
{
    SL.AddString(str);
    f.ReadString(str);
}
```

The first line of `chicago_bike_stations.csv` is the column header
(`Station ID,Station Name,Address,...`), not data. Nothing checked for
that, so the header became "station #1": `_wtoi("Total Docks")` and
`_wtof("Latitude")` both quietly evaluate to `0`, and the status field
became the literal string `Status`, which is not `"In Service"` — so the
phantom row was also counted as a station that was *out* of service.

The original program's own statistics panel shows exactly this
(screenshot: [`screenshots/legacy_station_overview.png`](screenshots/legacy_station_overview.png)):

| Original program's panel | Value |
| --- | --- |
| 城市站点总数 (total stations) | **589** |
| 正常运营总数 (in service) | 588 |
| 无法运营总数 (not in service) | **1** |
| 可用单车桩数 (docks in service) | 9835 |
| 异常单车桩数 (docks out of service) | 416 |

The modern loader, on the same file, reports:

```
Loaded 588 stations
  in service:     588
  not in service: 0
  total docks:    10251
  docks in service: 9835
```

The dock numbers agree exactly (9835 in service, and 10251 − 9835 = 416
out of service, matching the original's 416), which confirms the parsing
is otherwise faithful. The only differences are the station count (588
rather than 589) and the "not in service" count (0 rather than 1) — both
of which were the header row being mistaken for a real station with zero
docks. There was never an out-of-service station in this dataset.

`CsvStationLoader::loadStationsFromCsv` inspects the first field of the
first row and, if it is not numeric, treats the row as a header and skips
it with a recorded warning.
`testLoaderSkipsHeaderRow` pins this down on a fabricated header plus two
data rows, and `testRealDatasetLoadsExpectedTotals` asserts the full set
of real-dataset numbers above so the fix stays honest.

## 2. `rand() % 589` assumed a fixed dataset size

`OnBnClickedCheckbutton()` picked two random station indices with
`rand() % 589` — hardcoded to the row count of one specific file
(including, as it happens, the header row). Point it at a dataset with
more or fewer rows and it would either never reach the later stations or
index past the end of the arrays.

`pickRandomTripEndpoints` samples over `repository.size()`, and takes the
random source as an injected `std::function` so the selection logic is
testable without depending on process-global RNG state.

## 3. The "don't pick the same station twice" guard did not work

Having drawn two indices, the original tried to avoid a trip that starts
and ends at the same station:

```cpp
num1 = rand() % 589;
num2 = rand() % 589;
if (num1 == num2)
{
    num2 = num2 % 100;
}
```

`num2 % 100` does not make `num2` differ from `num1`. Whenever the
collision happened at a value below 100 — which is 100 of the 589
possible collisions — `num2 % 100 == num2`, so the "fix" left both
indices identical. The program got away with it because the separation
heuristic further down (which requires the two points to be at least 100
pixels apart on both axes) then rejected the degenerate pair anyway. The
guard was dead code masked by a later check.

`pickRandomTripEndpoints` simply draws again when `i == j`, and the
separation heuristic remains as an independent condition rather than as
the thing accidentally holding this together.

## 4. No bound on the trip-pair retry loop

The same function's `while (flag2 == 0)` loop kept drawing random pairs
until it found two stations far enough apart and roughly diagonal to each
other. For the bundled Chicago dataset that terminates quickly in
practice, but nothing stopped it spinning forever on a sparser or more
clustered dataset where the heuristic can never be satisfied — and since
this ran on the UI thread, that would have frozen the whole window with
no way out.

`pickRandomTripEndpoints` takes a `maxAttempts` bound (default 2000) and
returns `std::nullopt` instead of hanging.
`testTripSimulatorGivesUpOnImpossibleDataset` builds a dataset where
every station shares one coordinate, so no pair can ever qualify, and
checks the call returns promptly.

## 5. CSV parsing assumed no quoted fields, and never validated anything

Fields were extracted with repeated `CString::Find(',')` / `Mid()` calls.
That happens to work for the first eight columns of this file, but the
dataset's ninth column is `"(41.956057, -87.668835)"` — a quoted field
containing its own comma, which naive comma-splitting would tear in half
if anything ever needed it. Numeric fields were never validated either: a
malformed row was not rejected, it just converted to `0` through
`_wtoi`/`_wtof` and joined the totals.

`splitCsvLine` is a small quoted-field-aware tokenizer
(`testSplitCsvLineHandlesQuotedCommas` checks it against that exact
column), and `loadStationsFromCsv` validates every numeric field before
accepting a row, recording a warning and skipping it otherwise
(`testLoaderSkipsMalformedRows`). The result also distinguishes "the file
could not be opened" from "the file was readable but had no usable rows"
via `fileOpened`, because the CLI should exit non-zero on a mistyped path
rather than cheerfully reporting zero stations
(`testLoaderDistinguishesEmptyFileFromMissingFile`).

## 6. The coordinate transform was a magic-number one-liner, three times over

```cpp
int x = ((87.8 + _wtof(Longitude[num])) * 1460.0),
    y = ((42.1 - _wtof(Latitude[num])) * 1960.0);
```

appears, copy-pasted, in three different member functions
(`PaintPoint(int)`, `OnBnClickedPaintall()` and the overloaded
`PaintPoint(int, int, int, int, int)`), and the same two literals appear
a fourth time in the trip-distance calculation. Recalibrating the map
would have meant finding all four and changing them in lockstep.

`MapProjection` is that same affine transform as one class with named
parameters (`originLongitude`, `originLatitude`, `scaleX`, `scaleY`),
defaulted to the original constants so it reproduces identical pixel
positions (`testProjectionMatchesOriginalCalibration`) while remaining
configurable (`testProjectionIsConfigurable`). `estimateTripDistance`
now takes its scale factors *from the projection* instead of keeping its
own copies, so the distance metric cannot drift away from the map
calibration (`testTripDistanceFollowsProjectionScale`).

## 7. One station falls off the map, and nothing said so

Writing an integration test over the real dataset turned up something the
original could not have told you: of the 588 stations, **587 project
inside the map image and one does not**. Station 398, *Rainbow Beach*, is
the easternmost station in the export (longitude −87.5494) and lands at
x = 365 on a 365-pixel-wide image — about 50 metres past the right edge
of the area the map was calibrated for.

The original drew its points straight into a Win32 device context, which
clips anything outside the control and reports nothing, so the station
was simply never visible and no one was any the wiser.

`BitmapImage::drawMarker` still clips (one stray station should not abort
a batch render) but now returns `false` when the marker's centre lands
off-canvas, so callers can count and report. The CLI does:

```
Rendering 588 stations onto data/map.bmp...
  note: Rainbow Beach (398) projects outside the map area at pixel (365, 670)
  saved to rendered_map.bmp (587 drawn, 1 outside the map area)
```

and `testRealMapRendersEveryStation` asserts the 587/1 split *and* that
the off-map station is specifically `398 Rainbow Beach`, so a future
change to the projection constants cannot quietly move stations off the
map without a test noticing.

## 8. Hardcoded paths and row-count ceilings

The file-open dialog defaulted to `L"D:\\"`, which only helps if the
person running it happens to have a `D:` drive. The user-import handler
was capped at 50 rows (`i++ < 50`) the same way the station loader was
capped at 589 — literals baked into loop conditions rather than derived
from the data. None of this misbehaves on exactly the right input, which
is what makes it the kind of thing that breaks quietly on someone else's
machine. The CLI takes every path as an argument and imposes no row
ceiling; the loader processes as many well-formed rows as the file holds.

## What stayed the same on purpose

The trip-distance formula
(`(|Δlongitude| × scaleX + |Δlatitude| × scaleY) / 5`) and the
pixel-space "is this pair spread out enough to animate" heuristic
(`|Δy − Δx| < 30`, `|Δx| > 100`, `|Δy| > 100`) are both kept exactly as
the original defined them. They were sensible reads of the assignment
brief — approximate a trip on a Manhattan-style grid, and only animate
trips that read as a visible diagonal on the map — not bugs.
`testTripDistanceMatchesOriginalFormula` pins the formula to the original
worked values.

Worth being precise about one thing, though: that figure is a
grid-distance proxy in scaled map units, not a real distance. The
original dialog labelled the field "KM", and for Chicago the numbers land
in a plausible-looking range (the screenshot shows 45 for a cross-city
trip), but nothing in the formula converts to kilometres. The
modernised version keeps the metric and drops the unit claim, exposing
the divisor as a parameter for anyone who wants to calibrate it into real
units.
