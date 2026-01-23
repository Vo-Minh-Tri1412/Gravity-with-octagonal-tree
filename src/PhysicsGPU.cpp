#include "PhysicsGPU.hpp"
#include "Physics.hpp"
#include <iostream>

#ifdef USE_CUDA
#include "PhysicsCUDA.cuh"
#endif

// ============================================================================
// Implementation class (PIMPL pattern)
// ============================================================================
class PhysicsGPU::Impl
{
public:
#ifdef USE_CUDA
    std::unique_ptr<PhysicsCUDA> cuda;
#endif
};

// ============================================================================
// PhysicsGPU Implementation
// ============================================================================

PhysicsGPU::PhysicsGPU() : m_impl(std::make_unique<Impl>())
{
    m_gpuAvailable = checkCUDAAvailable();

    if (m_gpuAvailable)
    {
#ifdef USE_CUDA
        m_impl->cuda = std::make_unique<PhysicsCUDA>();
        std::cout << "[PhysicsGPU] CUDA GPU acceleration available!" << std::endl;
#endif
    }
    else
    {
        std::cout << "[PhysicsGPU] CUDA not available, using CPU physics" << std::endl;
    }
}

PhysicsGPU::~PhysicsGPU() = default;

bool PhysicsGPU::initialize(std::vector<Body> &bodies)
{
    m_numBodies = static_cast<int>(bodies.size());

    if (!m_gpuAvailable)
    {
        return false;
    }

#ifdef USE_CUDA
    // Allocate host buffers
    m_positions.resize(m_numBodies * 3);
    m_velocities.resize(m_numBodies * 3);
    m_masses.resize(m_numBodies);

    // Initialize CUDA
    m_impl->cuda->init(m_numBodies);

    // Upload initial data
    uploadBodies(bodies);

    m_initialized = true;
    std::cout << "[PhysicsGPU] Initialized with " << m_numBodies << " bodies" << std::endl;
    return true;
#else
    return false;
#endif
}

void PhysicsGPU::uploadBodies(const std::vector<Body> &bodies)
{
#ifdef USE_CUDA
    if (!m_gpuAvailable || !m_impl->cuda)
        return;

    // Convert Body data to GPU-friendly format
    for (size_t i = 0; i < bodies.size(); ++i)
    {
        const Body &b = bodies[i];

        // Position (float3 layout)
        m_positions[i * 3 + 0] = b.position.x;
        m_positions[i * 3 + 1] = b.position.y;
        m_positions[i * 3 + 2] = b.position.z;

        // Velocity (float3 layout)
        m_velocities[i * 3 + 0] = b.velocity.x;
        m_velocities[i * 3 + 1] = b.velocity.y;
        m_velocities[i * 3 + 2] = b.velocity.z;

        // Mass
        m_masses[i] = b.mass;
    }

    m_impl->cuda->uploadBodies(
        m_positions.data(),
        m_velocities.data(),
        m_masses.data(),
        static_cast<int>(bodies.size()));
#endif
}

void PhysicsGPU::step(float dt, float G, float blackHoleMass, float softening)
{
#ifdef USE_CUDA
    if (!m_initialized || !m_impl->cuda)
        return;

    m_impl->cuda->step(dt, G, blackHoleMass, softening);
    m_lastKernelTimeMs = m_impl->cuda->getLastKernelTimeMs();
#else
    (void)dt;
    (void)G;
    (void)blackHoleMass;
    (void)softening;
#endif
}

void PhysicsGPU::downloadBodies(std::vector<Body> &bodies)
{
#ifdef USE_CUDA
    if (!m_initialized || !m_impl->cuda)
        return;

    // Download positions and velocities
    m_impl->cuda->downloadPositions(m_positions.data());
    m_impl->cuda->downloadVelocities(m_velocities.data());

    // Convert back to Body format
    for (size_t i = 0; i < bodies.size(); ++i)
    {
        Body &b = bodies[i];

        // Position
        b.position.x = m_positions[i * 3 + 0];
        b.position.y = m_positions[i * 3 + 1];
        b.position.z = m_positions[i * 3 + 2];

        // Velocity
        b.velocity.x = m_velocities[i * 3 + 0];
        b.velocity.y = m_velocities[i * 3 + 1];
        b.velocity.z = m_velocities[i * 3 + 2];
    }
#else
    (void)bodies;
#endif
}

void PhysicsGPU::printDeviceInfo()
{
#ifdef USE_CUDA
    printCUDADeviceInfo();
#else
    std::cout << "[PhysicsGPU] CUDA not compiled in this build" << std::endl;
#endif
}

bool PhysicsGPU::checkCUDAAvailable()
{
#ifdef USE_CUDA
    return isCUDAAvailable();
#else
    return false;
#endif
}
