#pragma once

#include <cstring>
#include "../Pixel/Pixel.h"
#include <array>
#include <iostream>

using std::array;

class ColorMatrix {
    array<array<float, 5>, 4> _matrix;
public:
    explicit ColorMatrix(const array<array<float, 5>, 4>& matrix);
    Pixel operator*(Pixel pixel) const;
};