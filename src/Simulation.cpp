#include "Simulation.hpp"

#include <iostream>
#include <random>


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
    }
}
