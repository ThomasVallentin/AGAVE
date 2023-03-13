#include "Layer.hpp"
#include "Provider.hpp"

#include "Base/Logging.h"

#include <c3gaTools.hpp>
#include <C3GAUtils.hpp>

#include <random>


// == Layer ==

Layer::Layer(const std::string& name, 
             const MvecArray& objects) :
        m_name(name), 
        m_visibility(true), 
        m_objects(objects), 
        m_provider(new Explicit())
{

}

Layer::Layer(const std::string& name, 
             const ProviderPtr& provider) :
        m_name(name), 
        m_visibility(true), 
        m_provider(provider) 
{

}

LayerWeakPtrArray Layer::GetSources() const
{
    return m_sources;
}

void Layer::SetSources(const LayerWeakPtrArray& layers)
{
    // Remove current layer from old source's destinations
    for (auto& source : m_sources) {
        if (auto sourcePtr = source.lock()) {
            sourcePtr->RemoveDestination(std::shared_ptr<Layer>(this));
        }
    }

    // Add current layer to new source's destinations
    for (auto& layer : layers) {
        if (auto layerPtr = layer.lock()) {
            layerPtr->AddDestination(std::shared_ptr<Layer>(this));
        }
    }

    m_sources = layers;
    SetDirty(true);
}

void Layer::AddSource(const LayerWeakPtr& layer)
{
    auto layerPtr = layer.lock();
    if (!layerPtr) {
        return;
    }

    auto it = std::find_if(m_sources.begin(), m_sources.end(),
        [&layerPtr](const auto& other)
        {
            return layerPtr == other.lock();
        }
    );

    if (it == m_sources.end())
    {
        m_sources.push_back(layer);
    }

    SetDirty(true);
}

void Layer::RemoveSource(const LayerWeakPtr& layer)
{
    auto layerPtr = layer.lock();
    if (!layerPtr)
    {
        return;
    }

    auto it = std::find_if(m_sources.begin(), m_sources.end(),
        [&layerPtr](const auto& other)
        {
            return layerPtr == other.lock();
        }
    );  
    if (it != m_sources.end())
    {
        m_sources.erase(it);
    }

    SetDirty(true);
}

LayerWeakPtrArray Layer::GetDestinations() const
{
    return m_destinations;
}

void Layer::SetDestinations(const LayerWeakPtrArray& layers)
{

    // Remove current layer from old source's destinations
    for (auto& source : m_sources) {
        if (auto sourcePtr = source.lock()) {
            sourcePtr->RemoveDestination(std::shared_ptr<Layer>(this));
        }
    }

    // Add current layer to new source's destinations
    for (auto& layer : layers) {
        if (auto layerPtr = layer.lock()) {
            layerPtr->AddDestination(std::shared_ptr<Layer>(this));
        }
    }

    m_sources = layers;
}

void Layer::AddDestination(const LayerWeakPtr& layer)
{
    auto layerPtr = layer.lock();
    if (!layerPtr) {
        return;
    }

    auto it = std::find_if(m_destinations.begin(), m_destinations.end(),
        [&layerPtr](const auto& other)
        {
            return layerPtr == other.lock();
        }
    );

    if (it == m_sources.end())
    {
        m_destinations.push_back(layer);
    }

}

void Layer::RemoveDestination(const LayerWeakPtr& layer)
{
    auto layerPtr = layer.lock();
    if (!layerPtr)
    {
        return;
    }

    auto it = std::find_if(m_destinations.begin(), m_destinations.end(),
        [&layerPtr](const auto& other)
        {
            return layerPtr == other.lock();
        }
    );  
    if (it != m_destinations.end())
    {
        m_destinations.erase(it);
    }
}

void Layer::SetDirty(const bool& dirty)
{
    bool propagate = dirty && !m_isDirty;
    m_isDirty = dirty;
    if (!propagate) 
    {
        return;
    }

    for (const auto& dest : m_destinations)
    {
        if (auto ptr = dest.lock())
        {
            ptr->SetDirty(true);
        }
    }
}

void Layer::SetProvider(const ProviderPtr& provider)
{
    if (m_provider != provider)
    {
        m_provider = provider;
        SetDirty(true);
    }
}

bool Layer::Update() 
{
    if (!m_isDirty)
    {
        return false;
    }

    if (!m_provider)
    {
        m_objects.clear();
        m_isDirty = false;
        return false;
    }

    for (auto& sourcePtr : m_sources) {
        auto source = sourcePtr.lock();
        if (source)
        {
            source->Update();  
        }
    }

    m_provider->Compute(*this);
    SetDirty(false);

    return true;
}
