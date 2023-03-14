#ifndef UI_RESOURCES_H
#define UI_RESOURCES_H

#include "Renderer/Texture.h"




class Resources
{
public:
    static const TexturePtr GetTexture(const fs::path& path);

private:
    using TextureRegistry = std::unordered_map<std::string, TexturePtr>;
    static TextureRegistry s_registry;
};


#endif // GLOBAL_H