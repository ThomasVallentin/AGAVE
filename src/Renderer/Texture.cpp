#include "Texture.h"

#include "Base/Image.h"
#include "Base/Logging.h"


uint32_t ChannelsOfGLType(const GLenum& type) {
        switch (type) {
            case GL_RED: 
            case GL_R8:
            case GL_R32F:             return 1;

            case GL_RG: 
            case GL_RG8:
            case GL_RG32F:            return 2;

            case GL_RGB:
            case GL_RGB8: 
            case GL_RGB32F:           return 3;

            case GL_RGBA:
            case GL_RGBA8:
            case GL_RGBA32F:          return 4;

            default:                  return 0;
    }
}


Texture::Texture()
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    // Default wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Default filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}


Texture::Texture(const uint32_t& width, const uint32_t& height,
                 const GLenum& internalFormat) : 
        m_width(width),
        m_height(height),
        m_internalFormat(internalFormat)
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    // Reserve space on the GPU
    glTexStorage2D(GL_TEXTURE_2D, 1, m_internalFormat, width, height);

    // Default wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Default filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}


Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

void Texture::Bind(const GLuint& unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::BindFromId(const GLuint& id, const GLuint& unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}


bool Texture::IsValid() const
{
    return m_id;
}

void Texture::SetData(void* data, const uint32_t& size, 
                      const GLenum& dataFormat,
                      const GLenum& dataType) const
{
    uint32_t channels = ChannelsOfGLType(m_internalFormat);
    uint32_t internalSize = m_width * m_height * channels;
    ASSERT(size == internalSize, 
           "The passed data does not match the texture's requirements."
           "Expected %dx%dx%d=%d, got %d",
           m_width, m_height, channels, internalSize, size);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, dataFormat, dataType, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::SetData(const uint32_t& width, const uint32_t& height, void* data,
                      const GLenum& internalFormat, 
                      const GLenum& dataFormat,
                      const GLenum& dataType)
{
    m_width = width;
    m_height = height;
    m_internalFormat = internalFormat;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, dataType, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

TexturePtr Texture::Create()
{
    return TexturePtr(new Texture);
}

TexturePtr Texture::Create(const uint32_t& width, const uint32_t& height,
                           const GLenum& internalFormat)
{
    Texture* texture = new Texture(width, height, internalFormat);
    return TexturePtr(texture);
}

TexturePtr Texture::Open(const std::string& path, 
                         const ColorSpace& colorSpace,
                         const GLenum& internalFormat)
{
    return Texture::FromImage(Image::Read(path, colorSpace), internalFormat);
}

TexturePtr Texture::FromImage(const ImagePtr& image, const GLenum& internalFormat)
{
    if (image) {
        Texture* texture = new Texture();

        glBindTexture(GL_TEXTURE_2D, texture->m_id);
        texture->SetData(image->GetWidth(), 
                         image->GetHeight(),
                         (void*)image->GetPixels(),
                         internalFormat,
                         // dataFormat and dataType are fixed since we only consider images composed of glm::vec4. 
                         // This should change in the future when the Image implementation will be improved.
                         GL_RGBA,
                         GL_FLOAT);  
        glBindTexture(GL_TEXTURE_2D, 0);

        return TexturePtr(texture);
    }

    return Create();
}