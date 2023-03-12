#include "Provider.hpp"

#include "Simulation.hpp"

#include "c3gaTools.hpp"

#include <random>

// == Explicit Provider ==

void Explicit::SetAnimated(const bool& animated) {
    m_animated = animated;
}

// == Random Generator ==

void RandomGenerator::Compute(Layer& layer) 
{
    if (m_isDirty)
    {
        auto& objects = layer.GetObjects();
        objects.clear();
        switch (m_objType)
        {
            case c3ga::MvecType::Point:
            {
                for (size_t i=0 ; i < m_count ; ++i)
                    objects.push_back(c3ga::randomPoint<double>() * m_extents);

                break;
            }
            case c3ga::MvecType::Sphere:
            {
                for (size_t i=0 ; i < m_count ; ++i)
                {
		            std::uniform_real_distribution<double> distrib(-m_extents, m_extents);
                    objects.push_back(c3ga::dualSphere<double>(distrib(c3ga::generator),
                                                               distrib(c3ga::generator), 
                                                               distrib(c3ga::generator), 
                                                               1.0).dual());
                }

                break;
            }
            case c3ga::MvecType::DualSphere:
            {
                for (size_t i=0 ; i < m_count ; ++i)
                {
		            std::uniform_real_distribution<double> distrib(-m_extents, m_extents);
                    objects.push_back(c3ga::dualSphere<double>(distrib(c3ga::generator),
                                                               distrib(c3ga::generator), 
                                                               distrib(c3ga::generator), 
                                                               1.0));
                }

                break;
            }
        }
    }
}

// == Subset ==

void Subset::Compute(Layer& layer)
{
    auto sources = layer.GetSources();
    if (!m_count || sources.empty())
    {
        layer.Clear();
        return;
    }

    const auto source = sources[0].lock();
    const auto& sourceObjs = source->GetObjects();

    uint32_t count = m_count < 0 ? sourceObjs.size() : std::min((size_t)m_count, sourceObjs.size());

    auto& objects = layer.GetObjects();
    objects = MvecArray(sourceObjs.begin(), sourceObjs.begin() + count);
}

// == Operator Based Provider ==

void OperatorBasedProvider::SetOperator(const Operator& op)
{
    if (m_op != op)
        m_op = op;
}

// == Self Combination ==

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

void SelfCombination::Compute(Layer& layer) 
{
    auto sources = layer.GetSources();
    auto op = GetOperator();
    if (!m_count || !m_dimension || !op || sources.empty())
    {
        layer.Clear();
        return;
    }

    const auto source = sources[0].lock();
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

        outObjCount = m_count < 0 ? combinations.size() : std::min((size_t)m_count, combinations.size());
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
        for (uint i=0 ; i < m_dimension - 1 ; ++i) {
            ++idx;
            obj = op(obj, sourceObjs[m_indices[idx]]);
        }
    }

    m_prevCount = m_count;
    m_prevDim = m_dimension;
    m_prevSourceCount = sources.size();
}

// == Combination ==

void Combination::Compute(Layer& layer) 
{
    auto sources = layer.GetSources();

    auto op = GetOperator();
    if (sources.size() < 2 || !op)
    {
        layer.Clear();
        return;
    }
    
    LayerPtr sourcePtr1 = sources[0].lock();
    LayerPtr sourcePtr2 = sources[1].lock();

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
