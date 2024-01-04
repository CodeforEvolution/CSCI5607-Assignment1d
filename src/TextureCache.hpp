// Assignment 1d - Transparency and Mirror Reflections
// Work by Jacob Secunda
#ifndef TEXTURE_CACHE_H
#define TEXTURE_CACHE_H

#include <filesystem>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include "core/Texture.hpp"

class TextureCache {
public:
    TextureCache() = default;
    virtual ~TextureCache() = default;
    // Delete copy constructors...
    TextureCache(const TextureCache& other) = delete;

    // Global Instance
    static TextureCache& Instance();

    [[nodiscard]] bool HasTexture(const std::filesystem::path& texturePath) const;

    // Will block if Texture corresponding to the texture path is still loading.
    bool GetTexture(const std::filesystem::path& texturePath, SharedTexture& resourceOut);

    // Asynchronous function that starts loading a texture from texturePath...
    bool LoadTexture(const std::filesystem::path& texturePath);
//    void WaitForTextureLoad();

private:
    static constexpr std::string_view kTextureSubfolder = "texture/";

    static std::once_flag sInitTextureCache;
    static std::unique_ptr<TextureCache> sTheTextureCache;

//    std::mutex fLoaderMutex;
//    std::map<std::filesystem::path, std::promise<Texture>> fLoaderMap;

    std::mutex fResourceMutex;
    std::map<std::filesystem::path, std::shared_ptr<Texture>> fResourceMap;

    bool LoadTextureFromPPM_(const std::filesystem::path& texturePath);
};

#endif // TEXTURE_CACHE_H