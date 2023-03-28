#ifndef LAYERSTACKWIDGET_H
#define LAYERSTACKWIDGET_H

#include "LayerStack.hpp"

#include "Renderer/Renderer.hpp"


class LayerStackWidget
{
public:
    LayerStackWidget();
    explicit LayerStackWidget(const LayerStackPtr& layerStack);
    ~LayerStackWidget() = default;

    void SetLayerStack(const LayerStackPtr& layerStack);

    void DeleteSelectedLayers();
    void Clear();

    inline bool IsHovered() const { return m_hovered; }
    bool Draw();

    LayerPtrArray &GetSelection();
    bool IsSelected(const LayerPtr &layer) const;
    void SelectLayer(const LayerPtr &layer, const int& index=-1);
    void DeselectLayer(const LayerPtr &layer);
    void ClearSelection();
    LayerPtr GetLastSelectedLayer() const;

private:
    bool DrawLayer(const LayerPtr& layer, const int& index);
    
    bool IsSource(const LayerPtr &layer);
    void UpdateSources();
    
    LayerStackPtr m_layerStack;
    LayerPtrArray m_selection;
    LayerPtrArray m_sources;

    int m_lastIndex;
    bool m_hovered;

    uint32_t m_renamedUUID; 
    DualMode m_dualMode;
};

#endif