#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "Layer.hpp"

#include <c3ga/Mvec.hpp>

#include <vector>

struct Object
{
    c3ga::Mvec<float> position;
    c3ga::Mvec<float> velocity;
    c3ga::Mvec<float> accumulatedForces;

    void update(const double& deltaTime) {
        // velocity += accumulatedForces * deltaTime;
        position += (velocity + accumulatedForces) * deltaTime;
        accumulatedForces = c3ga::Mvec<float>(0);
    }
};

using Force = std::function<void(Object&)>;

inline Force LinearForce(c3ga::Mvec<float> force) {
    return [force](Object& object) {
        object.accumulatedForces += force;
    };
}

class SimulationEngine
{
public:
    static SimulationEngine& Init();
    inline static SimulationEngine& Get() { return *s_instance; }

    void AddLayer(const LayerPtr& layer);

    void Update(const double& deltaTime);
    void ComputeIntersections(Object& object);
    inline std::vector<Object>& GetObjects() { return m_objects; }
    inline const std::vector<Object>& GetObjects() const { return m_objects; }

private:
    SimulationEngine();
    ~SimulationEngine();

    static SimulationEngine* s_instance;

    std::vector<Object> m_objects;
    std::vector<Force> m_forces;
};

#endif
