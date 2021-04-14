#pragma once

#include <cstdint>
#include "ColorMatrix/ColorMatrix.h"
#include "Pixel/Pixel.h"

class Image;

class PixelMatrix {
    Pixel* _data;
    size_t _width;
    size_t _height;
public:
    friend Image;
    PixelMatrix(size_t h = 0, size_t w = 0);
    PixelMatrix(Pixel* data, size_t w = 0, size_t h = 0);
    PixelMatrix(const PixelMatrix& other);
    Pixel* operator[](size_t idx) const;
    PixelMatrix& operator=(PixelMatrix&& other) noexcept;
    void applyFilter(const ColorMatrix& matrix);
    ~PixelMatrix();

};

class Image {
public:
    /*
     * Негатив изображения
     */
    void invert();

    /*
     * Поворот изображения на угол deg
     * Поворот происходит с расширением холста
     */
    void rotate(double deg);

    /*
     * Отражение относительно горизонтальной оси симметрии
     */
    void mirrorHorizontally();

    /*
     * Отражение относительно вертикальной оси симметрии
     */
    void mirrorVertically();

    /*
     * Эффект типа старой фото?
     */
    void sepia();

    /*
     * Чёрно белый фильтр
     */
    void blackWhite();

    /*
     * Фильтр оттенков серого
     */
    void grayscale();

    /*
     * Делает эффект minecraft
     * @param blockSize - размер пикселя
     */
    void pixelate(size_t blockSize);

    /*
     * Фильтр яркости
     * @param percent - процент яркости
     */
    void brightness(float percent);

    /*
     * @param - saturation - задаётся числом
     * значение 0 убирает цветность
     * значение 1 не оказывает эффекта
     * значения от 0 до 1 уменьшают насыщенность
     * значения выше 1 увеличивают насыщенность
     */
    void saturate(float saturation);

    void resizeWidth(size_t w);
    void resizeHeight(size_t h);

    // пока шо процент > 100 не работает...
    void resize(size_t percent);

    // тут можно сделать картинку больше!
    void scale(size_t percent);
    void blur(int radius);

    /*
     * Смешивание каналов
     * @param xy  x - канал, в котором работаем, y - позиция в канале x
     * параметры принимают значение от -2 до 2
     */
    void mixChannels(float rr, float rg, float rb,
                     float gr, float gg, float gb,
                     float br, float bg, float bb
                     );

    void extractRed();
    void extractGreen();
    void extractBlue();

    static Image* create(const std::string& fileName);
    virtual void save(const std::string& fileName) = 0;
    virtual ~Image() = default; // TODO
    size_t height() const;
    size_t width() const;

private:
    void verticalBlur(int radius);
    void horizontalBlur(int radius);
protected:
    Image() = default;
    PixelMatrix _matrix;
    uint8_t* data() const;
};