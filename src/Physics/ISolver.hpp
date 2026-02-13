#pragma once
#include <vector>
#include "../Core/Particle.hpp"

namespace Physics
{
    /**
     * @brief Interface chung cho các thuật toán giải quyết bài toán N-Body.
     */
    class ISolver
    {
    public:
        virtual ~ISolver() = default;

        /**
         * @brief Tính toán lực và cập nhật vị trí cho các hạt.
         * @param particles Danh sách các hạt cần mô phỏng.
         * @param dt Thời gian giữa các khung hình (Delta time).
         */
        virtual void solve(std::vector<Core::Particle> &particles, float dt) = 0;
    };
}
