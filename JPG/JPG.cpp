#include <string>
#include "JPG.h"
#include "../JpegCompressor/jpgd.h"
#include "../JpegCompressor/jpge.h"

void JPG::save(const std::string& fileName) {
    jpge::compress_image_to_jpeg_file(fileName.c_str(), width(), height(), 4, data());
}

JPG::JPG(const std::string& fileName) {
    int width, height, actualComps;
    uint8_t* data = jpgd::decompress_jpeg_image_from_file(fileName.c_str(), &width, &height, &actualComps, 4);
    auto matrix = PixelMatrix(reinterpret_cast<Pixel*>(data), width, height);
    _matrix = std::move(matrix);
    delete[] data;
}

