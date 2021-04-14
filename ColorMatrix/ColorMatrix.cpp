#include "ColorMatrix.h"

ColorMatrix::ColorMatrix(const array<array<float, 5>, 4>& matrix) {
    memcpy(_matrix.data(), matrix.data(),
           _matrix.size() * _matrix[0].size() * sizeof(float));
}

static int min(int a, int b) {
    return a > b ? b : a;
}

Pixel ColorMatrix::operator*(Pixel pixel) const {
    int newRed = pixel.red * _matrix[0][0] + pixel.green * _matrix[0][1] + pixel.blue * _matrix[0][2] + pixel.alpha * _matrix[0][3] + _matrix[0][4];
    int newGreen = pixel.red * _matrix[1][0] + pixel.green * _matrix[1][1] + pixel.blue * _matrix[1][2] + pixel.alpha * _matrix[1][3] + _matrix[1][4];
    int newBlue = pixel.red * _matrix[2][0] + pixel.green * _matrix[2][1] + pixel.blue * _matrix[2][2] + pixel.alpha * _matrix[2][3] + _matrix[2][4];
    int newAlpha = pixel.red * _matrix[3][0] + pixel.green * _matrix[3][1] + pixel.blue * _matrix[3][2] + pixel.alpha * _matrix[3][3] + _matrix[3][4];
    Pixel newColor = {
            Pixel::calculateColor(newRed),
            Pixel::calculateColor(newGreen),
            Pixel::calculateColor(newBlue),
            Pixel::calculateColor(newAlpha)
    };
    return newColor;
}