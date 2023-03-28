#include "Renderer.hpp"
#include "Shapes.hpp"

#include "c3gaTools.hpp"
#include "C3GAUtils.hpp"

#include "Base/Resolver.h"
#include "Base/Logging.h"

#include <glm/glm.hpp>


struct InstancedData
{
    glm::mat4 transform;
    glm::vec4 color;

};

struct PointData
{
    glm::vec3 position;
    glm::vec4 color;
};


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
    pointsVbo->SetLayout({{"Position",  3, GL_FLOAT, false},
                          {"Color",     4, GL_FLOAT, false}});
    m_points = VertexArray::Create();
    m_points->AddVertexBuffer(pointsVbo);

    // Shaders
    auto& resolver = Resolver::Get();
    m_pointsShader = Shader::Open(resolver.Resolve("resources/shaders/points.vert"),
                                  resolver.Resolve("resources/shaders/solidColor.frag"));
    m_linesShader = Shader::Open(resolver.Resolve("resources/shaders/lines.vert"),
                                 resolver.Resolve("resources/shaders/solidColor.frag"));
    m_meshShader = Shader::Open(resolver.Resolve("resources/shaders/mesh.vert"),
                                resolver.Resolve("resources/shaders/lambert.frag"));
}

Renderer::~Renderer()
{

}

void Renderer::Invalidate()
{
    m_isValid = false;
}

void Renderer::Render(const LayerPtrArray& layers,
                      const glm::mat4& viewMatrix,
                      const glm::mat4& projMatrix) 
{
    if (!m_isValid)
    {
        BuildBatches(layers);
    }

    glPointSize(m_renderSettings.pointSize);

    m_pointsShader->Bind();
    m_pointsShader->SetMat4("uViewMatrix", viewMatrix);
    m_pointsShader->SetMat4("uProjMatrix", projMatrix);

    m_points->Bind();
    glDrawArrays(GL_POINTS, 0, m_pointCount);

    glDisable(GL_DEPTH_TEST);

    m_meshShader->Bind();
    m_meshShader->SetMat4("uViewMatrix", viewMatrix);
    m_meshShader->SetMat4("uProjMatrix", projMatrix);

    m_spheres.Render();
    m_planes.Render();

    glEnable(GL_DEPTH_TEST);

    m_linesShader->Bind();
    m_linesShader->SetMat4("uViewMatrix", viewMatrix);
    m_linesShader->SetMat4("uProjMatrix", projMatrix);

    m_circles.Render();
    m_lines.Render();
}

