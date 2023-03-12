#include "Simulation.hpp"

#include <iostream>
#include <random>


SimulationEngine* SimulationEngine::s_instance = nullptr;


SimulationEngine& SimulationEngine::Init()
{
    if (!s_instance)
        s_instance = new SimulationEngine();

    return *s_instance;
}

SimulationEngine::SimulationEngine() :
    m_forces({})
{
}

SimulationEngine::~SimulationEngine()
{
}

void SimulationEngine::Update(const double &deltaTime)
{
    for (auto& obj : m_objects) {
        for (auto& force : m_forces) {
            force(obj);
        }

        obj.update(deltaTime);
        ComputeIntersections(obj);
    }
}

void SimulationEngine::ComputeIntersections(Object &object)
{
    if (object.position[c3ga::E1] < -1.1f) {
        object.position[c3ga::E1] = -1.1f;
        object.velocity[c3ga::E1] *= -1.0f;
    }
    else 
    if (object.position[c3ga::E1] > 1.1f) {
        object.position[c3ga::E1] = 1.1f;
        object.velocity[c3ga::E1] *= -1.0f;
    }

    if (object.position[c3ga::E2] < -1.1f) {
        object.position[c3ga::E2] = -1.1f;
        object.velocity[c3ga::E2] *= -1.0f;
    }
    else 
    if (object.position[c3ga::E2] > 1.1f) {
        object.position[c3ga::E2] = 1.1f;
        object.velocity[c3ga::E2] *= -1.0f;
    }

    if (object.position[c3ga::E3] < -1.1f) {
        object.position[c3ga::E3] = -1.1f;
        object.velocity[c3ga::E3] *= -1.0f;
    }
    else 
    if (object.position[c3ga::E3] > 1.1f) {
        object.position[c3ga::E3] = 1.1f;
        object.velocity[c3ga::E3] *= -1.0f;
    }
}
