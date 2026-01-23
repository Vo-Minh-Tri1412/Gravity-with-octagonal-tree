#ifdef USE_CUDA

#include "PhysicsCUDA.cuh"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <math.h>

// ============================================================================
// CUDA Configuration
// ============================================================================
#define BLOCK_SIZE 256     // Threads per block (must be power of 2)
#define SOFTENING_SQ 0.25f // Softening squared to prevent singularity

// ============================================================================
// CUDA Error Checking Macro
// ============================================================================
#define CUDA_CHECK(call)                                                      \
    do                                                                        \
    {                                                                         \
        cudaError_t err = call;                                               \
        if (err != cudaSuccess)                                               \
        {                                                                     \
            fprintf(stderr, "CUDA Error at %s:%d - %s\n", __FILE__, __LINE__, \
                    cudaGetErrorString(err));                                 \
        }                                                                     \
    } while (0)

// ============================================================================
// CUDA Kernels
// ============================================================================

// Helper: make_float3 operations
__device__ __forceinline__ float3 operator+(float3 a, float3 b)
{
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__device__ __forceinline__ float3 operator-(float3 a, float3 b)
{
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

__device__ __forceinline__ float3 operator*(float3 a, float s)
{
    return make_float3(a.x * s, a.y * s, a.z * s);
}

__device__ __forceinline__ float3 operator+=(float3 &a, float3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

// ============================================================================
// Kernel 1: Compute gravitational forces using tiled shared memory
// This is O(N²) but highly optimized with shared memory
// ============================================================================
__global__ void computeForcesKernel(
    const float3 *__restrict__ positions,
    const float *__restrict__ masses,
    float3 *__restrict__ accelerations,
    int numBodies,
    float G,
    float blackHoleMass,
    float softening)
{
    // Shared memory for tile of bodies
    __shared__ float4 sharedBodies[BLOCK_SIZE]; // x, y, z, mass

    int i = blockIdx.x * blockDim.x + threadIdx.x;

    float3 myPos = (i < numBodies) ? positions[i] : make_float3(0, 0, 0);
    float3 acc = make_float3(0.0f, 0.0f, 0.0f);

    float softeningSq = softening * softening;

    // Process all bodies in tiles
    int numTiles = (numBodies + BLOCK_SIZE - 1) / BLOCK_SIZE;

    for (int tile = 0; tile < numTiles; tile++)
    {
        // Load tile into shared memory
        int loadIdx = tile * BLOCK_SIZE + threadIdx.x;
        if (loadIdx < numBodies)
        {
            float3 p = positions[loadIdx];
            sharedBodies[threadIdx.x] = make_float4(p.x, p.y, p.z, masses[loadIdx]);
        }
        else
        {
            sharedBodies[threadIdx.x] = make_float4(0, 0, 0, 0);
        }

        __syncthreads();

        // Compute interactions with all bodies in this tile
        if (i < numBodies)
        {
#pragma unroll 32
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                float4 other = sharedBodies[j];

                float3 r;
                r.x = other.x - myPos.x;
                r.y = other.y - myPos.y;
                r.z = other.z - myPos.z;

                float distSq = r.x * r.x + r.y * r.y + r.z * r.z + softeningSq;
                float invDist = rsqrtf(distSq);
                float invDist3 = invDist * invDist * invDist;

                float force = G * other.w * invDist3;

                acc.x += r.x * force;
                acc.y += r.y * force;
                acc.z += r.z * force;
            }
        }

        __syncthreads();
    }

    // Add central black hole force
    if (i < numBodies)
    {
        float3 bhDir = make_float3(-myPos.x, -myPos.y, -myPos.z);
        float distSq = bhDir.x * bhDir.x + bhDir.y * bhDir.y + bhDir.z * bhDir.z + softeningSq;
        float invDist = rsqrtf(distSq);
        float invDist3 = invDist * invDist * invDist;
        float bhForce = G * blackHoleMass * invDist3;

        acc.x += bhDir.x * bhForce;
        acc.y += bhDir.y * bhForce;
        acc.z += bhDir.z * bhForce;

        accelerations[i] = acc;
    }
}

// ============================================================================
// Kernel 2: Velocity Verlet Integration - First half step
// ============================================================================
__global__ void verletFirstHalfKernel(
    float3 *__restrict__ positions,
    float3 *__restrict__ velocities,
    const float3 *__restrict__ accelerations,
    int numBodies,
    float dt)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= numBodies)
        return;

    float3 pos = positions[i];
    float3 vel = velocities[i];
    float3 acc = accelerations[i];

    // r(t+dt) = r(t) + v(t)*dt + 0.5*a(t)*dt²
    float halfDtSq = 0.5f * dt * dt;
    pos.x += vel.x * dt + acc.x * halfDtSq;
    pos.y += vel.y * dt + acc.y * halfDtSq;
    pos.z += vel.z * dt + acc.z * halfDtSq;

    // v(t+0.5dt) = v(t) + 0.5*a(t)*dt
    float halfDt = 0.5f * dt;
    vel.x += acc.x * halfDt;
    vel.y += acc.y * halfDt;
    vel.z += acc.z * halfDt;

    positions[i] = pos;
    velocities[i] = vel;
}

// ============================================================================
// Kernel 3: Velocity Verlet Integration - Second half step
// ============================================================================
__global__ void verletSecondHalfKernel(
    float3 *__restrict__ velocities,
    const float3 *__restrict__ accelerations,
    int numBodies,
    float dt)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= numBodies)
        return;

    float3 vel = velocities[i];
    float3 acc = accelerations[i];

    // v(t+dt) = v(t+0.5dt) + 0.5*a(t+dt)*dt
    float halfDt = 0.5f * dt;
    vel.x += acc.x * halfDt;
    vel.y += acc.y * halfDt;
    vel.z += acc.z * halfDt;

    velocities[i] = vel;
}

