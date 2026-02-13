#pragma once
#include "ISolver.hpp"
#include <vector>
#include <glm/glm.hpp>
#include "../Core/Particle.hpp"
#include "../Structure/Octree.hpp"

namespace Physics
{
    /**
     * @brief Barnes-Hut Solver - Tính toán lực hấp dẫn O(N log N)
     *
     * Sử dụng Octree để tối ưu tính toán lực.
     * Phù hợp cho số lượng particles lớn (> 1000).
     */
    class BarnesHutSolver : public ISolver
    {
    public:
        /**
         * @brief Tính toán và cập nhật vị trí/vận tốc của tất cả particles
         * @param particles Vector chứa tất cả particles (sẽ bị thay đổi)
         * @param dt Delta time - thời gian giữa 2 frame (giây)
         */
        void solve(std::vector<Core::Particle> &particles, float dt) override;

    private:
        // Hằng số hấp dẫn (giống BruteForceSolver)
        const float G = 6.67430e-2f;

        // Hằng số làm mềm để tránh chia cho 0 khi khoảng cách quá gần
        const float EPSILON = 1e-3f;

        // Ngưỡng Theta (0.5 là giá trị tiêu chuẩn)
        // Dùng để quyết định xem một node có đủ xa để coi là một điểm hay không.
        // Điều kiện: (Kích thước node / Khoảng cách) < THETA
        const float THETA = 0.5f;

        /**
         * @brief Hàm đệ quy tính lực tác dụng lên một hạt từ một node của cây Octree
         *
         * @param p Hạt đang cần tính lực (tham chiếu để cộng dồn gia tốc)
         * @param node Node hiện tại đang xét trong cây
         */
        void calculateForce(Core::Particle &p, OctreeNode *node);
    };
}
