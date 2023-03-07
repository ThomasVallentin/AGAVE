#ifndef TEXTURE_H
#define TEXTURE_H

#include "Base/Image.h"

#include "Base/Foundations.h"

#include <glad/glad.h>

class Texture;


DECLARE_PTR_TYPE(Texture);


class Texture
{
public:
    ~Texture();

    void Bind(const GLuint& unit) const;
    void Unbind() const;
    bool IsValid() const;
    inline GLuint GetId() const { return m_id; } 

    void SetData(void* data, const uint32_t& size, 
                 const GLenum& dataFormat = GL_RGBA,
                 const GLenum& dataType = GL_FLOAT) const;
    void SetData(const uint32_t& width, const uint32_t& height, void* data,
                 const GLenum& internalFormat = GL_RGBA8, 
                 const GLenum& dataFormat = GL_RGBA, 
                 const GLenum& dataType = GL_FLOAT);

    static TexturePtr Create();
    static TexturePtr Create(const uint32_t &width, 
                             const uint32_t &height,
                             const GLenum& internalFormat);
    static TexturePtr Open(const std::string& path, 
                           const ColorSpace& colorSpace=ColorSpace::None,
                           const GLenum& internalFormat = GL_RGBA8);
    static TexturePtr FromImage(const ImagePtr& image, const GLenum& internalFormat = GL_RGBA8);

    static void BindFromId(const GLuint& id, const GLuint& unit);

private:
    Texture();
    Texture(const uint32_t &width, const uint32_t &height,
            const GLenum& internalFormat);

    GLuint m_id = 0;
    
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    GLenum m_internalFormat; 
};

#endif  // TEXTURE_H
