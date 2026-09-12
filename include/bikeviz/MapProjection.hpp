#pragma once

namespace bikeviz {

struct PixelPoint {
    int x = 0;
    int y = 0;
};

// Converts GPS coordinates (latitude, longitude) into pixel coordinates
// on the bundled Chicago map image (legacy/assets/map.bmp).
//
// The original program buried this transform as a magic-number one-liner
// inside a button-click handler:
//
//   int x = (87.8 + longitude) * 1460.0;
//   int y = (42.1 - latitude) * 1960.0;
//
// which is the same affine map expressed here as an explicit,
// independently testable class: pick an origin (longitude0, latitude0)
// that corresponds to pixel (0, 0), and a scale factor per axis that
// stretches one degree of GPS into map pixels.
class MapProjection {
public:
    // Defaults reproduce the original calibration exactly, so the modern
    // renderer lines up with the same map.bmp the coursework program used.
    MapProjection(double originLongitude = -87.8,
                   double originLatitude = 42.1,
                   double scaleX = 1460.0,
                   double scaleY = 1960.0)
        : originLongitude_(originLongitude),
          originLatitude_(originLatitude),
          scaleX_(scaleX),
          scaleY_(scaleY) {}

    PixelPoint toPixel(double latitude, double longitude) const {
        const double x = (longitude - originLongitude_) * scaleX_;
        const double y = (originLatitude_ - latitude) * scaleY_;
        return PixelPoint{static_cast<int>(x), static_cast<int>(y)};
    }

    double scaleX() const { return scaleX_; }
    double scaleY() const { return scaleY_; }
    double originLongitude() const { return originLongitude_; }
    double originLatitude() const { return originLatitude_; }

private:
    double originLongitude_;
    double originLatitude_;
    double scaleX_;
    double scaleY_;
};

}  // namespace bikeviz
