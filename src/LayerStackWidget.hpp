#ifndef LAYERSTACKWIDGET_H
#define LAYERSTACKWIDGET_H

#include "LayerStack.hpp"


class LayerStackWidget
{
public:
    LayerStackWidget() = default;
    explicit LayerStackWidget(const LayerStackPtr& layerStack) : m_layerStack(layerStack) {}
    ~LayerStackWidget() = default;

    void SetLayerStack(const LayerStackPtr& layerStack);

    void DeleteSelectedLayers();
    void Clear();

    void Draw();

    LayerPtrArray &GetSelection();
    bool IsSelected(const LayerPtr &layer) const;
    void SelectLayer(const LayerPtr &layer);
    void DeselectLayer(const LayerPtr &layer);
    void ClearSelection();

private:
    void DrawLayer(const LayerPtr& layer, const int& index);
    void DrawMapping(const LayerPtr& layer, 
                     const bool& isSelected, 
                     const bool& isSource);
    
    bool IsSource(const LayerPtr &layer);
    void UpdateSources();
    
    LayerStackPtr m_layerStack;
    LayerPtrArray m_selection;
    LayerPtrArray m_sources;

    int m_lastIndex;
};

#endif