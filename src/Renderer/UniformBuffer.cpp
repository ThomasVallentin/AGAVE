#include "UniformBuffer.h"

#include <iostream>


UniformBuffer::UniformBuffer(const uint32_t& size)
{
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer()
{
    glDeleteBuffers(1, &m_id);
    m_id = 0;
}

void UniformBuffer::Attach(const uint32_t& index) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, index, m_id);
}

void UniformBuffer::Detach(const uint32_t& index) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
}

void UniformBuffer::Bind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);
}

void UniformBuffer::Unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool UniformBuffer::IsValid() const
{
    return m_id;
}

void UniformBuffer::SetData(const void* data, const uint32_t& size, const uint32_t& offset) const
{
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

UniformBufferPtr UniformBuffer::Create(const uint32_t& size)
{
    return UniformBufferPtr(new UniformBuffer(size));
}
