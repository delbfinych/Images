#include <cassert>
#include <string>
#include "Image.h"
#include "JPG/JPG.h"
#include "PNG/PNG.h"
#include <utility>
#include <cmath>
#include <stdexcept>
#include <sstream>


PixelMatrix::PixelMatrix(size_t h, size_t w)
        : _height(h), _width(w)
{
    if (h == 0 || w == 0) {
        _data = nullptr;
    }
    _data = new Pixel[h * w];
}

Pixel* PixelMatrix::operator[](size_t idx) const {
    if (idx >= _height) {
        std::stringstream str;
        str << "PixelMatrix::out of range error. Idx = "
            << idx
            << " >= this->_height which is "
            << _height
            << std::endl;
        throw std::out_of_range(str.str());
    }
    return &_data[idx * _width];
}

PixelMatrix::PixelMatrix(Pixel* data, size_t w, size_t h)
        : _height(h), _width(w)
{
    _data = new Pixel[w * h];
    memcpy(_data, data, w * h * sizeof(Pixel));
}

PixelMatrix::~PixelMatrix() {
    delete[] _data;
}

PixelMatrix& PixelMatrix::operator=(PixelMatrix&& other) noexcept {
    if (this != &other) {
        delete[] _data;
        _data   = std::exchange(other._data, nullptr);
        _width  = std::exchange(other._width, 0);
        _height = std::exchange(other._height, 0);
    }
    return *this;
}

PixelMatrix::PixelMatrix(const PixelMatrix& other) {
    _height = other._height;
    _width = other._width;
    _data = new Pixel[_height * _width];
    memcpy(_data, other._data, _height * _width * sizeof(Pixel));
}

void PixelMatrix::applyFilter(const ColorMatrix& matrix) {
    for (size_t y = 0; y < _height; ++y) {
        for (size_t x = 0; x < _width; ++x) {
           operator[](y)[x] = matrix * operator[](y)[x];
        }
    }
}


Image* Image::create(const std::string& fileName) {

    const size_t dotIdx = fileName.find_last_of('.') + 1;
    const std::string& ext = &fileName[0] + dotIdx;

    if (ext == "jpg") {
        return new JPG(fileName);
    }
    else if (ext == "png") {
        return new PNG(fileName);
    }
    else {
        std::stringstream str;
        str << "Image::create. \"" << ext << "\" is invalid imageType" << std::endl;
        throw std::runtime_error(str.str());
    }
}

size_t Image::height() const {
    return _matrix._height;
}

size_t Image::width() const {
    return _matrix._width;
}

uint8_t* Image::data() const {
    return reinterpret_cast<uint8_t*>(_matrix._data);
}

void Image::invert() {
    for (size_t y = 0; y < height(); ++y) {
        for (size_t x = 0; x < width(); ++x) {
           _matrix[y][x].red = 255 - _matrix[y][x].red;
           _matrix[y][x].green = 255 - _matrix[y][x].green;
           _matrix[y][x].blue = 255 - _matrix[y][x].blue;
           _matrix[y][x].alpha = 255 - _matrix[y][x].alpha;
        }
    }
}

void Image::mirrorHorizontally() {
    for (size_t y = 0; y < height(); ++y) {
        for (size_t x = 0; x < width() / 2; ++x) {
            std::swap(_matrix[y][x], _matrix[y][width() - 1 - x]);
        }
    }

}

void Image::mirrorVertically() {
    for (size_t y = 0; y < height() / 2; ++y) {
        for (size_t x = 0; x < width(); ++x) {
            std::swap(_matrix[y][x], _matrix[height() - 1 - y][x]);
        }
    }
}

void Image::rotate(double deg) {
    if ((int)deg % 360 == 0) {
        return;
    }
    const double theta = deg * M_PI / 180.0;
    const double cosTheta = cos(theta);
    const double sinTheta = sin(theta);
    const double as = std::abs(sinTheta);
    const double ac = std::abs(cosTheta);
    const int newW = (int)(height() * as + width() * ac);
    const int newH = (int)(height() * ac + width() * as);
    PixelMatrix resultMatrix(newH, newW);
    const int x0 = resultMatrix._width / 2;
    const int y0 = resultMatrix._height / 2;

    int dx, dy;
    for (size_t y = 0; y < resultMatrix._height; ++y) {
        for (size_t x = 0; x < resultMatrix._width; ++x) {
            dx = x - x0;
            dy = y - y0;
            int newX = (int)(cosTheta * dx - sinTheta * dy + x0);
            int newY = (int)(sinTheta * dx + cosTheta * dy + y0);
            newX -= (int)(0.5 * resultMatrix._width - 0.5 * width()); // translate -50% x and -50% y
            newY -= (int)(0.5 * resultMatrix._height - 0.5 * height()); // translate -50% x and -50% y
            if (newX >= 0 && newX < width() && newX >= 0 && newY < height()) {
                resultMatrix[y][x] = _matrix[newY][newX];
            }
        }
    }
    _matrix = std::move(resultMatrix);
}



void Image::sepia() {
    mixChannels(0.393, 0.769, 0.189,
                0.349, 0.686, 0.168,
                0.272, 0.534, 0.131);
}

void Image::blackWhite() {
    for (size_t i = 0; i < height(); ++i) {
        for (size_t j = 0; j < width(); ++j) {
            size_t avg = (_matrix[i][j].red + _matrix[i][j].blue + _matrix[i][j].green) / 3;
            if (avg <= 127) {
                _matrix[i][j] = { 0, 0, 0, _matrix[i][j].alpha };
            }
            else {
                _matrix[i][j] = { 255, 255, 255, _matrix[i][j].alpha };
            }
        }
    }
}

