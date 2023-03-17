#include "Renderer.hpp"
#include "Shapes.hpp"

#include "c3gaTools.hpp"
#include "C3GAUtils.hpp"

#include "Base/Resolver.h"
#include "Base/Logging.h"

#include <glm/glm.hpp>


Renderer::Renderer()
{
    // Spheres
    m_spheres.vertexArray = Shapes::InstanciableSphere();
    m_spheres.drawType = GL_TRIANGLE_STRIP;

    // Circles
    m_circles.vertexArray = Shapes::InstanciableCircle();
    m_circles.drawType = GL_LINE_LOOP;

    // Planes
    m_planes.vertexArray = Shapes::InstanciablePlane(20.0f);
    m_planes.drawType = GL_TRIANGLES;

    // Line
    m_lines.vertexArray = Shapes::InstanciableLine(20.0f);
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

void Renderer::Render(const LayerPtrArray& layers,
                      const glm::mat4& viewProjMatrix) 
{
    if (!m_isValid)
    {
        BuildBatches(layers);
    }

    glPointSize(m_renderSettings.pointSize);

    m_pointsShader->Bind();
    m_pointsShader->SetMat4("uViewProjMatrix", viewProjMatrix);
    m_pointsShader->SetVec4("uColor", {1, 0, 0, 1});

    m_points->Bind();
    glDrawArrays(GL_POINTS, 0, m_pointCount);

    m_linesShader->Bind();
    m_linesShader->SetMat4("uViewProjMatrix", viewProjMatrix);

    glEnable(GL_DEPTH_TEST);

    m_linesShader->SetVec4("uColor", {1, 1, 0, 1});
    m_circles.Render();

    m_linesShader->SetVec4("uColor", {1, 0, 1, 1});
    m_lines.Render();

    glDisable(GL_DEPTH_TEST);

    m_linesShader->SetVec4("uColor", {0, 0, 1, 0.1});
    m_spheres.Render();

    m_linesShader->SetVec4("uColor", {0, 1, 0, 0.1});
    m_planes.Render();
}

void Renderer::BuildBatches(const LayerPtrArray& layers) 
{
    std::vector<glm::vec3> points;
    std::vector<glm::mat4> spheres;
    std::vector<glm::mat4> circles;
    std::vector<glm::mat4> planes;
    std::vector<glm::mat4> lines;
    for (const auto& layer : layers)
    {
        if (!layer->IsVisible()) {
            continue;
        }

        for (const auto& obj : layer->GetObjects())
        {
            switch (c3ga::getTypeOf(obj))
            {
                // Points
                case c3ga::MvecType::Point: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        points.push_back({obj[c3ga::E1], obj[c3ga::E2], obj[c3ga::E3]});
                    break;
                }

                // Flat point
                case c3ga::MvecType::FlatPoint: {
                    c3ga::Mvec<double> flatPoint;
                    c3ga::extractFlatPoint<double>(obj, flatPoint);
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        points.push_back({flatPoint[c3ga::E1], flatPoint[c3ga::E2], flatPoint[c3ga::E3]});
                    break;
                }
                case c3ga::MvecType::DualFlatPoint: {
                    c3ga::Mvec<double> flatPoint;
                    c3ga::extractFlatPoint<double>(obj.dual(), flatPoint);
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        points.push_back({flatPoint[c3ga::E1], flatPoint[c3ga::E2], flatPoint[c3ga::E3]});                    break;
                }

                // Spheres
                case c3ga::MvecType::Sphere:
                case c3ga::MvecType::ImaginarySphere: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        spheres.push_back(c3ga::extractDualSphereMatrix(obj.dual()));
                    break;
                }

                case c3ga::MvecType::DualSphere:           
                case c3ga::MvecType::ImaginaryDualSphere: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        spheres.push_back(c3ga::extractDualSphereMatrix(obj));
                    break;
                }

                // Circles
                case c3ga::MvecType::Circle:             // == DualPairPoint   
                case c3ga::MvecType::ImaginaryCircle: {  // == DualImaginaryPairPoint
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        circles.push_back(c3ga::extractDualCircleMatrix(obj.dual()));
                    
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        for (const auto& pt : c3ga::extractPairPoint(obj.dual()))
                            points.push_back({pt[c3ga::E1], pt[c3ga::E2], pt[c3ga::E3]});

                    break;
                }
                case c3ga::MvecType::PairPoint:             // == DualCircle
                case c3ga::MvecType::ImaginaryPairPoint: {  // == DualImaginaryCircle
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        for (const auto& pt : c3ga::extractPairPoint(obj))
                            points.push_back({pt[c3ga::E1], pt[c3ga::E2], pt[c3ga::E3]});

                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        circles.push_back(c3ga::extractDualCircleMatrix(obj));

                    break;
                }

                // Planes
                case c3ga::MvecType::Plane: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        planes.push_back(c3ga::extractDualPlaneMatrix(obj.dual()));
                    break;
                }
                case c3ga::MvecType::DualPlane: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        planes.push_back(c3ga::extractDualPlaneMatrix(obj));
                    break;
                }

                // Lines
                case c3ga::MvecType::Line: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        lines.push_back(c3ga::extractDualLineMatrix(obj.dual()));
                    break;
                }
                case c3ga::MvecType::DualLine: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        lines.push_back(c3ga::extractDualLineMatrix(obj));
                    break;
                }
            }
        }
    }

    m_pointCount = points.size();
    // LOG_INFO("point count %d", m_pointCount);
    auto vbo = m_points->GetVertexBuffers()[0];
    vbo->Bind();
    vbo->SetData(points.data(), m_pointCount * sizeof(glm::vec3));

    m_spheres.instanceCount = spheres.size();
    // LOG_INFO("sphere count %d", m_spheres.instanceCount);
    vbo = m_spheres.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(spheres.data(), m_spheres.instanceCount * sizeof(glm::mat4));
    vbo->Unbind();

    m_circles.instanceCount = circles.size();
    // LOG_INFO("circle count %d", m_circles.instanceCount);
    vbo = m_circles.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(circles.data(), m_circles.instanceCount * sizeof(glm::mat4));
    vbo->Unbind();

    m_planes.instanceCount = planes.size();
    // LOG_INFO("sphere count %d", m_planes.instanceCount);
    vbo = m_planes.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(planes.data(), m_planes.instanceCount * sizeof(glm::mat4));
    vbo->Unbind();

    m_lines.instanceCount = lines.size();
    // LOG_INFO("sphere count %d", m_lines.instanceCount);
    vbo = m_lines.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(lines.data(), m_lines.instanceCount * sizeof(glm::mat4));
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
