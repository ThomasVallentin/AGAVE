#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H


#include "Base/Foundations.h"

#include <glad/glad.h>

#include <vector>


class FrameBuffer;

DECLARE_PTR_TYPE(FrameBuffer);


struct FrameBufferSpecs 
{
    uint32_t width, height;
    uint32_t samples = 1;

    std::vector<GLenum> colorFormats = {GL_RGBA8};
    GLenum depthFormat = GL_DEPTH24_STENCIL8;
};


class FrameBuffer
{
public:
    ~FrameBuffer();

    void Bind() const;
    void Unbind() const;
    bool IsValid() const;
    inline GLuint GetId() const { return m_id; }

    inline uint32_t GetWidth() const { return m_specs.width; }
    inline uint32_t GetHeight() const { return m_specs.height; }
    inline FrameBufferSpecs GetSpecs() const { return m_specs; }

    GLuint GetColorAttachmentId(const uint32_t& index) const;
    GLuint GetDepthAttachmentId() const;

    void Resize(const uint32_t& width, const uint32_t& height);
    void Blit(const GLuint& destFrameBufferId, const uint32_t& destWidth, const uint32_t& destHeight) const;

    static FrameBufferPtr Create(const FrameBufferSpecs& specs);

    static void BindFromId(const GLuint& id);

private:
    FrameBuffer(const FrameBufferSpecs& specs);
    
    void AttachTexture(const GLenum& slot, const GLuint id, 
                       const GLenum& internalFormat);
    void GenerateAttachments();

    GLuint m_id = 0;
    
    // Framebuffer specifications
    FrameBufferSpecs m_specs;
    
    // Texture attachments ids
    std::vector<GLuint> m_colorAttachments;
    GLuint m_depthAttachment;
};

#endif  // FRAMEBUFFER_H