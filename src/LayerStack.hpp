#ifndef LAYERSTACK_H
#define LAYERSTACK_H

#include "Layer.hpp"

class LayerStack
{
public:
    LayerStack();
    ~LayerStack();

    LayerPtrArray GetLayers() const;
    void AddLayer(const LayerPtr& layer);
    void RemoveLayer(const LayerPtr& layer);
    void Clear();

    LayerPtr NewLayer(const std::string& name,
                      const MvecArray& objects);
    LayerPtr NewSubset(const std::string& name,
                       const LayerPtr& source, 
                       const uint32_t& dimension, 
                       const int& count=-1,
                       const Operator& op=Layer::OuterOp);
    LayerPtr NewCombination(const std::string& name,
                            const LayerPtr& source1,
                            const LayerPtr& source2,
                            const Operator& op=Layer::OuterOp);

    void ConnectLayers(const LayerPtr& source, const LayerPtr& destination) const;
    void DisconnectLayers(const LayerPtr& source, const LayerPtr& destination) const;

private:
    LayerPtrArray m_layers;
};

#endif