// ============================================================================
// PhysicsCUDA Implementation
// ============================================================================

PhysicsCUDA::PhysicsCUDA()
{
    // Create CUDA events for timing
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    m_startEvent = start;
    m_stopEvent = stop;
}

PhysicsCUDA::~PhysicsCUDA()
{
    if (d_positions)
        cudaFree(d_positions);
    if (d_velocities)
        cudaFree(d_velocities);
    if (d_accelerations)
        cudaFree(d_accelerations);
    if (d_masses)
        cudaFree(d_masses);

    if (m_startEvent)
        cudaEventDestroy((cudaEvent_t)m_startEvent);
    if (m_stopEvent)
        cudaEventDestroy((cudaEvent_t)m_stopEvent);
}

void PhysicsCUDA::init(int numBodies)
{
    m_numBodies = numBodies;

    // Allocate device memory
    CUDA_CHECK(cudaMalloc(&d_positions, numBodies * sizeof(float3)));
    CUDA_CHECK(cudaMalloc(&d_velocities, numBodies * sizeof(float3)));
    CUDA_CHECK(cudaMalloc(&d_accelerations, numBodies * sizeof(float3)));
    CUDA_CHECK(cudaMalloc(&d_masses, numBodies * sizeof(float)));

    // Initialize accelerations to zero
    CUDA_CHECK(cudaMemset(d_accelerations, 0, numBodies * sizeof(float3)));

    m_initialized = true;
    printf("[CUDA] Initialized with %d bodies\n", numBodies);
    printf("[CUDA] Memory allocated: %.2f MB\n",
           (numBodies * (sizeof(float3) * 3 + sizeof(float))) / (1024.0f * 1024.0f));
}

void PhysicsCUDA::uploadBodies(const float *positions, const float *velocities,
                               const float *masses, int count)
{
    if (!m_initialized || count != m_numBodies)
    {
        if (m_initialized)
        {
            // Cleanup old memory
            cudaFree(d_positions);
            cudaFree(d_velocities);
            cudaFree(d_accelerations);
            cudaFree(d_masses);
        }
        init(count);
    }

    CUDA_CHECK(cudaMemcpy(d_positions, positions, count * sizeof(float3), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_velocities, velocities, count * sizeof(float3), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_masses, masses, count * sizeof(float), cudaMemcpyHostToDevice));
}

void PhysicsCUDA::step(float dt, float G, float blackHoleMass, float softening)
{
    if (!m_initialized)
        return;

    int blocks = (m_numBodies + BLOCK_SIZE - 1) / BLOCK_SIZE;

    // Start timing
    cudaEventRecord((cudaEvent_t)m_startEvent);

    // 1. First half of Verlet integration (position update + half velocity)
    verletFirstHalfKernel<<<blocks, BLOCK_SIZE>>>(
        d_positions, d_velocities, d_accelerations, m_numBodies, dt);

    // 2. Compute new accelerations (force calculation)
    computeForcesKernel<<<blocks, BLOCK_SIZE>>>(
        d_positions, d_masses, d_accelerations, m_numBodies,
        G, blackHoleMass, softening);

    // 3. Second half of Verlet integration (complete velocity update)
    verletSecondHalfKernel<<<blocks, BLOCK_SIZE>>>(
        d_velocities, d_accelerations, m_numBodies, dt);

    // Stop timing
    cudaEventRecord((cudaEvent_t)m_stopEvent);
    cudaEventSynchronize((cudaEvent_t)m_stopEvent);

    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, (cudaEvent_t)m_startEvent, (cudaEvent_t)m_stopEvent);
    m_lastKernelTimeMs = milliseconds;
}

void PhysicsCUDA::downloadPositions(float *positions)
{
    CUDA_CHECK(cudaMemcpy(positions, d_positions,
                          m_numBodies * sizeof(float3), cudaMemcpyDeviceToHost));
}

void PhysicsCUDA::downloadVelocities(float *velocities)
{
    CUDA_CHECK(cudaMemcpy(velocities, d_velocities,
                          m_numBodies * sizeof(float3), cudaMemcpyDeviceToHost));
}

// ============================================================================
// Utility Functions
// ============================================================================

bool isCUDAAvailable()
{
    int deviceCount = 0;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);
    return (err == cudaSuccess && deviceCount > 0);
}

int getCUDADeviceCount()
{
    int deviceCount = 0;
    cudaGetDeviceCount(&deviceCount);
    return deviceCount;
}

void printCUDADeviceInfo()
{
    int deviceCount = 0;
    cudaGetDeviceCount(&deviceCount);

    printf("\n========== CUDA Device Info ==========\n");
    printf("Found %d CUDA device(s)\n\n", deviceCount);

    for (int i = 0; i < deviceCount; i++)
    {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, i);

        printf("Device %d: %s\n", i, prop.name);
        printf("  Compute Capability: %d.%d\n", prop.major, prop.minor);
        printf("  Total Global Memory: %.2f GB\n", prop.totalGlobalMem / (1024.0f * 1024.0f * 1024.0f));
        printf("  Shared Memory per Block: %.2f KB\n", prop.sharedMemPerBlock / 1024.0f);
        printf("  Max Threads per Block: %d\n", prop.maxThreadsPerBlock);
        printf("  Multiprocessors: %d\n", prop.multiProcessorCount);
        printf("  Warp Size: %d\n", prop.warpSize);
        printf("  Memory Clock Rate: %.2f GHz\n", prop.memoryClockRate / 1e6f);
        printf("  Memory Bus Width: %d-bit\n", prop.memoryBusWidth);
        printf("\n");
    }
    printf("=======================================\n\n");
}

#endif // USE_CUDA
