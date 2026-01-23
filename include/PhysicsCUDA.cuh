#pragma once

#ifdef USE_CUDA

#include <vector>
#include <glm/glm.hpp>

// ============================================================================
// CUDA N-Body Physics Engine
// Tính toán lực hấp dẫn trên GPU với shared memory optimization
// ============================================================================

struct BodyGPU
{
    float3 position;
    float3 velocity;
    float3 acceleration;
    float mass;
};

class PhysicsCUDA
{
public:
    PhysicsCUDA();
    ~PhysicsCUDA();

    // Initialize CUDA resources
    void init(int numBodies);

    // Upload bodies from CPU to GPU
    void uploadBodies(const float *positions, const float *velocities,
                      const float *masses, int count);

    // Run one physics step on GPU
    void step(float dt, float G, float blackHoleMass, float softening);

    // Download positions back to CPU (for rendering)
    void downloadPositions(float *positions);
    void downloadVelocities(float *velocities);

    // Get device pointer for OpenGL interop (advanced)
    float3 *getDevicePositions() { return d_positions; }

    int getNumBodies() const { return m_numBodies; }
    bool isInitialized() const { return m_initialized; }

    // Performance info
    float getLastKernelTimeMs() const { return m_lastKernelTimeMs; }

private:
    // Device memory pointers
    float3 *d_positions = nullptr;
    float3 *d_velocities = nullptr;
    float3 *d_accelerations = nullptr;
    float *d_masses = nullptr;

    int m_numBodies = 0;
    bool m_initialized = false;
    float m_lastKernelTimeMs = 0.0f;

    // CUDA events for timing
    void *m_startEvent = nullptr;
    void *m_stopEvent = nullptr;
};

// Factory function to check CUDA availability
bool isCUDAAvailable();
int getCUDADeviceCount();
void printCUDADeviceInfo();

#endif // USE_CUDA
