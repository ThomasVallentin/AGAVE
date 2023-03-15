#include "Layer.hpp"
#include "Provider.hpp"

#include "Base/Logging.h"

#include <c3gaTools.hpp>
#include <C3GAUtils.hpp>

#include <random>


// == UUID ==

static std::random_device layerUUIDGenerator;
static std::uniform_int_distribution<uint32_t> nextLayerUUIDDistrib(1, 1024);

static uint32_t lastLayerUUID;

uint32_t GetNextUUID()
{
    lastLayerUUID += nextLayerUUIDDistrib(layerUUIDGenerator);
    return lastLayerUUID;
}

// == Layer ==

Layer::Layer(const std::string& name, 
             const MvecArray& objects) :
        m_name(name), 
        m_uuid(GetNextUUID()),
        m_objects(objects), 
        m_isDual(false), 
        m_visibility(true), 
        m_provider(new Explicit())
{

}

Layer::Layer(const std::string& name, 
             const ProviderPtr& provider) :
        m_name(name), 
        m_uuid(GetNextUUID()),
        m_isDual(false), 
        m_visibility(true), 
        m_provider(provider) 
{

}

LayerWeakPtrArray Layer::GetSources() const
{
    return m_sources;
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
        m_dualSources.push_back(false);
    }

    SetDirty(DirtyBits_Provider);
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
        m_dualSources.erase(m_dualSources.begin() + (it - m_sources.begin()));
    }

    SetDirty(DirtyBits_Provider);
}

bool Layer::SourceIsDual(const uint32_t& index) const
{
    if (index < m_dualSources.size())
        return m_dualSources[index];

    return false;
}

void Layer::SetSourceDual(const uint32_t& index, const bool& dual)
{
    if (index < m_dualSources.size())
    {
        if (m_dualSources[index] != dual)
        {
            m_dualSources[index] = dual;
            SetDirty(DirtyBits_Provider);
        }
    }
}

LayerWeakPtrArray Layer::GetDestinations() const
{
    return m_destinations;
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

    if (it == m_destinations.end())
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
            return layerPtr.get() == other.lock().get();
        }
    );  
    if (it != m_destinations.end())
    {
        m_destinations.erase(it);
    }
}

void Layer::SetDirty(const DirtyBits& dirtyBits)
{
    bool propagate = (dirtyBits != DirtyBits_None) && !IsDirty();
    m_dirtyBits = (DirtyBits)(m_dirtyBits | dirtyBits);
    if (!propagate) 
    {
        return;
    }

    for (const auto& dest : m_destinations)
    {
        if (auto ptr = dest.lock())
        {
            ptr->SetDirty(DirtyBits_Provider);
        }
    }
}

void Layer::SetProvider(const ProviderPtr& provider)
{
    if (m_provider != provider)
    {
        m_provider = provider;
        SetDirty(DirtyBits_Provider);
    }
}

void Layer::SetDual(const bool& dual)
{
    if (m_isDual != dual)
    {
        m_isDual = dual;
        SetDirty(DirtyBits_Dual);
    }
}

bool Layer::Update() 
{
    if (m_dirtyBits == DirtyBits_None)
    {
        return false;
    }

    if (!m_provider)
    {
        m_objects.clear();
        m_dirtyBits = DirtyBits_None;
        return false;
    }

    for (auto& sourcePtr : m_sources) {
        auto source = sourcePtr.lock();
        if (source)
        {
            source->Update();  
        }
    }

    bool objectsChanged = m_provider->Compute(*this);
    if ((objectsChanged && m_isDual) || m_dirtyBits & DirtyBits_Dual)
        for (auto& obj : m_objects)
            obj = obj.dual();

    m_dirtyBits = DirtyBits_None;

    return true;
}
