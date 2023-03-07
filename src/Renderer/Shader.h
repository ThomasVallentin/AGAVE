#ifndef SHADER_H
#define SHADER_H

#include "Base/Foundations.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>

class Shader;

DECLARE_PTR_TYPE(Shader);


struct UniformBlockVariable
{
    std::string name;
    GLenum type;
    uint32_t count;
    uint32_t offset;
};


struct UniformBlockDescription
{
    uint32_t index;
    uint32_t size;
    std::vector<UniformBlockVariable> uniforms;
};


class Shader {
public:
    ~Shader();

    void Bind() const;
    void Unbind() const;
    bool IsValid() const;

    void SetBool(const std::string& name, const bool& value) const;
    void SetInt(const std::string& name, const int& value) const;
    void SetFloat(const std::string& name, const float& value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat3(const std::string& name, const glm::mat3& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;

    UniformBlockDescription GetUniformBlockDescription(const std::string& blockName);

    static ShaderPtr Create(const char* vertexCode, 
                                          const char* fragmentCode,
                                          const char* tessCtrlCode=nullptr,
                                          const char* tessEvalCode=nullptr,
                                          const char* geometryCode=nullptr);
    static ShaderPtr Open(const std::string& vertexPath,
                                        const std::string& fragmentPath,
                                        const std::string& tessCtrlPath="",
                                        const std::string& tessEvalPath="",
                                        const std::string& geometryPath=""
                                        );
    
    static const char* ShaderTypeToText(const GLint &type);

private:
    Shader();
    Shader(const char* vertexCode, 
           const char* fragmentCode,
           const char* tessCtrlCode,
           const char* tessEvalCode,
           const char* geometryCode);

    bool CompileShader(const GLint &type, const char* shaderSource, GLuint& outId) const;
    
    GLuint m_id = 0;
};

#endif  // SHADER_H
