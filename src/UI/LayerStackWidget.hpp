#ifndef LAYERSTACKWIDGET_H
#define LAYERSTACKWIDGET_H

#include "LayerStack.hpp"

#include "Renderer/Texture.h"


class LayerStackWidget
{
public:
    LayerStackWidget();
    explicit LayerStackWidget(const LayerStackPtr& layerStack);
    ~LayerStackWidget() = default;

    void SetLayerStack(const LayerStackPtr& layerStack);

    void DeleteSelectedLayers();
    void Clear();

    bool Draw();

    LayerPtrArray &GetSelection();
    bool IsSelected(const LayerPtr &layer) const;
    void SelectLayer(const LayerPtr &layer);
    void DeselectLayer(const LayerPtr &layer);
    void ClearSelection();

private:
    bool DrawLayer(const LayerPtr& layer, const int& index);
    bool DrawLayerTreeNode(const LayerPtr& layer, const int& index, bool& opened);
    bool DrawLayerContent(const LayerPtr& layer);
    
    bool IsSource(const LayerPtr &layer);
    void UpdateSources();
    
    LayerStackPtr m_layerStack;
    LayerPtrArray m_selection;
    LayerPtrArray m_sources;

    int m_lastIndex;

    TexturePtr m_dualIcon;
    static const char* s_dualIconName;
    TexturePtr m_notDualIcon;
    static const char* s_notDualIconName;
    TexturePtr m_visibleIcon;
    static const char* s_visibleIconName;
    TexturePtr m_notVisibleIcon;
    static const char* s_notVisibleIconName;
};

#endif