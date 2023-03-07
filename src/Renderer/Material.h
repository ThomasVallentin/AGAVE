#ifndef MATERIAL_H
#define MATERIAL_H

#include "Shader.h"
#include "Texture.h"
#include "UniformBuffer.h"

#include "Base/Foundations.h"

#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <unordered_map>
#include <any>


class Material;

DECLARE_PTR_TYPE(Material);

class Material 
{
public:
    ~Material() = default;

    void Bind() const;
    void Unbind() const;

    inline ShaderPtr GetShader() const { return m_shader; }
    inline UniformBufferPtr GetUniformBuffer() const { return m_uniformBuffer; }

    void SetInputTexture(const std::string& name, const TexturePtr& texture);
    void RemoveInputTexture(const std::string& name);
    
    template <typename T>
    void SetInputValue(const std::string& name, const T& value) const
    {
        SetInputValue(name, sizeof(T), &value);
    }

    void ApplyUniforms() const;

    static MaterialPtr Create(const ShaderPtr& shader);

private:
    explicit Material(const ShaderPtr& shader);
    Material();

    void SetInputValue(const std::string& name, const uint32_t& size, const void* value) const;

    ShaderPtr m_shader;
    UniformBufferPtr m_uniformBuffer;
    UniformBlockDescription m_uniformBlock;

    std::vector<uint32_t> m_textureBindings;
};


#endif  // MATERIAL_H
