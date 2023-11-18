// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef TEXTURE_H
#define TEXTURE_H

#include <filesystem>

#include "ColorRGB.hpp"
#include "TypeDefinitions.hpp"

class Texture {
public:
                                    Texture();
                                    Texture(Texture&& other) noexcept;
    virtual                         ~Texture() = default;

    void                            SetPixelSize(const Size& pixelSize) { fTextureSize = pixelSize; }
    [[nodiscard]] Size              PixelSize() const { return fTextureSize; }

    void                            SetMaxColorValue(uint32_t maxColorValue) { fMaxColorValue = maxColorValue; }
    [[nodiscard]] uint32_t          MaxColorValue() const { return fMaxColorValue; }

    bool                            MovePixelsIntoTexture(std::unique_ptr<ColorRGB[]>&& pixelArray, std::size_t pixelCount);

    ColorRGB&                       operator[](std::size_t index) { return fPixelArray.get()[index]; };
    const ColorRGB&                 operator[](std::size_t index) const { return fPixelArray.get()[index]; }

    bool                            GetPixel(std::size_t index, ColorRGB& pixel) const;
    bool                            GetPixel(std::size_t x, std::size_t y, ColorRGB& pixel) const;
    bool                            GetPixelWithTextureCoordinate(float u, float v, ColorRGB& pixel) const;

    bool                            SetPixel(std::size_t index, const ColorRGB& pixel);
    bool                            SetPixel(std::size_t x, std::size_t y, const ColorRGB& pixel);

    void                            Reset();

private:
    std::unique_ptr<ColorRGB[]> fPixelArray;
    size_t                      fPixelCount;

    Size                        fTextureSize;
    uint32_t                    fMaxColorValue;
};

MAKE_SHARED_NAME(Texture);

#endif // TEXTURE_H
