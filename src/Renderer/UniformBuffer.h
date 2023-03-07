#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#include "Base/Foundations.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>


class UniformBuffer;

DECLARE_PTR_TYPE(UniformBuffer);


class UniformBuffer
{
public:
    ~UniformBuffer();

    void Attach(const uint32_t& index) const;
    void Detach(const uint32_t& index) const;

    void Bind() const;
    void Unbind() const;
    bool IsValid() const;

    void SetData(const void* data, const uint32_t& size, const uint32_t& offset) const;

    static UniformBufferPtr Create(const uint32_t& size);

private:
    UniformBuffer(const uint32_t& size);

    GLuint m_id;
};

#endif  // UNIFORMBUFFER_H