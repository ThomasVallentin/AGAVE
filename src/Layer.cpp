#include "Layer.hpp"

#include <random>


// == Layer ==

void Layer::Invalidate()
{
    for (const auto& dep : m_dependencies)
    {
        if (auto ptr = dep.lock())
        {
            ptr->Invalidate();
        }
    }
}

void Layer::AddDependency(const LinkWeakPtr& link) 
{
    auto ptr = link.lock();
    if (!ptr)
    {
        return;
    }

    auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(),
        [&ptr](const auto& other)
        {
            return ptr == other.lock();
        }
    );  

    if (it == m_dependencies.end())
    {
        m_dependencies.push_back(link);
    }
}

void Layer::RemoveDependency(const LinkWeakPtr& link)
{
    auto ptr = link.lock();
    if (!ptr)
    {
        return;
    }

    auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(),
        [&ptr](const auto& other)
        {
            return ptr == other.lock();
        }
    );  
    if (it != m_dependencies.end())
    {
        m_dependencies.erase(it);
    }
}

// == Link ==

void Link::Invalidate()
{
    for (const auto& dep : m_dependencies)
    {
        if (auto ptr = dep.lock())
        {
            ptr->Invalidate();
        }
    }
}

void Link::AddDependency(const LayerWeakPtr& layer)
{
    auto ptr = layer.lock();
    if (!ptr)
    {
        return;
    }

    auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(),
        [&ptr](const auto& other)
        {
            return ptr == other.lock();
        }
    );  

    if (it == m_dependencies.end())
    {
        m_dependencies.push_back(layer);
    }
}

void Link::RemoveDependency(const LayerWeakPtr& layer)
{
    auto ptr = layer.lock();
    if (!ptr)
    {
        return;
    }

    auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(),
        [&ptr](const auto& other)
        {
            return ptr == other.lock();
        }
    );  
    
    if (it != m_dependencies.end())
    {
        m_dependencies.erase(it);
    }
}

// == Subset ==

void Subset::Invalidate() 
{
    m_indices.clear();
    Link::Invalidate();
}

void Subset::Update(const LayerPtr& destination) 
{
    if (!m_count || !m_dimension || !m_source)
    {
        destination->Clear();
        return;
    }

    const auto& source = m_source->Get(); 
    uint32_t count = m_count > 0 ? std::min(source.size(), (size_t)m_count) : source.size();
    uint32_t indicesCount = count * m_dimension;

    if (m_indices.empty())
    {
        // Generate random samples
        std::random_device device;
        std::mt19937 engine(device());
        std::uniform_int_distribution<uint32_t> distrib(0, source.size() - 1);

        m_indices.reserve(indicesCount);
        for (uint i=0 ; i < indicesCount ; ++i)
        {
            m_indices.push_back(distrib(engine));
        }
    }

    MvecArray& dest = destination->Get();
    dest.resize(count);

    // Identity case (no operator)
    if (!m_op)
    {
        dest.clear();
        for (uint32_t i=0 ; i < count ; ++i)
        {
            dest.push_back(source[i]);
        }
        return;
    }

    // Apply operator for each count for each "dimension"
    uint idx = 0;
    for (auto& obj : dest)
    {
        obj = source[m_indices[idx]];

        for (uint i=0 ; i < m_dimension - 1 ; ++i)
        {
            ++idx;
            obj = m_op(obj, source[m_indices[idx]]);
        }
    }
}

// == Combination ==

void Combination::Update(const LayerPtr& destination) 
{
    if (!m_source1 || !m_source2)
    {
        destination->Clear();
        return;
    }

    MvecArray result;
    const auto& source1 = m_source1->Get(); 
    const auto& source2 = m_source2->Get(); 

    result.reserve(source1.size() * source2.size());

    uint i=0;
    for (const auto& s1 : source1)
    {
        for (const auto& s2 : source2)
        {
            result[i] = m_op(s1, s2);
            ++i;
        }
    }
}


void Combination::SetSources(const LayerPtr& source1,
                             const LayerPtr& source2)
{ 
    m_source1 = source1; 
    m_source2 = source2; 
}

void Combination::SetSources(const LayerPair& sources)
{ 
    m_source1 = sources.first; 
    m_source2 = sources.second; 
}