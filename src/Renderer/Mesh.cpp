#include "Mesh.h"

Mesh::Mesh() 
{
    CreateVertexArray();
}

Mesh::Mesh(const std::vector<Vertex>& vertices, 
           const std::vector<uint32_t>& indices) :
        m_vertices(vertices), m_indices(indices)
{
    CreateVertexArray();
}

void Mesh::SetVertices(const std::vector<Vertex> &vertices) 
{
    m_vertices = vertices;
    const auto& vBuffer = m_vertexArray->GetVertexBuffers()[0];
    vBuffer->Bind();
    vBuffer->SetData(m_vertices.data(), m_vertices.size() * sizeof(Vertex));
    vBuffer->Unbind();
}

void Mesh::SetIndices(const std::vector<uint32_t>& indices) 
{
    m_indices = indices;
    const auto& iBuffer = m_vertexArray->GetIndexBuffer();
    iBuffer->Bind();
    iBuffer->SetData(indices.data(), indices.size());
    iBuffer->Unbind();
}

void Mesh::CreateVertexArray() 
{
    VertexBufferPtr vBuffer = VertexBuffer::Create(m_vertices.data(), m_vertices.size() * sizeof(Vertex));
    vBuffer->SetLayout({{"aPosition",  3, GL_FLOAT, false},
                        {"aNormal",    3, GL_FLOAT, false},
                        {"aTexCoords", 2, GL_FLOAT, false}
                       });

    IndexBufferPtr iBuffer = IndexBuffer::Create(m_indices.data(), m_indices.size());
    
    m_vertexArray = VertexArray::Create();
    m_vertexArray->AddVertexBuffer(vBuffer);
    m_vertexArray->SetIndexBuffer(iBuffer);
    m_vertexArray->Unbind();
}

MeshPtr Mesh::Create() 
{
    return MeshPtr(new Mesh);
}


MeshPtr Mesh::Create(const std::vector<Vertex>& vertices, 
                     const std::vector<uint32_t>& indices) 
{
    return MeshPtr(new Mesh(vertices, indices));
}
