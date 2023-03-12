#include "LayerStack.hpp"

LayerPtrArray LayerStack::GetLayers() const
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
    LayerPtr layer = std::make_shared<Layer>(name, objects);
    m_layers.push_back(layer);

    return layer;
}

LayerPtr LayerStack::NewSubset(const std::string& name,
                               const LayerPtr& source, 
                               const uint32_t& dimension, 
                               const int& count,
                               const Operator& op)
{
    ProviderPtr subset = std::make_shared<Subset>(dimension, count);
    LayerPtr layer = std::make_shared<Layer>(name, subset, op);
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
    ProviderPtr combination = std::make_shared<Combination>();
    LayerPtr layer = std::make_shared<Layer>(name, combination, op);
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
    source->AddDestination(destination);
    destination->SetDirty(true);
}

void LayerStack::DisconnectLayers(const LayerPtr& source, const LayerPtr& destination) const
{
    destination->RemoveSource(source);
    source->RemoveDestination(destination);
    destination->SetDirty(true);
}
