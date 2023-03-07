#include "Shader.h"

#include "Base/FileUtils.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>


Shader::Shader()
{
}

Shader::Shader(const char* vertexCode, 
               const char* fragmentCode,
               const char* tessCtrlCode,
               const char* tessEvalCode,
               const char* geometryCode) : m_id(0)
{
    GLuint vShader;
    GLuint fShader;
    GLuint tcShader = 0;
    GLuint teShader = 0;
    GLuint gShader = 0;
    if (!CompileShader(GL_VERTEX_SHADER, vertexCode, vShader)) 
        return;
    if (!CompileShader(GL_FRAGMENT_SHADER, fragmentCode, fShader)) 
        return;
    if ((tessCtrlCode && tessEvalCode)) {
        CompileShader(GL_TESS_CONTROL_SHADER, tessCtrlCode, tcShader);
        CompileShader(GL_TESS_EVALUATION_SHADER, tessEvalCode, teShader);
    }
    if (geometryCode) {
        CompileShader(GL_GEOMETRY_SHADER, geometryCode, gShader);
    }

    m_id = glCreateProgram();
    glAttachShader(m_id, vShader);
    glAttachShader(m_id, fShader);
    if (tcShader && teShader) {
        glAttachShader(m_id, tcShader);
        glAttachShader(m_id, teShader);
    }
    if (gShader) {
        glAttachShader(m_id, gShader);
    }

    GLint success = 0;
    glLinkProgram(m_id);
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);
        
        std::vector<char> log(length);
        glGetProgramInfoLog(fShader, length, &length, log.data());
        fprintf(stderr, "ERROR: Program linking failed with message :\n%s\n", log.data());

        m_id = 0;
        return;
    }

    // Cleanup
    glDetachShader(m_id, vShader);
    glDetachShader(m_id, fShader);
    if (tcShader != 0 && teShader != 0) {
        glDetachShader(m_id, tcShader);
        glDetachShader(m_id, teShader);
    }
    if (gShader != 0) {
        glDetachShader(m_id, gShader);
    }
}

Shader::~Shader()
{
    glDeleteProgram(m_id);
}

ShaderPtr Shader::Create(const char* vertexCode, 
                                       const char* fragmentCode,
                                       const char* tessCtrlCode,
                                       const char* tessEvalCode,
                                       const char* geometryCode)
{
    return ShaderPtr(new Shader(vertexCode, fragmentCode, tessCtrlCode, tessEvalCode, geometryCode));
}

ShaderPtr Shader::Open(const std::string& vertexPath,
                                     const std::string& fragmentPath,
                                     const std::string& tessCtrlPath,
                                     const std::string& tessEvalPath,
                                     const std::string& geometryPath)
{
    std::string vertexCode;
    std::string fragmentCode;
    std::string tessCtrlCode;
    std::string tessEvalCode;
    std::string geometryCode;

    if (!ReadFile(vertexPath, vertexCode) || !ReadFile(fragmentPath, fragmentCode)) {
        return ShaderPtr(new Shader());
    }

    if (!tessCtrlPath.empty())
        ReadFile(tessCtrlPath, tessCtrlCode);
    if (!tessEvalPath.empty())
        ReadFile(tessEvalPath, tessEvalCode);
    if (!geometryPath.empty())
        ReadFile(geometryPath, geometryCode);

    return Create(vertexCode.c_str(), 
                  fragmentCode.c_str(),
                  !tessCtrlCode.empty()  ? tessCtrlCode.c_str()  : nullptr,
                  !tessEvalCode.empty()  ? tessEvalCode.c_str()  : nullptr,
                  !geometryCode.empty() ? geometryCode.c_str() : nullptr);
}

void Shader::Bind() const {
    glUseProgram(m_id);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

bool Shader::IsValid() const {
    return m_id != 0;
}

void Shader::SetInt(const std::string& name, const int& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(location, value);
}

void Shader::SetFloat(const std::string& name, const float& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1f(location, value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::SetMat3(const std::string& name, const glm::mat3& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

UniformBlockDescription Shader::GetUniformBlockDescription(const std::string& blockName)
{
    uint32_t blockIndex = glGetUniformBlockIndex(m_id, blockName.c_str());
    if (blockIndex == GL_INVALID_INDEX)
    {
        return {0, 0};
    }

    int blockSize;
    glGetActiveUniformBlockiv(m_id, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    int uCount;
    glGetActiveUniformBlockiv(m_id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uCount);   

    UniformBlockDescription layout{blockIndex, (uint32_t)blockSize};
    layout.uniforms.reserve(uCount);

    int uIndices[uCount];
    glGetActiveUniformBlockiv(m_id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uIndices);

    uint32_t uOffsets[uCount];
    glGetActiveUniformsiv(m_id, uCount, (GLuint*)uIndices, GL_UNIFORM_OFFSET, (int*)uOffsets);

    char name[128];
    int nameLength;
    int size;
    GLenum type;
    for (size_t i=0 ; i < uCount ; i++)
    {
        glGetActiveUniform(m_id, (GLuint)uIndices[i], 128, &nameLength, &size, &type, name);
        layout.uniforms.push_back({std::string(name, nameLength), type, (uint32_t)size, uOffsets[i]});
    }

    return layout;
}


bool Shader::CompileShader(const GLint &type, const char* shaderSource, GLuint& shaderId) const
{
    shaderId = glCreateShader(type);
    glShaderSource(shaderId, 1, &shaderSource, nullptr);
    glCompileShader(shaderId);
    
    GLint success = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
        
        std::vector<char> log(length);
        glGetShaderInfoLog(shaderId, length, &length, log.data());
        fprintf(stderr, "ERROR: %s compilation failed with message :\n%s\n", Shader::ShaderTypeToText(type), log.data());

        shaderId = -1;
        return false;
    }

    return true;
}


const char* Shader::ShaderTypeToText(const GLint &type)
{
    switch (type) {
        case GL_VERTEX_SHADER:
            return "Vertex Shader";
        case GL_FRAGMENT_SHADER:
            return "Fragment Shader";
        case GL_GEOMETRY_SHADER:
            return "Geometry Shader";
        case GL_TESS_CONTROL_SHADER:
            return "Tessellation Control Shader";
        case GL_TESS_EVALUATION_SHADER:
            return "Tessellation Evaluation Shader";
    }
    return "UnknownShaderType";
}