#include "PNG.h"
#include "../LodePNG/lodepng.h"
#include <stdexcept>

void PNG::save(const std::string& fileName) {
    lodepng::encode(fileName, data(), width(), height());
}

PNG::PNG(const std::string& fileName) {
    unsigned width, height;
    unsigned dataSize;
    uint8_t** data = new uint8_t* (nullptr);
    size_t err = lodepng::decode(data, width, height, dataSize, fileName, LCT_RGBA, 8u);
    auto matrix = PixelMatrix(reinterpret_cast<Pixel*>(*data), width, height);

    if (err) {
        throw std::runtime_error("Error while reading photo");
    }
    _matrix = std::move(matrix);
    delete[] data;
}
