#ifndef SIMULATION_H
#define SIMULATION_H

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
    SimulationEngine();
    ~SimulationEngine();

    void Update(const double& deltaTime);
    void ComputeIntersections();
    inline std::vector<Object>& GetObjects() { return m_objects; }
    inline const std::vector<Object>& GetObjects() const { return m_objects; }

private:
    std::vector<Object> m_objects;
    std::vector<Force> m_forces;
};

#endif
