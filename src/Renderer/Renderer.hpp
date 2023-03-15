#ifndef RENDERER_H
#define RENDERER_H

#include "LayerStack.hpp"

#include "Mesh.h"
#include "Shader.h"

#include <c3ga/Mvec.hpp>



struct RenderSettings
{
    enum class DisplayMode
    {
        Default = 1 << 0,
        Dual    = 1 << 1,
        Both    = Default | Dual
    };

    float pointSize = 5.0f;
    DisplayMode displayMode = DisplayMode::Default;
};


class Renderer
{
public:
    Renderer();
    ~Renderer();

    inline RenderSettings& GetRenderSettings() { return m_renderSettings; }
    inline const RenderSettings& GetRenderSettings() const { return m_renderSettings; }

    void Invalidate();
    void Render(const LayerStackPtr& layerStack, 
                const glm::mat4& viewProjMatrix);

private:

    struct Batch
    {
        VertexArrayPtr vertexArray;
        uint32_t instanceCount = 0;
        GLenum drawType = GL_TRIANGLES;

        void Render() const;
    };

    void BuildBatches(const LayerStackPtr& layerStack);

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