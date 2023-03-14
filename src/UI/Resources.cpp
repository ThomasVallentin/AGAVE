#include "Resources.hpp"

#include "Base/Resolver.h"


Resources::TextureRegistry Resources::s_registry;


const TexturePtr Resources::GetTexture(const fs::path& path)
{
    fs::path resolvedPath = Resolver::Get().Resolve(path);

    auto it = s_registry.find(resolvedPath);
    if (it != s_registry.end())
    {
        return it->second;
    }

    TexturePtr texture = Texture::Open(resolvedPath, ColorSpace::Raw);
    s_registry[resolvedPath] = texture;
    
    return texture;
}
