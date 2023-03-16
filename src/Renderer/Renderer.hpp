#ifndef RENDERER_H
#define RENDERER_H

#include "Layer.hpp"

#include "Mesh.h"
#include "Shader.h"

#include <c3ga/Mvec.hpp>


enum DualMode
{
    DualMode_Default = 1 << 0,
    DualMode_Dual    = 1 << 1,
    DualMode_Both    = DualMode_Default | DualMode_Dual
};

struct RenderSettings
{
    float pointSize = 5.0f;
    DualMode dualMode = DualMode_Default;
};


class Renderer
{
public:
    Renderer();
    ~Renderer();

    inline RenderSettings& GetRenderSettings() { return m_renderSettings; }
    inline const RenderSettings& GetRenderSettings() const { return m_renderSettings; }

    void Invalidate();
    void Render(const LayerPtrArray& layers, 
                const glm::mat4& viewProjMatrix);

private:

    struct Batch
    {
        VertexArrayPtr vertexArray;
        uint32_t instanceCount = 0;
        GLenum drawType = GL_TRIANGLES;

        void Render() const;
    };

    void BuildBatches(const LayerPtrArray& layers);

    Batch m_spheres;
    Batch m_planes;
    Batch m_circles;
    Batch m_lines;
    VertexArrayPtr m_points;
    uint32_t m_pointCount = 0;

    ShaderPtr m_pointsShader;
    ShaderPtr m_linesShader;

    RenderSettings m_renderSettings;

    bool m_isValid = false;
};

#endif