#pragma once
#include "ISolver.hpp"
#include <vector>
#include <glm/glm.hpp>
#include "../Core/Particle.hpp"

namespace Physics
{
    /**
     * @brief Brute Force Solver - Tính toán lực hấp dẫn O(N²)
     *
     * Tính toán lực giữa mọi cặp particles.
     * Phù hợp cho số lượng particles nhỏ (< 1000).
     */
    class BruteForceSolver : public ISolver
    {
    public:
        /**
         * @brief Tính toán và cập nhật vị trí/vận tốc của tất cả particles
         * @param particles Vector chứa tất cả particles (sẽ bị thay đổi)
         * @param dt Delta time - thời gian giữa 2 frame (giây)
         */
        void solve(std::vector<Core::Particle> &particles, float dt) override;
    };
}
