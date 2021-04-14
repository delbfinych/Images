#pragma once

#include <cstdint>

class Pixel {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
public:
    size_t colorSum() const {
        return red + green + blue + alpha;
    }
    Pixel(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0)
            : red(r), green(g), blue(b), alpha(a) {};

    static uint8_t calculateColor(int colorValue) {
        if (colorValue > 255) {
            return 255;
        }
        if (colorValue < 0) {
            return 0;
        }
        return colorValue;
    }
};



