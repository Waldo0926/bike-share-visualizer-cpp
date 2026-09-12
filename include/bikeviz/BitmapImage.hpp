#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bikeviz {

struct RgbColor {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

// A tiny, dependency-free BMP loader/writer.
//
// The original program leaned on MFC's CBitmap/CDC to load map.bmp and
// draw single-pixel points straight to the screen device context, which
// only makes sense inside a live Windows dialog. This class reads the
// same 8-bit palette or 24-bit BMP into a plain RGB buffer, lets you
// stamp station markers onto it in headless code, and writes the result
// back out as a standalone 24-bit BMP any image viewer can open, so the
// visualisation survives outside of a running MFC window.
class BitmapImage {
public:
    // Creates a blank (black) canvas of the given size, mainly useful for
    // tests that don't need to load a real map image.
    BitmapImage(int width, int height);

    static BitmapImage loadFromFile(const std::string& path);

    void save(const std::string& path) const;

    int width() const { return width_; }
    int height() const { return height_; }

    // Draws a small filled square marker centred at (x, y), clipping to
    // the canvas rather than throwing, so one station outside the map's
    // calibrated area cannot abort a batch render.
    //
    // Returns false when the marker's centre falls outside the canvas
    // entirely. The original program drew straight to a Win32 device
    // context, which clipped such points away with no indication that
    // anything was missing; returning a flag lets a caller count and
    // report them instead.
    bool drawMarker(int x, int y, RgbColor color, int radius = 2);

    RgbColor pixelAt(int x, int y) const;

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<std::uint8_t> pixels_;  // row-major, top-to-bottom, RGB triples
};

}  // namespace bikeviz
