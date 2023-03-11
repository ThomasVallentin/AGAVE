#include "Renderer.hpp"

#include "C3GAUtils.hpp"

#include "Base/Resolver.h"
#include "Base/Logging.h"

#include <glm/glm.hpp>


Renderer::Renderer()
{
    m_spheres.vertexArray = VertexArray::Create();
    m_spheres.drawType = GL_TRIANGLES;

    m_planes.vertexArray = VertexArray::Create();
    m_planes.drawType = GL_TRIANGLES;

    m_circles.vertexArray = VertexArray::Create();
    m_circles.drawType = GL_LINE_STRIP;

    // Line
    glm::vec3 line[] = {{0, 0, 0}, {0, 0, 1}};
    uint32_t lineIndices[] = {0, 1};

    auto lineVbo = VertexBuffer::Create(line, sizeof(line));
    lineVbo->SetLayout({{"Position",  3, GL_FLOAT, false},
                        {"Normal",    3, GL_FLOAT, false},
                        {"TexCoords", 2, GL_FLOAT, false}});

    auto lineIbo = VertexBuffer::Create();
    lineIbo->SetLayout({{"Transform", 16, GL_FLOAT, false}});

    auto lineEbo = IndexBuffer::Create(lineIndices, 2);

    m_lines.vertexArray = VertexArray::Create();
    m_lines.vertexArray->AddVertexBuffer(lineVbo);
    m_lines.vertexArray->AddVertexBuffer(lineIbo);
    m_lines.vertexArray->SetIndexBuffer(lineEbo);
    m_lines.drawType = GL_LINE_STRIP;

    // Points
    auto pointsVbo = VertexBuffer::Create();
    pointsVbo->SetLayout({{"Position",  3, GL_FLOAT, false}});
    m_points = VertexArray::Create();
    m_points->AddVertexBuffer(pointsVbo);

    // Shaders
    auto& resolver = Resolver::Get();
    m_shader = Shader::Open(resolver.Resolve("resources/shaders/points.vert"),
                            resolver.Resolve("resources/shaders/points.frag"));
}

Renderer::~Renderer()
{

}

void Renderer::Invalidate()
{
    m_isValid = false;
}

void Renderer::Render(const LayerStackPtr& layerStack,
                      const glm::mat4& viewProjMatrix) 
{
    if (!m_isValid)
    {
        BuildBatches(layerStack);
    }

    // m_spheres.Render();
    // m_planes.Render();
    // m_circles.Render();
    // m_lines.Render();

    m_shader->Bind();
    m_shader->SetMat4("uViewProjMatrix", viewProjMatrix);

    m_points->Bind();
    glDrawArrays(GL_POINTS, 0, m_pointCount);
}

void Renderer::BuildBatches(const LayerStackPtr& layerStack) 
{
    std::vector<glm::vec3> points;
    for (const auto& layer : layerStack->GetLayers())
    {
        if (!layer->IsVisible()) {
            continue;
        }

        for (const auto& obj : layer->GetObjects())
        {
            switch (c3ga::getTypeOf(obj))
            {
                case c3ga::MvecType::Point: {
                    points.push_back({obj[c3ga::E1], obj[c3ga::E2], obj[c3ga::E3]});
                    break;
                }

                case c3ga::MvecType::Sphere: {
                    // Temporary rendering points instead of sphere for debug purpose
                    auto dual = obj.dual();
                    points.push_back({dual[c3ga::E1], dual[c3ga::E2], dual[c3ga::E3]});
                    break;
                }

                case c3ga::MvecType::Circle: {
                    break;
                }

                case c3ga::MvecType::Line: {
                    break;
                }
            }
        }
    }

    m_pointCount = points.size();
    m_points->GetVertexBuffers()[0]->SetData(points.data(), m_pointCount * sizeof(glm::vec3));
}


void Renderer::Batch::Render() const
{
    if (!instanceCount)
        return;
        
    if (auto indexBuffer = vertexArray->GetIndexBuffer()) {
        vertexArray->Bind();
        glDrawElementsInstanced(GL_TRIANGLES, 
                                indexBuffer->GetCount(), 
                                GL_UNSIGNED_INT, 
                                nullptr, 
                                instanceCount);
    }

}
