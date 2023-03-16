#ifndef CONTENTEDITOR_H
#define CONTENTEDITOR_H


#include "Renderer/Renderer.hpp"
#include "LayerStack.hpp"


class ContentEditor
{
public:
    ContentEditor(const LayerStackPtr& layerStack);
    ~ContentEditor();

    inline LayerPtr GetCurrentLayer() const { return m_layer; }
    inline void SetCurrentLayer(const LayerPtr& layer) { m_layer = layer; }

    inline bool IsLocked() const { return m_locked; }
    inline void SetLocked(const bool& locked) { m_locked = locked; }
    inline bool ShouldClose() const { return !m_opened; }

    inline DualMode GetDualMode() const { return m_dualMode; }
    inline void SetDualMode(const DualMode& dualMode) { m_dualMode = dualMode; }
    
    inline bool IsHovered() const { return m_hovered; }
    bool Draw();

private:
    uint32_t m_id;
    bool m_opened = true;
    bool m_locked;
    bool m_hovered;

    LayerPtr m_layer;
    LayerStackPtr m_layerStack;
    DualMode m_dualMode;
};

#endif