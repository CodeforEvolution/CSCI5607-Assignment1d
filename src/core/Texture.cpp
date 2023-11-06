// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

#include "Texture.hpp"

#include <fstream>

Texture::Texture()
    :
    fPixelArray(),
    fPixelCount(0),
    fTextureSize(),
    fMaxColorValue(0)
{
}

#if 0
Texture::Texture(const std::filesystem::path& texturePath)
    :
    fPixelArray(),
    fPixelCount(0),
    fTextureSize(),
    fMaxColorValue(0)
{
    if (LoadFromPPM_(texturePath))
        return;

    std::cerr << "(Error) Failed to load PPM file into Texture at path: " << texturePath << std::endl;
    Reset();
}
#endif

Texture::Texture(Texture&& other) noexcept
{
    this->fTextureSize = other.fTextureSize;
    this->fMaxColorValue = other.fMaxColorValue;
    this->fPixelCount = other.fPixelCount;
    this->fPixelArray = std::move(other.fPixelArray);

    other.Reset();
}

bool
Texture::MovePixelsIntoTexture(std::unique_ptr<ColorRGB[]>&& pixelArray, std::size_t pixelCount)
{
    // If the pixel count equals 0, just reset the pixel array.
    if (pixelCount == 0) {
        fPixelCount = 0;
        fPixelArray.reset();
        return true;
    }

    // If the pixel count is not 0, BUT have no pixel array...then something fishy is going on!
    if (!pixelArray)
        return false;

    fPixelArray = std::move(pixelArray);
    fPixelCount = pixelCount;

    return true;
}


bool
Texture::GetPixel(std::size_t index, ColorRGB& pixel) const
{
    if (index >= fPixelCount)
        return false;

    pixel = fPixelArray.get()[index];
    return true;
}

bool
Texture::GetPixel(std::size_t x, std::size_t y, ColorRGB& pixel) const
{
    if (x >= fTextureSize.width || y >= fTextureSize.height)
        return false;

    const std::size_t pixelIndex = x + (fTextureSize.width * y);
    if (pixelIndex >= fPixelCount)
        return false;

    pixel = fPixelArray[pixelIndex];
    return true;
}

bool
Texture::GetPixelWithTextureCoordinate(float u, float v, ColorRGB& pixel) const
{
	float dummy;
	// Wrap texture coordinates into the range of 0.0f to 1.0f
	float wrappedU = std::modf(u, &dummy);
	float wrappedV = std::modf(v, &dummy);

	// Nearest neighbor lookup!
    auto textureX = static_cast<std::size_t>(std::roundf(wrappedU * (fTextureSize.width - 1)));
    auto textureY = static_cast<std::size_t>(std::roundf(wrappedV * (fTextureSize.height - 1)));

    return GetPixel(textureX, textureY, pixel);
}

bool
Texture::SetPixel(std::size_t index, const ColorRGB& pixel)
{
    if (index >= fPixelCount)
        return false;

    fPixelArray.get()[index] = pixel;
    return true;
}

bool
Texture::SetPixel(std::size_t x, std::size_t y, const ColorRGB& pixel)
{
	if (x >= fTextureSize.width || y >= fTextureSize.height)
		return false;

	const std::size_t pixelIndex = x + (fTextureSize.width * y);
	return SetPixel(pixelIndex, pixel);
}

void
Texture::Reset()
{
    fPixelArray.reset();
    fPixelCount = 0;
    fTextureSize = {};
    fMaxColorValue = 0;
}

#if 0
bool
Texture::LoadFromPPM_(const std::filesystem::path& ppmPath)
{
    if (!std::filesystem::exists(ppmPath)) {
        std::cerr << "(Error) The texture doesn't exist at path: " << ppmPath << std::endl;
        return false;
    }

    std::ifstream fileReader(ppmPath);
    if (fileReader.fail()) {
        std::cerr << "(Error) Failed to open the texture for reading at path: " << ppmPath << std::endl;
        return false;
    }

    constexpr std::string_view kP3Type = "P3";

    std::string imageType;
    fileReader >> imageType;
    if (imageType != kP3Type) {
        std::cerr << "(Error) Unsupported PPM file type: " << imageType << std::endl;
        return false;
    }

    fileReader >> fTextureSize.width;
    fileReader >> fTextureSize.height;
    fileReader >> fMaxColorValue;

    if (fileReader.fail()) {
        std::cerr << "(Error) Failed to read PPM image header!" << std::endl;
        return false;
    }

    fPixelCount = fTextureSize.width * fTextureSize.height;
    fPixelArray = std::make_unique_for_overwrite<ColorRGB[]>(fPixelCount);

    // Terrible performance! Make this more efficient!
    for (std::size_t index = 0; index < fPixelCount; index++) {
        fileReader >> fPixelArray[index].red;
        fileReader >> fPixelArray[index].green;
        fileReader >> fPixelArray[index].blue;

        if (fileReader.fail()) {
            std::cerr << "(Error) Failed to read in pixel from file at index: " << index << std::endl;
            return false;
        }
    }

    return true;
}
#endif