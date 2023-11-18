// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

#include "TextureCache.hpp"

#include <memory>
#include <fstream>


std::once_flag TextureCache::sInitTextureCache;
std::unique_ptr<TextureCache> TextureCache::sTheTextureCache = nullptr;

TextureCache&
TextureCache::Instance()
{
    std::call_once(sInitTextureCache, [](){ sTheTextureCache = std::make_unique<TextureCache>(); });
    return *sTheTextureCache;
}


//void
//TextureCache::WaitForTextureLoad()
//{
//    for (auto& loader : fLoaderMap) {
//        std::cout << "Waiting for texture to load from file: " << loader.first << std::endl;
//        if (loader.second.joinable())
//            loader.second.join();
//    }
//}


bool
TextureCache::HasTexture(const std::filesystem::path& texturePath) const
{
    return fResourceMap.contains(texturePath)/* || fLoaderMap.contains(texturePath)*/;
}


bool
TextureCache::GetTexture(const std::filesystem::path& texturePath, SharedTexture& resourceOut)
{
    if (!HasTexture(texturePath))
        return false;

//    if (fLoaderMap.contains(texturePath))
//        fLoaderMap.at(texturePath).get_future().wait();

    if (!fResourceMap.contains(texturePath))
        return false;

    resourceOut = fResourceMap.at(texturePath);
    return true;
}


bool
TextureCache::LoadTexture(const std::filesystem::path& texturePath)
{
    // If the texture at texturePath is already loaded, don't load it again!
    if (HasTexture(texturePath))
        return true;

    // Check for the texture in a "texture" subfolder
    std::filesystem::path actualTexturePath = kTextureSubfolder;
    actualTexturePath /= texturePath;
    if (!std::filesystem::exists(actualTexturePath)) {
        std::cerr << "(Error) The texture doesn't exist at path: " << absolute(actualTexturePath) << std::endl;
        return false;
    }

//    const auto [iterator, success] = fLoaderMap.emplace(
//        texturePath, std::thread(&TextureCache::LoadTextureFromPPM_, this, texturePath));

    return LoadTextureFromPPM_(texturePath);
}


bool
TextureCache::LoadTextureFromPPM_(const std::filesystem::path& texturePath)
{
    // Look for texture in texture subfolder
    std::filesystem::path actualTexturePath = kTextureSubfolder;
    actualTexturePath /= texturePath.filename();

    std::ifstream fileReader(actualTexturePath);
    if (fileReader.fail()) {
        std::cerr << "(Error) Failed to open the texture for reading at path: " << absolute(actualTexturePath) << std::endl;
        return false;
    }

    constexpr std::string_view kP3Type = "P3";

    std::string imageType;
    fileReader >> imageType;
    if (imageType != kP3Type) {
        std::cerr << "(Error) Unsupported PPM file type: " << imageType << std::endl;
        return false;
    }

    Size textureSize;
    fileReader >> textureSize.width;
    fileReader >> textureSize.height;

    uint32_t maxColorValue;
    fileReader >> maxColorValue;

    if (fileReader.fail()) {
        std::cerr << "(Error) Failed to read PPM image header!" << std::endl;
        return false;
    }

    size_t pixelCount = textureSize.width * textureSize.height;
    auto pixelArray = std::make_unique_for_overwrite<ColorRGB[]>(pixelCount);

    // Terrible performance! Make this more efficient!
	std::string readValue;
    for (std::size_t index = 0; index < pixelCount; index++) {
		fileReader >> readValue;
		pixelArray[index].red = std::strtoul(readValue.c_str(), nullptr, 10);

		fileReader >> readValue;
		pixelArray[index].green = std::strtoul(readValue.c_str(), nullptr, 10);

		fileReader >> readValue;
		pixelArray[index].blue = std::strtoul(readValue.c_str(), nullptr, 10);

//		std::cerr << "Pixel at index #" << index << ", is: " << pixelArray[index] << std::endl;

        if (fileReader.fail()) {
            std::cerr << "(Error) Failed to read in pixel from file at index: " << index << std::endl;
            return false;
        }
    }

    auto texture = std::make_shared<Texture>();
    texture->SetPixelSize(textureSize);
    texture->SetMaxColorValue(maxColorValue);
    texture->MovePixelsIntoTexture(std::move(pixelArray), pixelCount);

    {
//        std::lock_guard<std::mutex> lock(fResourceMutex);
        auto [iterator, success] = fResourceMap.emplace(texturePath, std::move(texture));

        if (!success) {
            std::cerr << "(Error) Failed to move texture in resource map: " << texturePath << std::endl;
            return false;
        }
    }

//    {
//        std::lock_guard<std::mutex> lock(fLoaderMutex);
//        if (fLoaderMap.erase(texturePath) != 1) {
//            std::cerr << "(FATAL ERROR) Failed to erase loader entry from loader map, in inconsistent state!"
//                      << std::endl;
//            std::abort();
//        }
//    }

    return true;
}
