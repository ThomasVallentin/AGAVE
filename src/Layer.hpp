#ifndef LAYER_HPP
#define LAYER_HPP

#include <c3ga/Mvec.hpp>

#include <vector>
#include <memory>
#include <functional>

using MvecArray = std::vector<c3ga::Mvec<double>>;


class Layer;
class Provider; 

using LayerPtr = std::shared_ptr<Layer>;
using LayerWeakPtr = std::weak_ptr<Layer>;
using LayerPtrArray = std::vector<LayerPtr>;
using LayerWeakPtrArray = std::vector<LayerWeakPtr>;

using ProviderPtr = std::shared_ptr<Provider>;
using ProviderWeakPtr = std::weak_ptr<Provider>;


class Layer
{
public:
    Layer(const std::string& name, const MvecArray& objects);
    Layer(const std::string& name, const ProviderPtr& provider);
    ~Layer() = default;

    inline std::string GetName() const { return m_name; }

    inline const MvecArray& GetObjects() const { return m_objects; }
    inline MvecArray& GetObjects() { return m_objects; }
    inline void SetObjects(const MvecArray& objects) { m_objects = objects; SetDirty(true); }
    inline c3ga::Mvec<double>& operator[](const uint32_t& idx) { return m_objects[idx]; }
    inline const c3ga::Mvec<double>& operator[](const uint32_t& idx) const { return m_objects[idx]; }
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

    inline ProviderPtr GetProvider() const { return m_provider; }
    void SetProvider(const ProviderPtr& provider);

    inline bool IsVisible() const { return m_visibility; }
    inline bool& GetVisiblity() { return m_visibility; }
    inline void SetVisiblity(const bool& visibility) { m_visibility = visibility; }

    bool Update();

    inline MvecArray::iterator begin()             { return m_objects.begin(); }
    inline MvecArray::iterator end()               { return m_objects.end(); }
    inline MvecArray::const_iterator begin() const { return m_objects.begin(); }
    inline MvecArray::const_iterator end()   const { return m_objects.end(); }

    inline bool operator==(const Layer& other) { return true; }

private:
    std::string m_name;
    bool m_visibility;

    MvecArray m_objects;
    ProviderPtr m_provider;

    LayerWeakPtrArray m_sources;
    LayerWeakPtrArray m_destinations;
    bool m_isDirty = true;
};


#endif  // LAYER_HPP
