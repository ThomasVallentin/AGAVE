#include "FrameBuffer.h"

#include "Texture.h"
#include "Base/Logging.h"


void InternalFormatToDataFormat(const GLenum& format, GLenum& dataFormat, GLenum& dataType) {
    switch (format) {
        case GL_RED:            
        case GL_R8:                  dataFormat = GL_RED; dataType = GL_UNSIGNED_INT; break;
        case GL_R16F:                dataFormat = GL_RED; dataType = GL_HALF_FLOAT; break;
        case GL_R32F:                dataFormat = GL_RED; dataType = GL_FLOAT; break;

        case GL_RG: 
        case GL_RG8:                 dataFormat = GL_RG; dataType = GL_UNSIGNED_INT; break;
        case GL_RG16F:               dataFormat = GL_RG; dataType = GL_HALF_FLOAT; break;
        case GL_RG32F:               dataFormat = GL_RG; dataType = GL_FLOAT; break;

        case GL_RGB:
        case GL_RGB8:                dataFormat = GL_RGB; dataType = GL_UNSIGNED_INT; break;
        case GL_RGB16F:              dataFormat = GL_RGB; dataType = GL_HALF_FLOAT; break;
        case GL_RGB32F:              dataFormat = GL_RGB; dataType = GL_FLOAT; break;

        case GL_RGBA:
        case GL_RGBA8:               dataFormat = GL_RGBA; dataType = GL_UNSIGNED_INT; break;
        case GL_RGBA16F:             dataFormat = GL_RGBA; dataType = GL_HALF_FLOAT; break;
        case GL_RGBA32F:             dataFormat = GL_RGBA; dataType = GL_FLOAT; break;

        case GL_DEPTH_COMPONENT24:   dataFormat = GL_DEPTH_COMPONENT; dataType = GL_UNSIGNED_INT; break;
        case GL_DEPTH_COMPONENT32F:  dataFormat = GL_DEPTH_COMPONENT; dataType = GL_FLOAT; break;

        case GL_DEPTH24_STENCIL8:    dataFormat = GL_DEPTH_STENCIL; dataType = GL_UNSIGNED_INT_24_8; break;
        case GL_DEPTH32F_STENCIL8:   dataFormat = GL_DEPTH_STENCIL; dataType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV; break;
    }
}

FrameBuffer::FrameBuffer(const FrameBufferSpecs& specs) :
    m_specs(specs)
{
    GenerateAttachments();
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &m_id);
}

void FrameBuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glViewport(0, 0, m_specs.width, m_specs.height);
}

void FrameBuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FrameBuffer::IsValid() const
{
    return m_id;
}

GLuint FrameBuffer::GetColorAttachmentId(const uint32_t& index) const
{
    ASSERT_OR_RETURN(index < m_colorAttachments.size(), 0, 
                     "Invalid FrameBuffer texture Id %d (only %d available)", 
                     index, 
                     m_colorAttachments.size())

    return m_colorAttachments[index];
}

GLuint FrameBuffer::GetDepthAttachmentId() const 
{
    return m_depthAttachment;
}

void FrameBuffer::Resize(const uint32_t& width, const uint32_t& height)
{
    m_specs.width = width;
    m_specs.height = height;

    GenerateAttachments();
}

void FrameBuffer::Blit(const GLuint& destFrameBufferId, const uint32_t& destWidth, const uint32_t& destHeight) const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFrameBufferId);
    glBlitFramebuffer(0, 0, m_specs.width, m_specs.height, 
                      0, 0, destWidth, destHeight, 
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Restore the current framebuffer as the drawing buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_id);
}

void FrameBuffer::AttachTexture(const GLenum& slot, const GLuint id, 
                                const GLenum& internalFormat)
{
    GLenum dataFormat, dataType;
    InternalFormatToDataFormat(internalFormat, dataFormat, dataType);

    bool multisampled =  m_specs.samples > 1;
    if (multisampled)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, id);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_specs.samples, internalFormat, m_specs.width, m_specs.height, GL_FALSE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, slot, GL_TEXTURE_2D_MULTISAMPLE, id, 0);
    }
    else
    {
        // We only initialize the texture data by passing a nullptr
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_specs.width, m_specs.height, 0, dataFormat, dataType, nullptr);

        glFramebufferTexture2D(GL_FRAMEBUFFER, slot, GL_TEXTURE_2D, id, 0);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void FrameBuffer::GenerateAttachments()
{
    // Cleanup the previous buffers
    if (m_id) 
    {
        glDeleteFramebuffers(1, &m_id);
        glDeleteTextures(m_colorAttachments.size(), m_colorAttachments.data());
        glDeleteTextures(1, &m_depthAttachment);

        m_id = 0;
        m_colorAttachments.clear();
        m_depthAttachment = 0;
    }

    glGenFramebuffers(1, &m_id); 
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);

    // Fill the new framebuffer with textures that matches the specs
    if (!m_specs.colorFormats.empty())
    {
        m_colorAttachments.resize(m_specs.colorFormats.size());
        glGenTextures(m_colorAttachments.size(), m_colorAttachments.data());

        GLenum drawBuffers[m_colorAttachments.size()];
        for (size_t i=0 ; i < m_colorAttachments.size() ; i++ )
        {
            AttachTexture(GL_COLOR_ATTACHMENT0 + i, m_colorAttachments[i], m_specs.colorFormats[i]);
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        glDrawBuffers(m_colorAttachments.size(), drawBuffers);
    } 
    else
    {
        glDrawBuffer(GL_NONE);
    }

    if (m_specs.depthFormat) 
    {
        glGenTextures(1, &m_depthAttachment);
        AttachTexture(GL_DEPTH_STENCIL_ATTACHMENT, m_depthAttachment, m_specs.depthFormat);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBufferPtr FrameBuffer::Create(const FrameBufferSpecs& specs)
{
    return FrameBufferPtr(new FrameBuffer(specs));
}

void FrameBuffer::BindFromId(const GLuint& id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}
