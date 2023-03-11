#include "Layer.hpp"

#include "Base/Logging.h"

#include <c3gaTools.hpp>

#include <random>


// == Layer ==

Layer::Layer(const std::string& name, 
             const MvecArray& objects) :
        m_name(name), 
        m_objects(objects), 
        m_mapping(new NoMapping())
{

}

Layer::Layer(const std::string& name, 
             const MappingPtr& mapping, 
             const Operator& op) :
        m_name(name), 
        m_mapping(mapping), 
        m_op(op) 
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

void Layer::Update() 
{
    LOG_INFO("Start update ! %s", m_name.c_str());
    if (!m_isDirty)
    {
        LOG_INFO("Already updated ! %s", m_name.c_str());
        return;
    }

    if (!m_mapping)
    {
        m_objects.clear();
        m_isDirty = false;
        return;
    }


    for (auto& sourcePtr : m_sources) {
        auto source = sourcePtr.lock();
        if (source)
        {
            source->Update();  
        }
    }

    LOG_INFO("Update ! %s", m_name.c_str());
    m_mapping->Compute(m_op, *this);
    SetDirty(false);
}

// == Subset ==

void Subset::Compute(const Operator& op, Layer& layer) 
{
    auto sources = layer.GetSources();
    LayerPtr sourcePtr = nullptr;
    for (uint i = 0 ; i < sources.size() & !sourcePtr ; ++i) {
        sourcePtr = sources[i].lock();
    }

    if (!m_count || !m_dimension || !sourcePtr)
    {
        layer.Clear();
        return;
    }

    const auto& sourceObjs = sourcePtr->Get(); 
    uint32_t count = m_count > 0 ? std::min(sourceObjs.size(), (size_t)m_count) : sourceObjs.size();
    uint32_t indicesCount = count * m_dimension;

    if (m_indices.empty() || indicesCount != m_indices.size())
    {
        // Generate random samples
        std::random_device device;
        std::mt19937 engine(device());
        std::uniform_int_distribution<uint32_t> distrib(0, sourceObjs.size() - 1);

        m_indices.reserve(indicesCount);
        for (uint i=0 ; i < indicesCount ; ++i)
        {
            m_indices.push_back(distrib(engine));
        }
    }

    MvecArray& result = layer.Get();
    result.resize(count);

    // Identity case (no operator)
    if (!op)
    {
        result.resize(count);
        for (uint32_t i=0 ; i < count ; ++i)
        {
            result[i] = sourceObjs[i];
        }

        return;
    }

    // Apply operator for each count for each "dimension"
    uint idx = 0;
    for (auto& obj : result)
    {
        obj = sourceObjs[m_indices[idx]];
        std::cout << m_indices[idx] << " ";
        for (uint i=0 ; i < m_dimension - 1 ; ++i)
        {
            ++idx;
            obj = op(obj, sourceObjs[m_indices[idx]]);
            std::cout << m_indices[idx] << " ";
        }
        std::cout << c3ga::whoAmI(obj) << std::endl;
        
    }
}

// == Combination ==

void Combination::Compute(const Operator& op, Layer& layer) 
{
    auto sources = layer.GetSources();
    LayerPtr sourcePtr1;
    LayerPtr sourcePtr2;
    for (uint i = 0 ; i < sources.size() && !(sourcePtr1 && sourcePtr2) ; ++i) {
        std::cout << i << " " << sources.size() << std::endl;
        if (!sourcePtr1) {
            sourcePtr1 = sources[i].lock();
        } else {
            sourcePtr2 = sources[i].lock();
        }
    }

    if (!sourcePtr1 || !sourcePtr2 || !op)
    {
        LOG_INFO("clear %d %d", sourcePtr1.get(), sourcePtr2.get());
        layer.Clear();
        return;
    }

    const auto& sourceObjs1 = sourcePtr1->Get(); 
    const auto& sourceObjs2 = sourcePtr2->Get(); 

    auto& result = layer.Get();
    result.resize(sourceObjs1.size() * sourceObjs2.size());

    uint i=0;
    for (const auto& s1 : sourceObjs1)
    {
        for (const auto& s2 : sourceObjs2)
        {
            result[i] = op(s1.dual(), s2.dual()).dual();
            ++i;
        }
    }
}

