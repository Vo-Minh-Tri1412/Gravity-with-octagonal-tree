#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

// Forward declaration
struct Body;
class PhysicsSystem;

/**
 * @brief GPU-accelerated physics wrapper
 *
 * This class provides a high-level interface for GPU physics.
 * When CUDA is available, it uses GPU for N-body simulation.
 * Falls back to CPU (OpenMP) when CUDA is not available.
 */
class PhysicsGPU
{
public:
    PhysicsGPU();
    ~PhysicsGPU();

    /**
     * @brief Initialize GPU physics with bodies
     * @param bodies Reference to the body vector
     * @return true if GPU is available and initialized
     */
    bool initialize(std::vector<Body> &bodies);

    /**
     * @brief Upload body data to GPU
     * @param bodies Body data to upload
     */
    void uploadBodies(const std::vector<Body> &bodies);

    /**
     * @brief Run one physics step on GPU
     * @param dt Time step
     * @param G Gravitational constant
     * @param blackHoleMass Central black hole mass
     * @param softening Softening parameter
     */
    void step(float dt, float G, float blackHoleMass, float softening = 0.5f);

    /**
     * @brief Download computed positions back to bodies
     * @param bodies Body vector to update
     */
    void downloadBodies(std::vector<Body> &bodies);

    /**
     * @brief Check if GPU acceleration is available
     */
    bool isGPUAvailable() const { return m_gpuAvailable; }

    /**
     * @brief Check if GPU physics is initialized
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief Get last kernel execution time in ms
     */
    float getLastKernelTimeMs() const { return m_lastKernelTimeMs; }

    /**
     * @brief Print CUDA device info
     */
    static void printDeviceInfo();

    /**
     * @brief Check if CUDA is available at runtime
     */
    static bool checkCUDAAvailable();

private:
    // Implementation pointer (PIMPL pattern to hide CUDA details)
    class Impl;
    std::unique_ptr<Impl> m_impl;

    // State tracking
    bool m_gpuAvailable = false;
    bool m_initialized = false;
    float m_lastKernelTimeMs = 0.0f;
    int m_numBodies = 0;

    // Host buffers for data transfer
    std::vector<float> m_positions;  // float3 layout (x,y,z,x,y,z,...)
    std::vector<float> m_velocities; // float3 layout
    std::vector<float> m_masses;
};
