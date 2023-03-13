#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "Layer.hpp"

#include <c3ga/Mvec.hpp>
#include <c3gaTools.hpp>

#include <unordered_map>
#include <vector>

struct SimObject
{
    c3ga::Mvec<double> object;
    c3ga::Mvec<double> velocity;
    c3ga::Mvec<double> accumulatedForces;

    void Update(const double& deltaTime);
};

using SimObjectArray = std::vector<SimObject>;

using Force = std::function<void(SimObject&)>;

inline Force LinearForce(c3ga::Mvec<double> force) {
    return [force](SimObject& object) {
        object.accumulatedForces += force;
    };
}

class SimulationEngine;

class SimulationHandle
{
public:
    SimulationHandle() : m_id(0) {}

    inline uint32_t GetId() const { return m_id; }
    inline bool IsValid() const { return m_id != 0; }

    MvecArray GetObjects() const;
    void SetObjects(const MvecArray& objects);

    bool operator==(const SimulationHandle& other) const { return m_id == other.m_id; }

private:
    friend SimulationEngine;

    SimulationHandle(const uint32_t& id) : m_id(id) {}

    uint32_t m_id;
};

class SimulationEngine
{
public:
    static SimulationEngine& Init();
    inline static SimulationEngine& Get() { return *s_instance; }

    SimulationHandle NewSimulation();
    void RemoveSimulation(const SimulationHandle& handle);

    void Update(const double& deltaTime);

private:
    friend SimulationHandle;

    SimObjectArray& GetSimObjects(const SimulationHandle& handle);
    const SimObjectArray& GetSimObjects(const SimulationHandle& handle) const;

    void ComputeIntersections(SimObject& object);

    SimulationEngine();
    ~SimulationEngine();

    static SimulationEngine* s_instance;

    std::unordered_map<uint32_t, SimObjectArray> m_simulations;
    std::vector<Force> m_forces;

    uint32_t m_lastUUID;
};

#endif
