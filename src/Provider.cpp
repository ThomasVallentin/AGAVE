#include "Provider.hpp"

#include "Simulation.hpp"
#include "Base/Logging.h"

#include "c3gaTools.hpp"

#include <random>

// == Explicit Provider ==

void Explicit::SetAnimated(const bool& animated) {
    auto& engine = SimulationEngine::Get();

    if (!animated && m_simHandle.IsValid())
    {
        engine.RemoveSimulation(m_simHandle);
        m_simHandle = SimulationHandle();
        return;
    }

    if (animated && !m_simHandle.IsValid())
    {
        m_simHandle = engine.NewSimulation();
    }
}

bool Explicit::Compute(Layer& layer)
{
    if (!IsAnimated()) 
    {
        return false;
    }

    MvecArray& objects = layer.GetObjects();
    MvecArray simulated = m_simHandle.GetObjects();

    if (simulated.empty()) 
    {
        // Un-dualing the objects before sending them to the simulation 
        // if the layer store them as dual. 
        if (layer.IsDual())
        {
            MvecArray duals(objects.size());
            for (size_t i=0 ; i < objects.size() ; ++i)
                duals[i] = objects[i].dual();

            m_simHandle.SetObjects(duals);
        }
        else 
        {
            m_simHandle.SetObjects(objects);
        }
    }
    else 
    {
        objects = simulated;
    }

    return true;
}


// == Random Generator ==

bool RandomGenerator::Compute(Layer& layer) 
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
                {
                    std::uniform_real_distribution<double> distrib(-m_extents, m_extents);
                    objects.push_back(c3ga::point<double>(distrib(c3ga::generator),
                                                          distrib(c3ga::generator), 
                                                          distrib(c3ga::generator)));
                }
                    
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
            case c3ga::MvecType::Plane:
            {
                for (size_t i=0 ; i < m_count ; ++i)
                    objects.push_back(c3ga::dualPlane<double>(c3ga::randomVector<double>() * m_extents).dual());

                break;
            }
            case c3ga::MvecType::DualPlane:
            {
                for (size_t i=0 ; i < m_count ; ++i)
                    objects.push_back(c3ga::dualPlane<double>(c3ga::randomVector<double>() * m_extents));

                break;
            }
            case c3ga::MvecType::PairPoint:
            {
                for (size_t i=0 ; i < m_count ; ++i)
                    objects.push_back(c3ga::randomPoint<double>() * m_extents ^ 
                                      c3ga::randomPoint<double>() * m_extents);

                break;
            }
            case c3ga::MvecType::DualPairPoint:
            {
                for (size_t i=0 ; i < m_count ; ++i)
                    objects.push_back((c3ga::randomPoint<double>() * m_extents ^ 
                                       c3ga::randomPoint<double>() * m_extents).dual());

                break;
            }
        }

        if (IsAnimated())
        {
            m_simHandle.SetObjects({});
        }

        m_isDirty = false;
    }

    return Explicit::Compute(layer);
}

// == Subset ==

bool Subset::Compute(Layer& layer)
{
    auto sources = layer.GetSources();
    if (!m_count || sources.empty())
    {
        layer.Clear();
        return false;
    }

    const auto source = sources[0].lock();
    const auto& sourceObjs = source->GetObjects();
    const bool sourceIsDual = layer.SourceIsDual(0);

    uint32_t count = m_count < 0 ? sourceObjs.size() : std::min((size_t)m_count, sourceObjs.size());

    auto& objects = layer.GetObjects();
    objects.resize(count);
    if (sourceIsDual)
        for (size_t i=0 ; i < count ; ++i)
            objects[i] = sourceObjs[i].dual();
    else
        for (size_t i=0 ; i < count ; ++i)
            objects[i] = sourceObjs[i];

    return true;
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

bool SelfCombination::Compute(Layer& layer) 
{
    auto sources = layer.GetSources();
    auto op = GetOperator();

    if (!m_count || !m_dimension || !op || sources.empty())
    {
        layer.Clear();
        return false;
    }

    const auto source = sources[0].lock();
    const auto& sourceObjs = source->GetObjects(); 
    const uint32_t sourceObjCount = sourceObjs.size();
    const bool sourceIsDual = layer.SourceIsDual(0);

    if (sourceObjCount < m_dimension)
    {
        layer.Clear();
        return false;
    }

    MvecArray& result = layer.GetObjects();
    uint32_t outObjCount = layer.GetObjects().size();
    
    if (m_indices.empty() || 
        m_prevCount != m_count || 
        m_prevDim != m_dimension || 
        m_prevSourceCount != sourceObjCount ||
        m_prevProductWithEi != GetProductWithEi())
    {
        // Generate random samples
        std::random_device device;
        std::mt19937 engine(device());

        auto combinations = GetIntegerCombinations(sourceObjs.size(), m_dimension);
        std::shuffle(combinations.begin(), combinations.end(), engine);

        outObjCount = m_count < 0 ? combinations.size() : std::min((size_t)m_count, combinations.size());
        m_indices.resize(outObjCount * m_dimension);
        uint32_t i = 0;
        for (uint n=0 ; n < outObjCount ; ++n)
        {
            for (const auto& index : combinations[n])
                m_indices[i++] = index;
        }
    }

    // Apply operator for each count for each "dimension"
    result.resize(outObjCount);
    uint idx = 0;
    if (sourceIsDual)
    {
        for (auto& obj : result)
        {
            obj = sourceObjs[m_indices[idx]].dual();
            for (uint i=0 ; i < m_dimension - 1 ; ++i) {
                ++idx;
                obj = op(obj, sourceObjs[m_indices[idx]].dual());
            }
            ++idx;

            if (GetProductWithEi())
                obj = op(obj, c3ga::ei<double>());
        }
    }
    else
    {
        for (auto& obj : result)
        {
            obj = sourceObjs[m_indices[idx]];
            for (uint i=0 ; i < m_dimension - 1 ; ++i) {
                ++idx;
                obj = op(obj, sourceObjs[m_indices[idx]]);
            }
            ++idx;

            if (GetProductWithEi())
                obj = op(obj, c3ga::ei<double>());
        }
    }

    m_prevCount = m_count;
    m_prevDim = m_dimension;
    m_prevSourceCount = sourceObjCount;
    m_prevProductWithEi = GetProductWithEi();

    return true;
}

// == Combination ==

bool Combination::Compute(Layer& layer) 
{
    auto sources = layer.GetSources();

    auto op = GetOperator();
    if (sources.size() < 2 || !op)
    {
        layer.Clear();
        return false;
    }
    
    LayerPtr sourcePtr1 = sources[0].lock();
    LayerPtr sourcePtr2 = sources[1].lock();

    const auto& sourceObjs1 = sourcePtr1->GetObjects(); 
    const auto& sourceObjs2 = sourcePtr2->GetObjects(); 

    const bool source1IsDual = layer.SourceIsDual(0);
    const bool source2IsDual = layer.SourceIsDual(1);

    auto& result = layer.GetObjects();
    result.resize(sourceObjs1.size() * sourceObjs2.size());

    uint i=0;
    for (const auto& s1 : sourceObjs1)
    {
        for (const auto& s2 : sourceObjs2)
        {
            result[i] = op(source1IsDual ? s1.dual() : s1, 
                           source2IsDual ? s2.dual() : s2);
            if (GetProductWithEi())
                result[i] = op(result[i], c3ga::ei<double>());
            ++i;
        }

    }

    return true;
}