void Renderer::BuildBatches(const LayerPtrArray& layers) 
{
    std::vector<PointData> points;
    std::vector<InstancedData> spheres;
    std::vector<InstancedData> circles;
    std::vector<InstancedData> planes;
    std::vector<InstancedData> lines;
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
                        points.push_back({{obj[c3ga::E1], obj[c3ga::E2], obj[c3ga::E3]}, 
                                          {1.0f, 1.0f, 0.0f, 1.0f}});
                    break;
                }

                // Flat point
                case c3ga::MvecType::FlatPoint: {
                    c3ga::Mvec<double> flatPoint;
                    c3ga::extractFlatPoint<double>(obj, flatPoint);
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        points.push_back({{flatPoint[c3ga::E1], flatPoint[c3ga::E2], flatPoint[c3ga::E3]},
                                          {1.0, 1.0, 0.0, 1.0}});
                    break;
                }
                case c3ga::MvecType::DualFlatPoint: {
                    c3ga::Mvec<double> flatPoint;
                    c3ga::extractFlatPoint<double>(obj.dual(), flatPoint);
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        points.push_back({{flatPoint[c3ga::E1], flatPoint[c3ga::E2], flatPoint[c3ga::E3]},
                                          {1.0, 1.0, 0.0, 1.0}});
                    break;
                }

                // Spheres
                case c3ga::MvecType::Sphere:
                case c3ga::MvecType::ImaginarySphere: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        spheres.push_back({c3ga::extractDualSphereMatrix(obj.dual()),
                                           {0.0, 0.0, 1.0, 0.1}});
                    break;
                }

                case c3ga::MvecType::DualSphere:           
                case c3ga::MvecType::ImaginaryDualSphere: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        spheres.push_back({c3ga::extractDualSphereMatrix(obj),
                                           {0.0, 0.0, 1.0, 0.1}});
                    break;
                }

                // Circles
                case c3ga::MvecType::Circle:             // == DualPairPoint   
                case c3ga::MvecType::ImaginaryCircle: {  // == DualImaginaryPairPoint
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        circles.push_back({c3ga::extractDualCircleMatrix(obj.dual()),
                                           {1.0, 1.0, 0.0, 1.0}});

                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                    {
                        c3ga::Mvec<double> pt1, pt2;
                        c3ga::extractPairPoint(obj.dual(), pt1, pt2);
                        points.push_back({{pt1[c3ga::E1], pt1[c3ga::E2], pt1[c3ga::E3]},
                                          {1.0, 0.0, 0.0, 1.0}});
                        points.push_back({{pt2[c3ga::E1], pt2[c3ga::E2], pt2[c3ga::E3]},
                                          {1.0, 0.0, 0.0, 1.0}});
                    }
                    break;
                }
                case c3ga::MvecType::PairPoint:             // == DualCircle
                case c3ga::MvecType::ImaginaryPairPoint: {  // == DualImaginaryCircle
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                    {
                        c3ga::Mvec<double> pt1, pt2;
                        c3ga::extractPairPoint(obj, pt1, pt2);
                        points.push_back({{pt1[c3ga::E1], pt1[c3ga::E2], pt1[c3ga::E3]}, 
                                          {1.0, 0.0, 0.0, 1.0}});
                        points.push_back({{pt2[c3ga::E1], pt2[c3ga::E2], pt2[c3ga::E3]}, 
                                          {1.0, 0.0, 0.0, 1.0}});
                    }

                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        circles.push_back({c3ga::extractDualCircleMatrix(obj), 
                                           {1.0, 1.0, 0.0, 1.0}});

                    break;
                }

                // Planes
                case c3ga::MvecType::Plane: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        planes.push_back({c3ga::extractDualPlaneMatrix(obj.dual()), 
                                          {0.0, 1.0, 0.0, 0.1}});
                    break;
                }
                case c3ga::MvecType::DualPlane: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        planes.push_back({c3ga::extractDualPlaneMatrix(obj), 
                                          {0.0, 1.0, 0.0, 0.1}});
                    break;
                }

                // Lines
                case c3ga::MvecType::Line: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Default)
                        lines.push_back({c3ga::extractDualLineMatrix(obj.dual()), 
                                         {1.0, 0.0, 1.0, 1.0}});
                    break;
                }
                case c3ga::MvecType::DualLine: {
                    if ((int)m_renderSettings.dualMode & (int)DualMode_Dual)
                        lines.push_back({c3ga::extractDualLineMatrix(obj), 
                                         {1.0, 0.0, 1.0, 1.0}});
                    break;
                }
            }
        }
    }

    m_pointCount = points.size();
    LOG_DEBUG("Renderer: Point count %d", m_pointCount);
    auto vbo = m_points->GetVertexBuffers()[0];
    vbo->Bind();
    vbo->SetData(points.data(), m_pointCount * sizeof(PointData));
    vbo->Unbind();

    m_spheres.instanceCount = spheres.size();
    LOG_DEBUG("Renderer: Sphere count %d", m_spheres.instanceCount);
    vbo = m_spheres.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(spheres.data(), m_spheres.instanceCount * sizeof(InstancedData));
    vbo->Unbind();

    m_circles.instanceCount = circles.size();
    LOG_DEBUG("Renderer: Circle count %d", m_circles.instanceCount);
    vbo = m_circles.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(circles.data(), m_circles.instanceCount * sizeof(InstancedData));
    vbo->Unbind();

    m_planes.instanceCount = planes.size();
    LOG_DEBUG("Renderer: Sphere count %d", m_planes.instanceCount);
    vbo = m_planes.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(planes.data(), m_planes.instanceCount * sizeof(InstancedData));
    vbo->Unbind();

    m_lines.instanceCount = lines.size();
    LOG_DEBUG("Renderer: Lines count %d", m_lines.instanceCount);
    vbo = m_lines.vertexArray->GetVertexBuffers()[1];
    vbo->Bind();
    vbo->SetData(lines.data(), m_lines.instanceCount * sizeof(InstancedData));
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
