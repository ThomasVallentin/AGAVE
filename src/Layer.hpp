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


enum DirtyBits
{
    DirtyBits_None = 0,
    DirtyBits_Dual = 1 << 0,
    DirtyBits_Provider = 1 << 2,
};


class Layer
{
public:
    Layer(const std::string& name, const MvecArray& objects);
    Layer(const std::string& name, const ProviderPtr& provider);
    ~Layer() = default;

    inline std::string GetName() const { return m_name; }
    inline void SetName(const std::string& name) { m_name = name; }
    inline uint32_t GetUUID() const { return m_uuid; }

    inline const MvecArray& GetObjects() const { return m_objects; }
    inline MvecArray& GetObjects() { return m_objects; }
    inline void SetObjects(const MvecArray& objects) { m_objects = objects; SetDirty(DirtyBits_Provider); }
    inline c3ga::Mvec<double>& operator[](const uint32_t& idx) { return m_objects[idx]; }
    inline const c3ga::Mvec<double>& operator[](const uint32_t& idx) const { return m_objects[idx]; }
    inline void Clear() { m_objects.clear(); }

    LayerWeakPtrArray GetSources() const;
    virtual void AddSource(const LayerWeakPtr& layer);
    virtual void RemoveSource(const LayerWeakPtr& layer);
    bool SourceIsDual(const uint32_t& index) const;
    void SetSourceDual(const uint32_t& index, const bool& dual);

    LayerWeakPtrArray GetDestinations() const;
    virtual void AddDestination(const LayerWeakPtr& layer);
    virtual void RemoveDestination(const LayerWeakPtr& layer);

    void SetDirty(const DirtyBits& dirtyBits);
    inline bool IsDirty() const { return m_dirtyBits != DirtyBits_None; }
    DirtyBits GetDirtyBits() const { return m_dirtyBits; }

    inline ProviderPtr GetProvider() const { return m_provider; }
    void SetProvider(const ProviderPtr& provider);

    inline bool IsVisible() const { return m_visibility; }
    inline bool& GetVisiblity() { return m_visibility; }
    inline void SetVisible(const bool& visibility) { m_visibility = visibility; }

    inline bool IsDual() const { return m_isDual; }
    void SetDual(const bool& dual);

    bool Update();

    inline MvecArray::iterator begin()             { return m_objects.begin(); }
    inline MvecArray::iterator end()               { return m_objects.end(); }
    inline MvecArray::const_iterator begin() const { return m_objects.begin(); }
    inline MvecArray::const_iterator end()   const { return m_objects.end(); }

    inline bool operator==(const Layer& other) { return true; }

private:
    std::string m_name;
    uint32_t m_uuid;
    bool m_visibility;

    MvecArray m_objects;
    ProviderPtr m_provider;

    LayerWeakPtrArray m_sources;
    std::vector<bool> m_dualSources;
    LayerWeakPtrArray m_destinations;
    DirtyBits m_dirtyBits = DirtyBits_Provider;
    bool m_isDual;
};


#endif  // LAYER_HPP
