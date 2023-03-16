#include "LayerStack.hpp"

#include "Base/Logging.h"


LayerPtr LayerStack::GetLayer(const uint32_t& index) const
{
    return m_layers[index];
}

const LayerPtrArray& LayerStack::GetLayers() const
{
    return m_layers;
}

void LayerStack::AddLayer(const LayerPtr& layer)
{
    auto it = std::find_if(m_layers.begin(), m_layers.end(),
        [&layer](const auto& other)
        {
            return layer == other;
        }
    );

    if (it == m_layers.end())
    {
        m_layers.push_back(layer);
    }
}

void LayerStack::RemoveLayer(const LayerPtr& layer)
{
    auto it = std::find_if(m_layers.begin(), m_layers.end(),
        [&layer](const auto& other)
        {
            return layer == other;
        }
    );

    // Disconnect sources
    for (const auto& src : layer->GetSources())
    {
        auto source = src.lock();
        if (source)
        {
            DisconnectLayers(source, layer);
        }
    }

    // Disconnect destinations
    for (const auto& dst : layer->GetDestinations())
    {
        auto destination = dst.lock();
        if (destination)
        {
            DisconnectLayers(layer, destination);
        }
    }

    if (it != m_layers.end())
    {
        m_layers.erase(it);
    }
}

void LayerStack::Clear()
{
    m_layers.clear();
}

LayerPtr LayerStack::NewLayer(const std::string& name,
                              const MvecArray& objects)
{
    LayerPtr layer = std::make_shared<Layer>(GetNextAvailableName(name), objects);
    m_layers.push_back(layer);

    return layer;
}

LayerPtr LayerStack::NewRandomGenerator(const std::string& name,
                                        const c3ga::MvecType& objType,
                                        const uint32_t& count,
                                        const float& extents)
{
    ProviderPtr provider = std::make_shared<RandomGenerator>(objType, count, extents);
    LayerPtr layer = std::make_shared<Layer>(GetNextAvailableName(name), provider);
    m_layers.push_back(layer);

    return layer;
}

LayerPtr LayerStack::NewSubset(const std::string& name,
                               const LayerPtr& source, 
                               const uint32_t& count)
{
    ProviderPtr provider = std::make_shared<Subset>(count);
    LayerPtr layer = std::make_shared<Layer>(GetNextAvailableName(name), provider);
    layer->AddSource(source);
    source->AddDestination(layer);
    m_layers.push_back(layer);

    return layer;
}

LayerPtr LayerStack::NewSelfCombination(const std::string& name,
                               const LayerPtr& source, 
                               const uint32_t& dimension, 
                               const int& count,
                               const Operator& op)
{
    ProviderPtr provider = std::make_shared<SelfCombination>(dimension, count, op);
    LayerPtr layer = std::make_shared<Layer>(GetNextAvailableName(name), provider);
    layer->AddSource(source);
    source->AddDestination(layer);
    m_layers.push_back(layer);

    return layer;
}

LayerPtr LayerStack::NewCombination(const std::string& name,
                                    const LayerPtr& source1,
                                    const LayerPtr& source2,
                                    const Operator& op)
{
    ProviderPtr combination = std::make_shared<Combination>(op);
    LayerPtr layer = std::make_shared<Layer>(GetNextAvailableName(name), combination);
    layer->AddSource(source1);
    layer->AddSource(source2);
    source1->AddDestination(layer);
    source2->AddDestination(layer);
    m_layers.push_back(layer);

    return layer;
}

void LayerStack::ConnectLayers(const LayerPtr& source, const LayerPtr& destination) const
{
    destination->AddSource(source);
    if (source)
        source->AddDestination(destination);
}

void LayerStack::DisconnectLayers(const LayerPtr& source, const LayerPtr& destination) const
{
    destination->RemoveSource(source);
    if (source)
        source->RemoveDestination(destination);
}

std::string LayerStack::GetNextAvailableName(std::string basename) const
{
    auto alreadyExists = [&](const std::string& n)->bool {
        for (const auto& layer : m_layers)
            if (layer->GetName() == n)
                return true;

        return false;
    };

    int i = 1;
    std::string name = basename;
    while (alreadyExists(name)) 
        name = basename + std::to_string(i++);

    return name;
}