void Image::grayscale() {
    mixChannels(0.3, 0.59, 0.11,
                0.3, 0.59, 0.11,
                0.3, 0.59, 0.11);
}


void Image::pixelate(size_t blockSize) {
    for (size_t y = 0; y < height(); y += blockSize) {
        for (size_t x = 0; x < width(); x += blockSize) {
            Pixel color = _matrix[y][x];

            // начиная с позиции (y,x) покрываем пикселем color
            // квадрат размера blockSize*blockSize
            for (size_t i = y; i < y + blockSize; ++i) {
                for (size_t j = x; j < x + blockSize; ++j) {
                    if (i < height() && j < width()) {
                        _matrix[i][j] = color;
                    }
                }
            }
        }
    }
}

void Image::brightness(float percent) {
    float c = 1 / 100 * percent;
    mixChannels(c, 0, 0,
                0, c, 0,
                0, 0, c);
}

void Image::saturate(float s) {
    float sr = (1.f - s) * 0.3086;
    float sg = (1 - s) * 0.6094;
    float sb = (1 - s) * 0.0820;

    mixChannels(sr+s, sg,   sb,
                sr,   sg+s, sb,
                sr,   sg,   sb+s);
}

void Image::resizeWidth(size_t w) {
    float ratio = w > width() ? w / width() : width() / w;

    PixelMatrix buffer(height(), w) ;

    for (size_t y = 0; y < height(); ++y) {
        for (size_t x = 0; x < w; ++x) {
            int beg = (int)(x * ratio);
            int end = (int)(beg + ratio);

            int pIdx = beg;
            for (int k = beg; k < end; ++k) {
                if (_matrix[y][beg].colorSum() > _matrix[y][pIdx].colorSum()) {
                    pIdx = k;
                }
            }
            buffer[y][x] = _matrix[y][pIdx];
        }
    }
    _matrix = std::move(buffer);

}

void Image::resizeHeight(size_t h) {
    float ratio = h > height() ? h / height() : height() / h;
    PixelMatrix buffer(h, width());

    for (size_t y = 0; y < width(); ++y) {
        for (size_t x = 0; x < h; ++x) {
            int beg = (int)(x * ratio);
            int end = (int)(beg + ratio);

            int pIdx = beg;
            for (int k = beg; k < end; ++k) {
                if (_matrix[beg][y].colorSum() > _matrix[pIdx][y].colorSum()) {
                    pIdx = k;
                }
            }
            buffer[x][y] = _matrix[pIdx][y];
        }
    }
    _matrix = std::move(buffer);
}

void Image::resize(size_t percent) {
    size_t h = height() * 0.01 * percent;
    size_t w = width() * 0.01 * percent;
    resizeWidth(w);
    resizeHeight(h);
}

void Image::blur(int radius) {
    horizontalBlur(radius);
    verticalBlur(radius);
}

void Image::verticalBlur(int radius) {
    for (int i = 0; i < width(); ++i) {
        for (int j = 0; j < height(); ++j) {
            int r, g, b, a, count;
            r = g = b = a = count = 0;

            for (int k = j - radius; k <= j + radius; ++k) {
                if (k >= 0 && k < height()) {
                    r += _matrix[k][i].red;
                    g += _matrix[k][i].green;
                    b += _matrix[k][i].blue;
                    a += _matrix[k][i].alpha;
                    count ++;
                }
            }
            Pixel blured(
                    r / count,
                    g / count,
                    b / count,
                    a / count
            );
            _matrix[j][i] = blured;
        }
    }
}

void Image::horizontalBlur(int radius) {
    for (int i = 0; i < height(); ++i) {
        // Можно оптимизировать: если обработать первый пиксель в строке, то каждый следующий
        // может быть вычислен через предыдущий, путем вычитания из суммы самого первого на предыдущем шаге
        // и прибавления последнего на новом шаге.
        for (int j = 0; j < width(); ++j) {
            int r, g, b, a, count;
            r = g = b = a = count = 0;

            for (int k = j - radius; k <= j + radius; ++k) {
                if (k >= 0 && k < width()) {
                    r += _matrix[i][k].red;
                    g += _matrix[i][k].green;
                    b += _matrix[i][k].blue;
                    a += _matrix[i][k].alpha;
                    count ++;
                }
            }
            Pixel blured(
                    r / count,
                    g / count,
                    b / count,
                    a / count
            );
            _matrix[i][j] = blured;
        }
    }
}

void Image::mixChannels(float rr, float rg, float rb,
                        float gr, float gg, float gb,
                        float br, float bg, float bb)
{
    array<array<float, 5>, 4> m = {
            {
                    {rr, rg, rb, 0, 0},
                    {gr, gg, gb, 0, 0},
                    {br, bg, bb, 0, 0},
                    { 0,  0,  0, 1, 0}
            }
    };
    _matrix.applyFilter(ColorMatrix(m));

}

void Image::scale(size_t percent) {
    size_t newWidth = width() * 0.01 * percent;
    size_t newHeight = height() * 0.01 * percent;

    float dx = (float)width() / newWidth;
    float dy = (float)height() / newHeight;

    PixelMatrix result(newHeight, newWidth);
    for(size_t i = 0 ; i < newHeight; ++i) {
        for (size_t j = 0; j < newWidth; ++j) {
            result[i][j] = _matrix[(int)(i * dy)][(int)(j * dx)];
        }
    }
    _matrix = std::move(result);
}

void Image::extractRed() {
    mixChannels(1, 0, 0, 0, 0, 0, 0, 0, 0);
}

void Image::extractGreen() {
    mixChannels(0, 0, 0, 0, 1, 0, 0, 0, 0);
}

void Image::extractBlue() {
    mixChannels(0, 0, 0, 0, 0, 0, 0, 0, 1);
}





