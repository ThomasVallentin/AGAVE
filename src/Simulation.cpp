#include "Simulation.hpp"

#include "Base/Logging.h"

#include "c3gaTools.hpp"
#include "C3GAUtils.hpp"

#include <iostream>
#include <random>


SimulationEngine* SimulationEngine::s_instance = nullptr;


static std::random_device uuidGenerator;
static std::uniform_int_distribution<uint32_t> nextUuidDistrib(1, 1024);


SimulationEngine& SimulationEngine::Init()
{
    if (!s_instance)
        s_instance = new SimulationEngine();

    return *s_instance;
}

SimulationEngine::SimulationEngine() :
    m_forces({})
{
    std::uniform_int_distribution<uint32_t> dist(1, UINT32_MAX);
    m_lastUUID = dist(uuidGenerator);
}

SimulationEngine::~SimulationEngine()
{
}


SimulationHandle SimulationEngine::NewSimulation()
{
    m_lastUUID += nextUuidDistrib(uuidGenerator);
    m_simulations.insert({m_lastUUID, {}});

    return {m_lastUUID};
}

void SimulationEngine::RemoveSimulation(const SimulationHandle& handle)
{
    auto it = m_simulations.find(handle.GetId());
    if (it != m_simulations.end())
    {
        m_simulations.erase(it);
    }
}


void SimulationEngine::Update(const double &deltaTime)
{
    for (auto& simulation : m_simulations)
    {
        for (auto& obj : simulation.second) {
            for (auto& force : m_forces) {
                force(obj);
            }

            obj.Update(deltaTime);
            // ComputeIntersections(obj);
            // obj.position = c3ga::point(obj.position[c3ga::E1], 
            //                            obj.position[c3ga::E2], 
            //                            obj.position[c3ga::E3]);
        }
    }
}

void SimulationEngine::ComputeIntersections(SimObject &object)
{
    if (object.object[c3ga::E1] < -1.1f) {
        object.object[c3ga::E1] = -1.1f;
        object.velocity[c3ga::E1] *= -1.0f;
    }
    else 
    if (object.object[c3ga::E1] > 1.1f) {
        object.object[c3ga::E1] = 1.1f;
        object.velocity[c3ga::E1] *= -1.0f;
    }

    if (object.object[c3ga::E2] < -1.1f) {
        object.object[c3ga::E2] = -1.1f;
        object.velocity[c3ga::E2] *= -1.0f;
    }
    else 
    if (object.object[c3ga::E2] > 1.1f) {
        object.object[c3ga::E2] = 1.1f;
        object.velocity[c3ga::E2] *= -1.0f;
    }

    if (object.object[c3ga::E3] < -1.1f) {
        object.object[c3ga::E3] = -1.1f;
        object.velocity[c3ga::E3] *= -1.0f;
    }
    else 
    if (object.object[c3ga::E3] > 1.1f) {
        object.object[c3ga::E3] = 1.1f;
        object.velocity[c3ga::E3] *= -1.0f;
    }
}

SimObjectArray& SimulationEngine::GetSimObjects(const SimulationHandle& handle)
{
    return m_simulations.at(handle.GetId());
}

const SimObjectArray& SimulationEngine::GetSimObjects(const SimulationHandle& handle) const
{
    return m_simulations.at(handle.GetId());
}

MvecArray SimulationHandle::GetObjects() const
{
    auto& engine = SimulationEngine::Get();
    const SimObjectArray& simObjects = engine.GetSimObjects(*this);
    MvecArray result(simObjects.size());

    size_t i = 0;
    for (const auto& simObj : simObjects) {
        result[i] = simObj.object;
        ++i;
    }

    return result;
}

void SimulationHandle::SetObjects(const MvecArray& objects)
{
    auto& engine = SimulationEngine::Get();
    SimObjectArray& simObjects = engine.GetSimObjects(*this);
    simObjects.resize(objects.size());

    size_t i = 0;
    for (auto& simObj : simObjects)
    {
        simObj.object = objects[i];
        simObj.velocity = c3ga::randomVector<double>();
        ++i;
    }
}


void SimObject::Update(const double& deltaTime)
{
    auto translator = c3ga::translator(velocity * deltaTime);
    object = translator * object * translator.inv();
    // Rounding to make sure precision issues don't mess up the rest of the program
    object.roundZero(1e-6);  
    accumulatedForces = c3ga::Mvec<double>(0);
}