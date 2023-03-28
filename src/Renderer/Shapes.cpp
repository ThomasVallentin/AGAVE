#include "Shapes.hpp"
#include "Mesh.h"

#include <glm/glm.hpp>

#include <glad/glad.h>


namespace Shapes {


VertexArrayPtr InstanciableSphere(const float& radius, 
                                  const uint32_t& wSubdivs, 
                                  const uint32_t& hSubdivs)
{
    std::vector<Vertex> points;
    std::vector<uint32_t> indices;
    for (uint32_t i=0 ; i < wSubdivs + 1 ; ++i) {
        const float latitude = i * M_PI / wSubdivs;
        for (uint32_t j=0 ; j < hSubdivs ; ++j) {
            const float longitude = j * 2.0 * M_PI / wSubdivs;
            glm::vec3 position = {radius * std::sin(latitude) * std::cos(longitude),
                                  radius * std::sin(latitude) * std::sin(longitude),
                                  radius * std::cos(latitude)};
            points.push_back({position, glm::normalize(position)});
            indices.push_back(hSubdivs * i + j);
            indices.push_back(hSubdivs * (i + 1) + j);
        }
    }

    auto vertexBuffer = VertexBuffer::Create(points.data(), points.size() * sizeof(Vertex));
    vertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false},
                             {"Normal",    3, GL_FLOAT, false},
                             {"TexCoord",  2, GL_FLOAT, false}});

    auto instancedBuffer = VertexBuffer::Create();
    instancedBuffer->SetLayout({{"Transform[0]", 4, GL_FLOAT, false},
                                {"Transform[1]", 4, GL_FLOAT, false},
                                {"Transform[2]", 4, GL_FLOAT, false},
                                {"Transform[3]", 4, GL_FLOAT, false}});

    auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());

    auto vertexArray = VertexArray::Create();
    vertexArray->AddVertexBuffer(vertexBuffer);
    vertexArray->AddVertexBuffer(instancedBuffer);
    vertexArray->SetIndexBuffer(indexBuffer);
    vertexArray->Bind();
    glVertexAttribDivisor(3, 1); 
    glVertexAttribDivisor(4, 1); 
    glVertexAttribDivisor(5, 1); 
    glVertexAttribDivisor(6, 1); 
    vertexArray->Unbind();

    return vertexArray;
}

VertexArrayPtr InstanciableCircle(const float& radius, const uint32_t& subdivs)
{
    std::vector<glm::vec3> points;
    std::vector<uint32_t> indices;
    for (uint32_t i=0 ; i < subdivs ; ++i) {
        points.push_back(glm::vec3(std::cos(2.0 * M_PI / subdivs * i), 
                                   std::sin(2.0 * M_PI / subdivs * i),
                                   0.0f));
        indices.push_back(i);
    }

    auto vertexBuffer = VertexBuffer::Create(points.data(), points.size() * sizeof(glm::vec3));
    vertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false}});

    auto instancedBuffer = VertexBuffer::Create();
    instancedBuffer->SetLayout({{"Transform[0]", 4, GL_FLOAT, false},
                                {"Transform[1]", 4, GL_FLOAT, false},
                                {"Transform[2]", 4, GL_FLOAT, false},
                                {"Transform[3]", 4, GL_FLOAT, false}});

    auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());

    auto vertexArray = VertexArray::Create();
    vertexArray->AddVertexBuffer(vertexBuffer);
    vertexArray->AddVertexBuffer(instancedBuffer);
    vertexArray->SetIndexBuffer(indexBuffer);
    vertexArray->Bind();
    glVertexAttribDivisor(1, 1); 
    glVertexAttribDivisor(2, 1); 
    glVertexAttribDivisor(3, 1); 
    glVertexAttribDivisor(4, 1); 
    vertexArray->Unbind();

    return vertexArray;
}

VertexArrayPtr InstanciablePlane(const float& width, const float& height)
{
    std::vector<Vertex> points = {{{-width * 0.5f, 0.0f, -width * 0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{ width * 0.5f, 0.0f, -width * 0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{ width * 0.5f, 0.0f,  width * 0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{-width * 0.5f, 0.0f,  width * 0.5f}, {0.0f, 1.0f, 0.0f}}};
    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

    auto vertexBuffer = VertexBuffer::Create(points.data(), points.size() * sizeof(Vertex));
    vertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false},
                             {"Normal",    3, GL_FLOAT, false},
                             {"TexCoord",  2, GL_FLOAT, false}});

    auto instancedBuffer = VertexBuffer::Create();
    instancedBuffer->SetLayout({{"Transform[0]", 4, GL_FLOAT, false},
                                {"Transform[1]", 4, GL_FLOAT, false},
                                {"Transform[2]", 4, GL_FLOAT, false},
                                {"Transform[3]", 4, GL_FLOAT, false}});

    auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());

    auto vertexArray = VertexArray::Create();
    vertexArray->AddVertexBuffer(vertexBuffer);
    vertexArray->AddVertexBuffer(instancedBuffer);
    vertexArray->SetIndexBuffer(indexBuffer);
    vertexArray->Bind();
    glVertexAttribDivisor(3, 1); 
    glVertexAttribDivisor(4, 1); 
    glVertexAttribDivisor(5, 1); 
    glVertexAttribDivisor(6, 1); 
    vertexArray->Unbind();

    return vertexArray;
}


VertexArrayPtr InstanciableLine(const float& length)
{
    std::vector<glm::vec3> points = {{0.0f, 0.0f, -length * 0.5f}, {0.0f, 0.0f, length * 0.5f}};
    std::vector<uint32_t> indices = {0, 1};

    auto vertexBuffer = VertexBuffer::Create(points.data(), points.size() * sizeof(glm::vec3));
    vertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false}});

    auto instancedBuffer = VertexBuffer::Create();
    instancedBuffer->SetLayout({{"Transform[0]", 4, GL_FLOAT, false},
                                {"Transform[1]", 4, GL_FLOAT, false},
                                {"Transform[2]", 4, GL_FLOAT, false},
                                {"Transform[3]", 4, GL_FLOAT, false}});

    auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());

    auto vertexArray = VertexArray::Create();
    vertexArray->AddVertexBuffer(vertexBuffer);
    vertexArray->AddVertexBuffer(instancedBuffer);
    vertexArray->SetIndexBuffer(indexBuffer);
    vertexArray->Bind();
    glVertexAttribDivisor(1, 1); 
    glVertexAttribDivisor(2, 1); 
    glVertexAttribDivisor(3, 1); 
    glVertexAttribDivisor(4, 1); 
    vertexArray->Unbind();

    return vertexArray;
}


}  // namespace Shapes
