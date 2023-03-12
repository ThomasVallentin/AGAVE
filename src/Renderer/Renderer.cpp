#include "Renderer.hpp"

#include "C3GAUtils.hpp"

#include "Base/Resolver.h"
#include "Base/Logging.h"

#include <glm/glm.hpp>


Renderer::Renderer()
{
    m_spheres.vertexArray = VertexArray::Create();
    uint32_t longSubdiv = 32;
    uint32_t latSubdiv = 32;
    float radius = 1.0;

    std::vector<glm::vec3> spherePoints;
    std::vector<uint32_t> sphereIndices;
    for (uint32_t i=0 ; i < latSubdiv ; ++i) {
        const float latitude = i * M_PI / latSubdiv;
        for (uint32_t j=0 ; j < longSubdiv ; ++j) {
            const float longitude = j * 2.0 * M_PI / latSubdiv;
            spherePoints.push_back({radius * std::sin(latitude) * std::cos(longitude),
                                    radius * std::sin(latitude) * std::sin(longitude),
                                    radius * std::cos(latitude)});
            sphereIndices.push_back(longSubdiv * i + j);
            sphereIndices.push_back(longSubdiv * (i + 1) + j);
        }
    }

    auto sphereVbo = VertexBuffer::Create(spherePoints.data(), spherePoints.size() * sizeof(glm::vec3));
    sphereVbo->SetLayout({{"Position",  3, GL_FLOAT, false}});

    auto sphereIbo = VertexBuffer::Create();
    sphereIbo->SetLayout({{"Transform[0]", 4, GL_FLOAT, false},
                          {"Transform[1]", 4, GL_FLOAT, false},
                          {"Transform[2]", 4, GL_FLOAT, false},
                          {"Transform[3]", 4, GL_FLOAT, false}});

    auto sphereEbo = IndexBuffer::Create(sphereIndices.data(), sphereIndices.size());

    m_spheres.vertexArray = VertexArray::Create();
    m_spheres.vertexArray->AddVertexBuffer(sphereVbo);
    m_spheres.vertexArray->AddVertexBuffer(sphereIbo);
    m_spheres.vertexArray->SetIndexBuffer(sphereEbo);
    m_spheres.vertexArray->Bind();
    glVertexAttribDivisor(1, 1); 
    glVertexAttribDivisor(2, 1); 
    glVertexAttribDivisor(3, 1); 
    glVertexAttribDivisor(4, 1); 
    m_spheres.vertexArray->Unbind();

    m_spheres.drawType = GL_TRIANGLE_STRIP;

    // Planes
    m_planes.vertexArray = VertexArray::Create();
    m_planes.drawType = GL_TRIANGLES;

    // Circles
    uint32_t circleSubdivs = 64;
    std::vector<glm::vec3> circle;
    std::vector<uint32_t> circleIndices;
    for (uint32_t i=0 ; i < circleSubdivs - 1 ; ++i) {
        circle.push_back(glm::vec3(std::cos(2.0 * M_PI / circleSubdivs * i), 0.0f, std::sin(2.0 * M_PI / circleSubdivs * i)));
        circleIndices.push_back(i);
    }

    auto circleVbo = VertexBuffer::Create(circle.data(), circle.size() * sizeof(glm::vec3));
    circleVbo->SetLayout({{"Position",  3, GL_FLOAT, false}});

    auto circleIbo = VertexBuffer::Create();
    circleIbo->SetLayout({{"Transform[0]", 4, GL_FLOAT, false},
                          {"Transform[1]", 4, GL_FLOAT, false},
                          {"Transform[2]", 4, GL_FLOAT, false},
                          {"Transform[3]", 4, GL_FLOAT, false}});

    auto circleEbo = IndexBuffer::Create(circleIndices.data(), circleIndices.size());

    m_circles.vertexArray = VertexArray::Create();
    m_circles.vertexArray->AddVertexBuffer(circleVbo);
    m_circles.vertexArray->AddVertexBuffer(circleIbo);
    m_circles.vertexArray->SetIndexBuffer(circleEbo);
    m_circles.vertexArray->Bind();
    glVertexAttribDivisor(1, 1); 
    glVertexAttribDivisor(2, 1); 
    glVertexAttribDivisor(3, 1); 
    glVertexAttribDivisor(4, 1); 
    m_circles.vertexArray->Unbind();
    m_circles.drawType = GL_LINE_LOOP;

    // Line
    glm::vec3 line[] = {{0, 0, 0}, {0, 0, 1}};
    uint32_t lineIndices[] = {0, 1};

    auto lineVbo = VertexBuffer::Create(line, sizeof(line));
    lineVbo->SetLayout({{"Position",  3, GL_FLOAT, false}});

    auto lineIbo = VertexBuffer::Create();
    lineIbo->SetLayout({{"Transform[0]", 4, GL_FLOAT, false},
                        {"Transform[1]", 4, GL_FLOAT, false},
                        {"Transform[2]", 4, GL_FLOAT, false},
                        {"Transform[3]", 4, GL_FLOAT, false}});

    auto lineEbo = IndexBuffer::Create(lineIndices, 2);

    m_lines.vertexArray = VertexArray::Create();
    m_lines.vertexArray->AddVertexBuffer(lineVbo);
    m_lines.vertexArray->AddVertexBuffer(lineIbo);
    m_lines.vertexArray->SetIndexBuffer(lineEbo);
    m_circles.vertexArray->Bind();
    glVertexAttribDivisor(1, 1); 
    glVertexAttribDivisor(2, 1); 
    glVertexAttribDivisor(3, 1); 
    glVertexAttribDivisor(4, 1); 
    m_circles.vertexArray->Unbind();
    m_lines.drawType = GL_LINE_STRIP;

    // Points
    auto pointsVbo = VertexBuffer::Create();
    pointsVbo->SetLayout({{"Position",  3, GL_FLOAT, false}});
    m_points = VertexArray::Create();
    m_points->AddVertexBuffer(pointsVbo);

    // Shaders
    auto& resolver = Resolver::Get();
    m_pointsShader = Shader::Open(resolver.Resolve("resources/shaders/points.vert"),
                                  resolver.Resolve("resources/shaders/points.frag"));
    m_linesShader = Shader::Open(resolver.Resolve("resources/shaders/lines.vert"),
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


    m_pointsShader->Bind();
    m_pointsShader->SetMat4("uViewProjMatrix", viewProjMatrix);
    m_pointsShader->SetVec4("uColor", {1, 0, 0, 1});

    m_points->Bind();
    glDrawArrays(GL_POINTS, 0, m_pointCount);

    m_linesShader->Bind();
    m_linesShader->SetMat4("uViewProjMatrix", viewProjMatrix);
    m_linesShader->SetVec4("uColor", {1, 1, 0, 1});

    m_circles.Render();
    m_linesShader->SetVec4("uColor", {0, 0, 1, 0.1});
    m_spheres.Render();
    // m_planes.Render();

    // m_lines.Render();

}

void Renderer::BuildBatches(const LayerStackPtr& layerStack) 
{
    std::vector<glm::vec3> points;
    std::vector<glm::mat4> circles;
    std::vector<glm::mat4> spheres;
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
                    spheres.push_back(extractDualSphereMatrix(obj.dual()));
                    break;
                }

                case c3ga::MvecType::DualSphere: {
                    // Temporary rendering points instead of sphere for debug purpose
                    spheres.push_back(extractDualSphereMatrix(obj));
                    break;
                }

                case c3ga::MvecType::Circle:
                case c3ga::MvecType::ImaginaryCircle: {
                    circles.push_back(extractDualCircleMatrix(obj.dual()));
                    break;
                }
                case c3ga::MvecType::DualCircle:
                case c3ga::MvecType::ImaginaryDualCircle: {
                    circles.push_back(extractDualCircleMatrix(obj));
                    break;
                }

                case c3ga::MvecType::Line: {
                    break;
                }
            }
        }
    }

    m_pointCount = points.size();
    LOG_INFO("point count %d", m_pointCount);
    auto vbo = m_points->GetVertexBuffers()[0];
    vbo->Bind();
    vbo->SetData(points.data(), m_pointCount * sizeof(glm::vec3));

    m_circles.instanceCount = circles.size();
    LOG_INFO("circle count %d", m_circles.instanceCount);
    vbo = m_circles.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(circles.data(), m_circles.instanceCount * sizeof(glm::mat4));
    vbo->Unbind();

    m_spheres.instanceCount = spheres.size();
    LOG_INFO("sphere count %d", m_spheres.instanceCount);
    vbo = m_spheres.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(spheres.data(), m_spheres.instanceCount * sizeof(glm::mat4));
    vbo->Unbind();

    m_isValid = true;
}


void Renderer::Batch::Render() const
{
    if (!instanceCount)
        return;
        
    if (auto indexBuffer = vertexArray->GetIndexBuffer()) {
        vertexArray->Bind();
        glDrawElementsInstanced(drawType, 
                                indexBuffer->GetCount(), 
                                GL_UNSIGNED_INT, 
                                nullptr, 
                                instanceCount);
        vertexArray->Unbind();
    }

}
