#ifndef PROVIDER_HPP
#define PROVIDER_HPP

#include "Layer.hpp"


using Operator = c3ga::Mvec<double>(*)(const c3ga::Mvec<double>&, const c3ga::Mvec<double>&);


enum ProviderType
{
    ProviderType_None = 0,
    ProviderType_Static           = 1 << 0,
    ProviderType_Subset           = 1 << 1,
    ProviderType_Combination      = 1 << 2,
    ProviderType_SelfCombination  = 1 << 3,
};

class Provider
{
public:
    virtual void Compute(Layer& layer) = 0;
    virtual ProviderType GetType() const = 0;
    virtual inline uint32_t GetSourceCount() const { return 0; }
};

class Static : public Provider
{
public:
    inline void Compute(Layer& layer) override {};
    inline ProviderType GetType() const override { return ProviderType_Static; };
    inline uint32_t GetSourceCount() const override { return 0; }
};

class Subset : public Provider
{
public:
    Subset(const int& count=-1) : m_count(count) {}

    inline int GetCount() const { return m_count; }
    inline void SetCount(const int& count) { m_count = count; }

    void Compute(Layer& layer) override;
    inline ProviderType GetType() const override { return ProviderType_Subset; }
    inline uint32_t GetSourceCount() const override { return 1; }

private:
    int m_count; 
};

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

class OperatorBasedProvider : public Provider
{
public:
    OperatorBasedProvider(const Operator& op=Operators::OuterProduct) : m_op(op) {}

    inline Operator GetOperator() const { return m_op; }
    void SetOperator(const Operator& op);

private:
    Operator m_op;
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

    void Compute(Layer& layer) override;
    inline ProviderType GetType() const override { return ProviderType_SelfCombination; }
    inline uint32_t GetSourceCount() const override { return 1; }

private:
    int m_count;
    uint32_t m_dimension;

    std::vector<uint32_t> m_indices;
    uint32_t m_prevCount, m_prevDim, m_prevSourceCount;
};

class Combination : public OperatorBasedProvider
{
public:
    Combination(const Operator& op=Operators::OuterProduct) : 
            OperatorBasedProvider(op) {}

    void Compute(Layer& layer) override;
    inline ProviderType GetType() const override { return ProviderType_Combination; }
    inline uint32_t GetSourceCount() const override { return 2; }
};

#endif  // PROVIDER_HPP