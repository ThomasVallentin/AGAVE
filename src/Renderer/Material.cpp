#include "Material.h"

#include "Base/Logging.h"

const char* MATERIAL_BLOCK_NAME = "MaterialInputs";

// This variable describes the index of the textures for every input described by the material
// it should be replaced by a description of the shader's material (in a json file for example)
static const std::unordered_map<std::string, uint32_t> MATERIAL_TEXTURE_MAPPING = {
    {"baseColor", 0}, 
    {"metallic", 1},
    {"roughness", 2},
    {"transmissionColor", 3},
    {"emissionColor", 4}
};


Material::Material()
{
}

Material::Material(const ShaderPtr& shader) :
        m_shader(shader), 
        m_uniformBlock(shader->GetUniformBlockDescription(MATERIAL_BLOCK_NAME))
{
    m_uniformBuffer = UniformBuffer::Create(m_uniformBlock.size);

    m_textureBindings.resize(MATERIAL_TEXTURE_MAPPING.size());
}

void Material::Bind() const
{
    m_shader->Bind();
}

void Material::Unbind() const
{
    m_shader->Unbind();
    m_uniformBuffer->Detach(m_uniformBlock.index);
}

void Material::SetInputTexture(const std::string& name, const TexturePtr& texture)
{
    std::string useTextureName = name + "UseTexture";
    for (auto uniform=m_uniformBlock.uniforms.begin() ; uniform != m_uniformBlock.uniforms.end() ; uniform++)
    {
        if (uniform->name == useTextureName)
        {
            m_uniformBuffer->Bind();
            bool value = true;
            m_uniformBuffer->SetData(&value, sizeof(bool), uniform->offset);
            m_uniformBuffer->Unbind();
        }
    }

    const auto binding = MATERIAL_TEXTURE_MAPPING.find(name);
    if (binding != MATERIAL_TEXTURE_MAPPING.end())
    {
        m_textureBindings[binding->second] = texture->GetId();
    }
}

void Material::RemoveInputTexture(const std::string& name)
{
    std::string useTextureName = name + "UseTexture";
    for (auto uniform=m_uniformBlock.uniforms.begin() ; uniform != m_uniformBlock.uniforms.end() ; uniform++)
    {
        if (uniform->name == useTextureName)
        {
            m_uniformBuffer->Bind();
            bool value = false;
            m_uniformBuffer->SetData(&value, sizeof(bool), uniform->offset);
            m_uniformBuffer->Unbind();
        }
    }

    const auto binding = MATERIAL_TEXTURE_MAPPING.find(name);
    if (binding != MATERIAL_TEXTURE_MAPPING.end())
    {
        m_textureBindings[binding->second] = 0;
    }
}

void Material::SetInputValue(const std::string& name, const uint32_t& size, const void* value) const
{
    for (auto uniform=m_uniformBlock.uniforms.begin() ; uniform != m_uniformBlock.uniforms.end() ; uniform++)
    {
        if (uniform->name == name)
        {
            m_uniformBuffer->Bind();
            m_uniformBuffer->SetData(value, size, uniform->offset);
            m_uniformBuffer->Unbind();
        }
    }
}

void Material::ApplyUniforms() const 
{
    m_uniformBuffer->Attach(m_uniformBlock.index);
    
    std::string uTexture = "uTexture[";
    for (size_t i=0 ; i < m_textureBindings.size() ; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureBindings[i]);
        m_shader->SetInt(uTexture + std::to_string(i) + "]", i);
    }
}

template <>
void Material::SetInputValue<glm::vec2>(const std::string& name, const glm::vec2& value) const
{
    SetInputValue(name, sizeof(glm::vec2), glm::value_ptr(value));
}

template <>
void Material::SetInputValue<glm::vec3>(const std::string& name, const glm::vec3& value) const
{
    SetInputValue(name, sizeof(glm::vec3), glm::value_ptr(value));
}

template <>
void Material::SetInputValue<glm::vec4>(const std::string& name, const glm::vec4& value) const
{
    SetInputValue(name, sizeof(glm::vec4), glm::value_ptr(value));
}

template <>
void Material::SetInputValue<glm::mat3>(const std::string& name, const glm::mat3& value) const
{
    SetInputValue(name, sizeof(glm::mat3), glm::value_ptr(value));
}

template <>
void Material::SetInputValue<glm::mat4>(const std::string& name, const glm::mat4& value) const
{
    SetInputValue(name, sizeof(glm::mat4), glm::value_ptr(value));
}

MaterialPtr Material::Create(const ShaderPtr& shader) 
{
    return MaterialPtr(new Material(shader));
}
