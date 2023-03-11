#ifndef LAYER_H
#define LAYER_H

#include <c3ga/Mvec.hpp>

#include <vector>
#include <memory>
#include <functional>

using MvecArray = std::vector<c3ga::Mvec<float>>;
using Operator = std::function<c3ga::Mvec<float>(const c3ga::Mvec<float>&, 
                                                 const c3ga::Mvec<float>&)>;

class Layer;
class Layer;
class Mapping; 
class NoMapping; 
class Subset; 
class Combination;

using LayerPtr = std::shared_ptr<Layer>;
using LayerWeakPtr = std::weak_ptr<Layer>;
using LayerPtrArray = std::vector<LayerPtr>;
using LayerWeakPtrArray = std::vector<LayerWeakPtr>;

using MappingPtr = std::shared_ptr<Mapping>;
using MappingWeakPtr = std::weak_ptr<Mapping>;



class Layer
{
public:
    Layer(const std::string& name, const MvecArray& objects);
    Layer(const std::string& name, const MappingPtr& mapping, const Operator& op);
    ~Layer() = default;

    inline std::string GetName() const { return m_name; }

    inline const MvecArray& Get() const { return m_objects; }
    inline MvecArray& Get() { return m_objects; }
    inline void Set(const MvecArray& objects) { m_objects = objects; }
    inline c3ga::Mvec<float>& operator[](const uint32_t& idx) { return m_objects[idx]; }
    inline const c3ga::Mvec<float>& operator[](const uint32_t& idx) const { return m_objects[idx]; }
    inline void Clear() { m_objects.clear(); }

    LayerWeakPtrArray GetSources() const;
    virtual void SetSources(const LayerWeakPtrArray& layers);
    virtual void AddSource(const LayerWeakPtr& layer);
    virtual void RemoveSource(const LayerWeakPtr& layer);

    LayerWeakPtrArray GetDestinations() const;
    virtual void SetDestinations(const LayerWeakPtrArray& layers);
    virtual void AddDestination(const LayerWeakPtr& layer);
    virtual void RemoveDestination(const LayerWeakPtr& layer);

    void SetDirty(const bool& dirty);
    inline bool IsDirty() const { return m_isDirty; }

    inline Operator GetOperator() const { return m_op; }
    void SetOperator(const Operator& op) { m_op = op; }

    inline MappingPtr GetMapping() const { return m_mapping; }
    void SetMapping(const MappingPtr& mapping) { m_mapping = mapping; }

    void Update();

    inline MvecArray::iterator begin()             { return m_objects.begin(); }
    inline MvecArray::iterator end()               { return m_objects.end(); }
    inline MvecArray::const_iterator begin() const { return m_objects.begin(); }
    inline MvecArray::const_iterator end()   const { return m_objects.end(); }

    inline bool operator==(const Layer& other) { return true; }

public:
    // Operators
    inline static c3ga::Mvec<float> InnerOp(const c3ga::Mvec<float>& first, 
                                            const c3ga::Mvec<float>& second) { return first | second; }
    inline static c3ga::Mvec<float> OuterOp(const c3ga::Mvec<float>& first, 
                                            const c3ga::Mvec<float>& second) { return first ^ second; }
    inline static c3ga::Mvec<float> GeomOp(const c3ga::Mvec<float>& first, 
                                           const c3ga::Mvec<float>& second) { return first * second; }

private:
    std::string m_name;

    MvecArray m_objects;
    MappingPtr m_mapping;
    Operator m_op;

    LayerWeakPtrArray m_sources;
    LayerWeakPtrArray m_destinations;
    bool m_isDirty = true;
};



class Mapping
{
public:
    virtual void Compute(const Operator& op, Layer& layer) = 0;
};

class NoMapping : public Mapping
{
public:
    NoMapping() = default;

    void Compute(const Operator& op, Layer& layer) override {}
};

class Subset : public Mapping
{
public:
    Subset() = default;
    Subset(const uint32_t& dimension, 
           const int& count=-1) :
            m_dimension(dimension), 
            m_count(count) {}

    inline int GetCount() const { return m_count; }
    inline void SetCount(const int& count) { m_count = count; }

    inline uint8_t GetDimension() const { return m_dimension; }
    inline void SetDimension(const uint8_t& dimension) { m_dimension = dimension; }

    void Compute(const Operator& op, Layer& layer) override;

private:
    int m_count;
    uint32_t m_dimension;

    std::vector<uint32_t> m_indices;
};


class Combination : public Mapping
{
public:
    Combination() = default;

    void Compute(const Operator& op, Layer& layer) override;
};

#endif
