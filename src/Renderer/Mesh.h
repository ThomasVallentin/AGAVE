#ifndef MESH_H
#define MESH_H

#include "Base/Logging.h"

#include "VertexArray.h"

#include <glm/glm.hpp>


class Mesh;

DECLARE_PTR_TYPE(Mesh);


struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;

    inline bool operator==(const Vertex& other) const
    {
        return (position == other.position && normal == other.normal && texCoords == other.texCoords);
    }
};


class Mesh
{
public:
    ~Mesh() = default;

    inline const std::vector<Vertex> &GetVertices() const { return m_vertices; }
    void SetVertices(const std::vector<Vertex> &vertices);

    inline const std::vector<uint32_t> &GetIndices() const { return m_indices; }
    void SetIndices(const std::vector<uint32_t> &indices);

    inline void Bind() const { m_vertexArray->Bind(); }
    inline void Unbind() const { m_vertexArray->Unbind(); }
    inline uint32_t GetElementCount() const { return m_vertexArray->GetIndexBuffer()->GetCount(); }

    static MeshPtr Create();
    static MeshPtr Create(const std::vector<Vertex> &vertices, 
                          const std::vector<uint32_t> &indices);

private:
    Mesh();
    explicit Mesh(const std::vector<Vertex> &vertices, 
                  const std::vector<uint32_t> &indices);
    void CreateVertexArray();

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    VertexArrayPtr m_vertexArray;
};


#endif // MESH_H
