#ifndef PROVIDER_HPP
#define PROVIDER_HPP

#include "Layer.hpp"
#include "Simulation.hpp"

#include "C3GAUtils.hpp"


using Operator = c3ga::Mvec<double>(*)(const c3ga::Mvec<double>&, const c3ga::Mvec<double>&);

namespace Operators {
    inline c3ga::Mvec<double> 
    InnerProduct(const c3ga::Mvec<double>& first, 
                 const c3ga::Mvec<double>& second) { return first | second; }
                 
    inline c3ga::Mvec<double> 
    OuterProduct(const c3ga::Mvec<double>& first, 
                 const c3ga::Mvec<double>& second) { return first ^ second; }

    inline c3ga::Mvec<double> 
    GeomProduct(const c3ga::Mvec<double>& first, 
                const c3ga::Mvec<double>& second) { return first * second; }
}

enum ProviderType
{
    ProviderType_Explicit = 0,
    ProviderType_RandomGenerator,
    ProviderType_Subset,
    ProviderType_Combination,
    ProviderType_SelfCombination,
};

class Provider
{
public:
    virtual bool Compute(Layer& layer) = 0;
    virtual ProviderType GetType() const = 0;
    virtual inline uint32_t GetSourceCount() const { return 0; }
};


class Explicit : public Provider
{
public:
    inline bool IsAnimated() const { return m_simHandle.IsValid(); }
    void SetAnimated(const bool& animated);

    bool Compute(Layer& layer) override;
    inline ProviderType GetType() const override { return ProviderType_Explicit; };
    inline uint32_t GetSourceCount() const override { return 0; }

protected:
    SimulationHandle m_simHandle;
};

class RandomGenerator : public Explicit
{
public:
    RandomGenerator() : 
            m_objType(c3ga::MvecType::Point), 
            m_count(4), 
            m_extents(1.0) {}
    RandomGenerator(const c3ga::MvecType& objType, 
                    const uint32_t& count=4, 
                    const float& extents=1.0f) : 
           m_objType(objType), 
           m_count(count),
           m_extents(1.0) {}

    inline c3ga::MvecType GetObjectType() const { return m_objType; }
    inline void SetObjectType(const c3ga::MvecType& objType) { m_objType = objType; m_isDirty = true; }

    inline uint32_t GetCount() const { return m_count; }
    inline void SetCount(const uint32_t& count) { m_count = count; m_isDirty = true; }

    inline float GetExtents() const { return m_extents; }
    inline void SetExtents(const float& extents) { m_extents = extents; m_isDirty = true; }

    bool Compute(Layer& layer) override;
    inline ProviderType GetType() const override { return ProviderType_RandomGenerator; };
    inline uint32_t GetSourceCount() const override { return 0; }

private:
    bool m_isDirty = true;
    
    c3ga::MvecType m_objType;
    uint32_t m_count;
    float m_extents;
};


class Subset : public Provider
{
public:
    Subset(const int& count=-1) : m_count(count) {}

    inline int GetCount() const { return m_count; }
    inline void SetCount(const int& count) { m_count = count; }

    bool Compute(Layer& layer) override;
    inline ProviderType GetType() const override { return ProviderType_Subset; }
    inline uint32_t GetSourceCount() const override { return 1; }

private:
    int m_count; 
};


class OperatorBasedProvider : public Provider
{
public:
    OperatorBasedProvider(const Operator& op=Operators::OuterProduct) : m_op(op) {}

    inline Operator GetOperator() const { return m_op; }
    inline void SetOperator(const Operator& op) { m_op = op; }

    inline bool GetProductWithEi() const { return m_productWithEi; }
    inline void SetProductWithEi(const bool& productWithEi) { m_productWithEi = productWithEi; }

private:
    Operator m_op;
    bool m_productWithEi = false;
};


class SelfCombination : public OperatorBasedProvider
{
public:
    SelfCombination() : OperatorBasedProvider(), m_dimension(0), m_count(-1) {}
    SelfCombination(const uint32_t& dimension, 
                    const int& count=-1,
                    const Operator& op=Operators::OuterProduct) :
            OperatorBasedProvider(op),
            m_dimension(dimension), 
            m_count(count) {}

    inline int GetCount() const { return m_count; }
    inline void SetCount(const int& count) { m_count = count; }

    inline uint8_t GetDimension() const { return m_dimension; }
    inline void SetDimension(const uint8_t& dimension) { m_dimension = dimension; }

    bool Compute(Layer& layer) override;
    inline ProviderType GetType() const override { return ProviderType_SelfCombination; }
    inline uint32_t GetSourceCount() const override { return 1; }

private:
    int m_count;
    uint32_t m_dimension;

    std::vector<uint32_t> m_indices;
    int m_prevCount = 0;
    uint32_t m_prevDim = 0, m_prevSourceCount = 0;
    bool m_prevProductWithEi = false;
};


class Combination : public OperatorBasedProvider
{
public:
    Combination(const Operator& op=Operators::OuterProduct) : 
            OperatorBasedProvider(op) {}

    bool Compute(Layer& layer) override;
    inline ProviderType GetType() const override { return ProviderType_Combination; }
    inline uint32_t GetSourceCount() const override { return 2; }
};

#endif  // PROVIDER_HPP