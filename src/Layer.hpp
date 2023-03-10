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
class Link; 
class Subset; 
class Combination;

using LayerPtr = std::shared_ptr<Layer>;
using LayerWeakPtr = std::weak_ptr<Layer>;
using LinkPtr = std::shared_ptr<Link>;
using LinkWeakPtr = std::weak_ptr<Link>;


class Layer
{
public:
    Layer() = default;
    Layer(const MvecArray& objects) : m_objects(objects) {}
    ~Layer() = default;

    inline const MvecArray& Get() const { return m_objects; }
    inline MvecArray& Get() { return m_objects; }
    inline void Set(const MvecArray& objects) { m_objects = objects; }
    inline c3ga::Mvec<float>& operator[](const uint32_t& idx) { return m_objects[idx]; }
    inline const c3ga::Mvec<float>& operator[](const uint32_t& idx) const { return m_objects[idx]; }
    inline void Clear() { m_objects.clear(); }

    void AddDependency(const LinkWeakPtr& link);
    void RemoveDependency(const LinkWeakPtr& link);
    void Invalidate();

    inline MvecArray::iterator begin()             { return m_objects.begin(); }
    inline MvecArray::iterator end()               { return m_objects.end(); }
    inline MvecArray::const_iterator begin() const { return m_objects.begin(); }
    inline MvecArray::const_iterator end()   const { return m_objects.end(); }

    inline bool operator==(const Layer& other) { return true; }

private:
    MvecArray m_objects;
    std::vector<LinkWeakPtr> m_dependencies;
};


class Link
{
public:
    Link(const Operator& op=nullptr) : m_op(op) {}

    inline Operator GetOperator(const Operator& op) { return m_op; }
    inline void SetOperator(const Operator& op) { m_op = op; }
    virtual void Update(const LayerPtr& destination) = 0;

    void AddDependency(const LayerWeakPtr& layer);
    void RemoveDependency(const LayerWeakPtr& layer);
    virtual void Invalidate();

    inline static c3ga::Mvec<float> InnerOp(const c3ga::Mvec<float>& first, 
                                            const c3ga::Mvec<float>& second) { return first | second; }
    inline static c3ga::Mvec<float> OuterOp(const c3ga::Mvec<float>& first, 
                                            const c3ga::Mvec<float>& second) { return first ^ second; }
    inline static c3ga::Mvec<float> GeomOp(const c3ga::Mvec<float>& first, 
                                           const c3ga::Mvec<float>& second) { return first * second; }

    inline bool operator==(const Link& other) { return true; }

protected:
    Operator m_op;
    std::vector<LayerWeakPtr> m_dependencies;
};


class Subset : public Link
{
public:
    Subset() : Link() {};
    Subset(const LayerPtr& source,
           const uint32_t& dimension, 
           const int& count=-1, 
           const Operator& op=nullptr) :
            Link(op), 
            m_source(source), 
            m_dimension(dimension), 
            m_count(count) {}

    inline int GetCount() const { return m_count; }
    inline void SetCount(const int& count) { m_count = count; }

    inline uint8_t GetDimension() const { return m_dimension; }
    inline void SetDimension(const uint8_t& dimension) { m_dimension = dimension; }

    void Update(const LayerPtr& destination) override;
    void Invalidate() override;

private:
    LayerPtr m_source;

    int m_count;
    uint32_t m_dimension;

    std::vector<uint32_t> m_indices;
};


class Combination : public Link
{
public:
    Combination() : Link() {};
    Combination(const LayerPtr& source1,
                const LayerPtr& source2, 
                const Operator& op=nullptr) :
            Link(op), 
            m_source1(source1), 
            m_source2(source2) {}

    using LayerPair = std::pair<LayerPtr, LayerPtr>;

    inline LayerPair GetSources() const { return {m_source1, m_source2}; }
    void SetSources(const LayerPair& sources);
    void SetSources(const LayerPtr& source1,
                    const LayerPtr& source2);


    void Update(const LayerPtr& destination) override;

private:
    LayerPtr m_source1;
    LayerPtr m_source2;
};

#endif