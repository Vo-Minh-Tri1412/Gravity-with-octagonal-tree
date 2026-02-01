#include "BarnesHutSolver.hpp"
#include <glm/glm.hpp>

namespace Physics
{
    void BarnesHutSolver::solve(std::vector<Core::Particle> &particles, float dt)
    {
        // TODO: Implement Barnes-Hut solver (O(N log N))
        // 1. Build octree from particles
        // 2. For each particle, traverse tree and calculate force
        // 3. Update velocity and position
        (void)particles; // Suppress unused parameter warning
        (void)dt;
    }
}
