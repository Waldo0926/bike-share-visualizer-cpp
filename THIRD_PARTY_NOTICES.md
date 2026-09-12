# Third-Party Notices

The MIT license in [`LICENSE`](LICENSE) covers the code written for this
project: everything under `include/`, `src/`, and `tests/`, plus the build
and CI configuration. It does not cover the items below, which predate this
project or originate outside it.

## Station dataset (`data/chicago_bike_stations.csv`)

This CSV is a snapshot of Chicago's Divvy bike-share station data, obtained
as part of an object-oriented-programming coursework assignment and reused
here unchanged so the tests and CLI examples run against real station data
rather than synthetic values. It is bundled purely for demonstration and
testing; no ownership over the underlying dataset is claimed. Anyone wanting
to use this data for a purpose beyond running this repository's own tests
should get a current copy from Divvy's own published data and check the
terms attached to it, since a multi-year-old classroom snapshot is not a
substitute for that.

## Map image (`data/map.bmp`, `legacy/mfc_original/map.bmp`)

The calibrated Chicago-area bitmap used as a rendering backdrop is the same
image asset from the original 2021 coursework. Its ultimate origin was not
tracked at the time; it is included only so the modernised renderer can be
demonstrated against the exact backdrop the original MFC program used.

## Legacy MFC project (`legacy/mfc_original/`)

This folder is the original coursework submission by Shuoxun Wen (温硕勋),
kept unmodified for comparison against the modern reimplementation. Most of
it is original coursework code, but several files are boilerplate generated
by Visual Studio's MFC AppWizard rather than hand-written for the
assignment: `framework.h`, `targetver.h`, `pch.h`/`pch.cpp`, and the
generated sections of `resource.h` and the `.rc`/`.rc2` resource scripts.
That scaffolding is standard output of Visual Studio's project templates
and is reproduced here as-is, as part of preserving the original project
structure, rather than as code this repository claims authorship of.
