#include "bikeviz/BitmapImage.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace bikeviz {

namespace {

// Anything past this many pixels is almost certainly either a corrupt
// header or a deliberately hostile one (a 32-bit width/height field can
// claim up to ~4 billion x 4 billion), not a real map image. 64 million
// pixels is generous for this project's use case — the bundled map is
// 365x785 — while keeping a malformed file from making this class try
// to allocate gigabytes of memory before anyone notices it's garbage.
constexpr std::int64_t kMaxPixelCount = 64'000'000;
constexpr std::int32_t kMaxDimension = 20'000;

std::uint32_t readU32(const std::vector<std::uint8_t>& buf, std::size_t offset) {
    return static_cast<std::uint32_t>(buf[offset]) |
           (static_cast<std::uint32_t>(buf[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(buf[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(buf[offset + 3]) << 24);
}

std::int32_t readI32(const std::vector<std::uint8_t>& buf, std::size_t offset) {
    return static_cast<std::int32_t>(readU32(buf, offset));
}

std::uint16_t readU16(const std::vector<std::uint8_t>& buf, std::size_t offset) {
    return static_cast<std::uint16_t>(buf[offset]) |
           (static_cast<std::uint16_t>(buf[offset + 1]) << 8);
}

void writeU32(std::vector<std::uint8_t>& buf, std::uint32_t value) {
    buf.push_back(static_cast<std::uint8_t>(value & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
}

void writeU16(std::vector<std::uint8_t>& buf, std::uint16_t value) {
    buf.push_back(static_cast<std::uint8_t>(value & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
}

// Shared by the public blank-canvas constructor and the file loader, so
// a caller cannot construct an unreasonably large or degenerate canvas
// through either path.
void validateDimensions(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("bitmap width and height must both be positive");
    }
    if (width > kMaxDimension || height > kMaxDimension) {
        throw std::runtime_error("bitmap dimensions exceed the supported maximum (" +
                                  std::to_string(kMaxDimension) + " per side)");
    }
    const std::int64_t pixelCount = static_cast<std::int64_t>(width) * static_cast<std::int64_t>(height);
    if (pixelCount > kMaxPixelCount) {
        throw std::runtime_error("bitmap pixel count exceeds the supported maximum (" +
                                  std::to_string(kMaxPixelCount) + " pixels)");
    }
}

}  // namespace

BitmapImage::BitmapImage(int width, int height)
    : width_(width), height_(height), pixels_() {
    validateDimensions(width, height);
    pixels_.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 3, 0);
}

BitmapImage BitmapImage::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("could not open bitmap: " + path);
    }

    std::vector<std::uint8_t> buf((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
    if (buf.size() < 54 || buf[0] != 'B' || buf[1] != 'M') {
        throw std::runtime_error("not a BMP file: " + path);
    }

    const std::uint32_t dataOffset = readU32(buf, 10);
    const std::uint32_t headerSize = readU32(buf, 14);
    const std::int32_t rawWidth = readI32(buf, 18);
    const std::int32_t rawHeight = readI32(buf, 22);
    const std::uint16_t planes = readU16(buf, 26);
    const std::uint16_t bpp = readU16(buf, 28);
    const std::uint32_t compression = readU32(buf, 30);

    // BITMAPINFOHEADER is the only layout this reader understands the
    // field offsets of; an older BITMAPCOREHEADER (12 bytes) or a newer
    // V4/V5 header would put width/height/bpp at different positions,
    // and silently misreading them is how you get a "valid-looking" but
    // wrong width used later for pointer arithmetic.
    if (headerSize < 40) {
        throw std::runtime_error("unsupported BMP header size (" + std::to_string(headerSize) +
                                  " bytes); only BITMAPINFOHEADER (40) is supported: " + path);
    }
    if (compression != 0) {
        throw std::runtime_error("only uncompressed BMP files are supported: " + path);
    }
    if (planes != 1) {
        throw std::runtime_error("unsupported BMP plane count: " + path);
    }
    if (bpp != 8 && bpp != 24) {
        throw std::runtime_error("only 8-bit palette or 24-bit BMP files are supported: " + path);
    }
    if (rawWidth <= 0) {
        throw std::runtime_error("BMP width must be positive: " + path);
    }
    if (rawHeight == 0) {
        throw std::runtime_error("BMP height must not be zero: " + path);
    }
    // rawHeight == INT32_MIN has no positive negation and would
    // overflow signed int if negated directly.
    if (rawHeight == std::numeric_limits<std::int32_t>::min()) {
        throw std::runtime_error("BMP height out of range: " + path);
    }

    const int width = rawWidth;
    const bool bottomUp = rawHeight > 0;
    const int height = bottomUp ? rawHeight : -rawHeight;

    // Validates width/height are within the sane range this class
    // supports before anything downstream does arithmetic with them.
    validateDimensions(width, height);

    if (dataOffset > buf.size()) {
        throw std::runtime_error("BMP pixel data offset is past the end of the file: " + path);
    }

    const std::uint64_t paletteOffset = static_cast<std::uint64_t>(14) + headerSize;
    if (bpp == 8) {
        // Every byte in an 8-bit row indexes into a 256-entry, 4-byte
        // BGRA palette; the whole table has to actually be present.
        const std::uint64_t paletteEnd = paletteOffset + 256ull * 4;
        if (paletteOffset > buf.size() || paletteEnd > buf.size()) {
            throw std::runtime_error("BMP colour palette is truncated: " + path);
        }
    }

    // Row stride is padded to a 4-byte boundary per the BMP spec. Computed
    // in 64-bit so a large-but-still-in-range width cannot overflow it.
    const std::uint64_t bytesPerPixelRow =
        (static_cast<std::uint64_t>(width) * bpp + 7) / 8;
    const std::uint64_t rowStride = (bytesPerPixelRow + 3) & ~static_cast<std::uint64_t>(3);
    const std::uint64_t requiredBytes =
        static_cast<std::uint64_t>(dataOffset) + rowStride * static_cast<std::uint64_t>(height);

    if (requiredBytes > buf.size()) {
        throw std::runtime_error(
            "BMP file is smaller than its header claims (truncated or corrupt pixel data): " + path);
    }

    BitmapImage image(width, height);

    for (int row = 0; row < height; ++row) {
        const int srcRow = bottomUp ? (height - 1 - row) : row;
        const std::size_t rowStart =
            static_cast<std::size_t>(dataOffset) + static_cast<std::size_t>(srcRow) * static_cast<std::size_t>(rowStride);

        for (int col = 0; col < width; ++col) {
            std::uint8_t r, g, b;
            if (bpp == 8) {
                const std::uint8_t index = buf[rowStart + static_cast<std::size_t>(col)];
                const std::size_t entry =
                    static_cast<std::size_t>(paletteOffset) + static_cast<std::size_t>(index) * 4;
                b = buf[entry + 0];
                g = buf[entry + 1];
                r = buf[entry + 2];
            } else {
                const std::size_t px = rowStart + static_cast<std::size_t>(col) * 3;
                b = buf[px + 0];
                g = buf[px + 1];
                r = buf[px + 2];
            }
            const std::size_t out = (static_cast<std::size_t>(row) * static_cast<std::size_t>(width) + static_cast<std::size_t>(col)) * 3;
            image.pixels_[out + 0] = r;
            image.pixels_[out + 1] = g;
            image.pixels_[out + 2] = b;
        }
    }

    return image;
}

void BitmapImage::save(const std::string& path) const {
    const int rowStride = ((width_ * 3) + 3) & ~3;
    const std::uint32_t pixelDataSize = static_cast<std::uint32_t>(rowStride) * height_;
    const std::uint32_t fileSize = 54 + pixelDataSize;

    std::vector<std::uint8_t> out;
    out.reserve(fileSize);

    out.push_back('B');
    out.push_back('M');
    writeU32(out, fileSize);
    writeU32(out, 0);
    writeU32(out, 54);

    writeU32(out, 40);
    writeU32(out, static_cast<std::uint32_t>(width_));
    writeU32(out, static_cast<std::uint32_t>(height_));
    writeU16(out, 1);
    writeU16(out, 24);
    writeU32(out, 0);
    writeU32(out, pixelDataSize);
    writeU32(out, 2835);
    writeU32(out, 2835);
    writeU32(out, 0);
    writeU32(out, 0);

    std::vector<std::uint8_t> row(rowStride, 0);
    for (int r = height_ - 1; r >= 0; --r) {
        for (int c = 0; c < width_; ++c) {
            const std::size_t src = (static_cast<std::size_t>(r) * width_ + c) * 3;
            row[c * 3 + 0] = pixels_[src + 2];  // B
            row[c * 3 + 1] = pixels_[src + 1];  // G
            row[c * 3 + 2] = pixels_[src + 0];  // R
        }
        for (int pad = width_ * 3; pad < rowStride; ++pad) row[pad] = 0;
        out.insert(out.end(), row.begin(), row.end());
    }

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("could not write bitmap: " + path);
    }
    file.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
}

bool BitmapImage::drawMarker(int x, int y, RgbColor color, int radius) {
    const bool centreInside = x >= 0 && y >= 0 && x < width_ && y < height_;

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const int px = x + dx;
            const int py = y + dy;
            if (px < 0 || py < 0 || px >= width_ || py >= height_) continue;
            const std::size_t idx = (static_cast<std::size_t>(py) * width_ + px) * 3;
            pixels_[idx + 0] = color.r;
            pixels_[idx + 1] = color.g;
            pixels_[idx + 2] = color.b;
        }
    }

    return centreInside;
}

RgbColor BitmapImage::pixelAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return RgbColor{};
    const std::size_t idx = (static_cast<std::size_t>(y) * width_ + x) * 3;
    return RgbColor{pixels_[idx + 0], pixels_[idx + 1], pixels_[idx + 2]};
}

}  // namespace bikeviz
