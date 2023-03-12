#include "Layer.hpp"

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
        m_mapping(new Copy())
{

}

Layer::Layer(const std::string& name, 
             const MappingPtr& mapping, 
             const Operator& op) :
        m_name(name), 
        m_visibility(true), 
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

void Layer::SetOperator(const Operator& op)
{
    if (m_op != op)
    {
        m_op = op;
        SetDirty(true);
    }
}

void Layer::SetMapping(const MappingPtr& mapping)
{
    if (m_mapping != mapping)
    {
        m_mapping = mapping;
        SetDirty(true);
    }
}

bool Layer::Update() 
{
    if (!m_isDirty)
    {
        return false;
    }

    if (!m_mapping)
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

    m_mapping->Compute(m_op, *this);
    SetDirty(false);

    return true;
}

// == Subset ==


// Get all order independent combinations of integers.
// This code is adapted from https://rosettacode.org/wiki/Combinations
std::vector<std::vector<uint32_t>> GetIntegerCombinations(const uint32_t& maxIndex, const uint32_t& combinationSize)
{
    std::string bitmask(combinationSize, 1);
    bitmask.resize(maxIndex, 0);

    std::vector<std::vector<uint32_t>> result;
    do {
        std::vector<uint32_t> combination(combinationSize);
        uint32_t currIdx = 0;
        for (uint32_t i = 0; i < maxIndex; ++i)
        {
            if (bitmask[i])
                combination[currIdx++] = i;
        }
        result.push_back(combination);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
    
    return result;
}

void Subset::Compute(const Operator& op, Layer& layer) 
{

    auto sources = layer.GetSources();
    if (!m_count || !m_dimension || !op || sources.empty())
    {
        layer.Clear();
        return;
    }

    const auto& source = sources[0].lock();
    const auto& sourceObjs = source->GetObjects(); 
    uint32_t sourceObjCount = sourceObjs.size();

    MvecArray& result = layer.GetObjects();
    uint32_t outObjCount;

    if (m_indices.empty() || 
        m_prevCount != m_count || 
        m_prevDim != m_dimension || 
        m_prevSourceCount != sourceObjCount)
    {
        // Generate random samples
        std::random_device device;
        std::mt19937 engine(device());

        auto combinations = GetIntegerCombinations(sourceObjs.size(), m_dimension);
        std::shuffle(combinations.begin(), combinations.end(), engine);

        outObjCount = m_count > 0 ? std::min((size_t)m_count, combinations.size()) : combinations.size();
        m_indices.reserve(outObjCount * m_dimension);
        for (uint i=0 ; i < outObjCount ; ++i)
        {
            for (const auto& index : combinations[i])
                m_indices.push_back(index);
        }
    }

    // Apply operator for each count for each "dimension"
    result.resize(outObjCount);
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

    m_prevCount = m_count;
    m_prevDim = m_dimension;
    m_prevSourceCount = sources.size();
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
        layer.Clear();
        return;
    }

    const auto& sourceObjs1 = sourcePtr1->GetObjects(); 
    const auto& sourceObjs2 = sourcePtr2->GetObjects(); 

    auto& result = layer.GetObjects();
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

