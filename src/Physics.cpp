#include "Physics.hpp"
#include "Octree.hpp"
#include <omp.h> // Parallelization
#include <cmath>
#include <algorithm>

// 2. Physics Engine: Data-Oriented & Stable
// Integrator: Velocity Verlet

void PhysicsSystem::setBodies(const std::vector<Body> &bodies)
{
    m_bodies = bodies;
}

void PhysicsSystem::step(float dt)
{
    // 1. First Verlet Step: r(t+dt) = r(t) + v(t)dt + 0.5*a(t)dt^2
    //    v(t+0.5dt) = v(t) + 0.5*a(t)dt

#pragma omp parallel for
    for (size_t i = 0; i < m_bodies.size(); ++i)
    {
        if (m_bodies[i].isStatic)
            continue;

        m_bodies[i].position += m_bodies[i].velocity * dt + 0.5f * m_bodies[i].acceleration * dt * dt;
        m_bodies[i].velocity += 0.5f * m_bodies[i].acceleration * dt; // Half step velocity
    }

    // 2. Update Tree (Rebuild)
    // Extract positions for tree build to maximize cache locality
    std::vector<glm::vec3> positions(m_bodies.size());
    std::vector<float> masses(m_bodies.size());

#pragma omp parallel for
    for (size_t i = 0; i < m_bodies.size(); ++i)
    {
        positions[i] = m_bodies[i].position;
        masses[i] = m_bodies[i].mass;
    }

    m_octree.build(positions, masses);

// 3. Calculate New Forces a(t+dt) using Barnes-Hut
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < m_bodies.size(); ++i)
    {
        if (m_bodies[i].isStatic)
            continue;

        glm::vec3 force = m_octree.calculateForce(m_bodies[i].position);

        // Add central black hole force explicitly if not in tree
        glm::vec3 bhDir = glm::vec3(0.0f) - m_bodies[i].position;
        float dist = glm::length(bhDir);
        // Softening kernel
        float softening = 1.0f;
        float f = (G * m_blackHoleMass * m_bodies[i].mass) / pow(dist * dist + softening * softening, 1.5f);
        force += bhDir * f;

        m_bodies[i].acceleration = force / m_bodies[i].mass;
    }

// 4. Second Verlet Step: v(t+dt) = v(t+0.5dt) + 0.5*a(t+dt)dt
#pragma omp parallel for
    for (size_t i = 0; i < m_bodies.size(); ++i)
    {
        if (m_bodies[i].isStatic)
            continue;
        m_bodies[i].velocity += 0.5f * m_bodies[i].acceleration * dt;
    }
}