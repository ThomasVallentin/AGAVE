#ifndef LAYERSTACK_H
#define LAYERSTACK_H

#include "Layer.hpp"
#include "Provider.hpp"


class LayerStack;

using LayerStackPtr = std::shared_ptr<LayerStack>;


class LayerStack
{
public:
    LayerStack() = default;
    ~LayerStack() = default;

    LayerPtrArray GetLayers() const;
    void AddLayer(const LayerPtr& layer);
    void RemoveLayer(const LayerPtr& layer);
    void Clear();

    LayerPtr NewLayer(const std::string& name,
                      const MvecArray& objects);
    LayerPtr NewRandomGenerator(const std::string& name,
                                const c3ga::MvecType& objType=c3ga::MvecType::Point,
                                const uint32_t& count=4,
                                const float& extents=1.0f);
    LayerPtr NewSubset(const std::string& name,
                       const LayerPtr& source, 
                       const uint32_t& count=-1);
    LayerPtr NewSelfCombination(const std::string& name,
                       const LayerPtr& source, 
                       const uint32_t& dimension=2, 
                       const int& count=-1,
                       const Operator& op=Operators::OuterProduct);
    LayerPtr NewCombination(const std::string& name,
                            const LayerPtr& source1,
                            const LayerPtr& source2,
                            const Operator& op=Operators::OuterProduct);

    void ConnectLayers(const LayerPtr& source, const LayerPtr& destination) const;
    void DisconnectLayers(const LayerPtr& source, const LayerPtr& destination) const;

private:
    std::string GetNextAvailableName(std::string basename="Layer") const;
    
    LayerPtrArray m_layers;
};

#endif