#include "BarnesHutSolver.hpp"
#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>

namespace Physics
{
    void BarnesHutSolver::solve(std::vector<Core::Particle> &particles, float dt)
    {
        if (particles.empty())
            return;
        // ============================================================
        // BƯỚC 1: XÁC ĐỊNH KHÔNG GIAN (BOUNDING BOX)
        // ============================================================
        // Để xây dựng Octree, ta cần biết giới hạn không gian bao trùm tất cả các hạt.
        glm::vec3 minPos = particles[0].position;
        glm::vec3 maxPos = particles[0].position;

        for (const auto &p : particles)
        {
            minPos = glm::min(minPos, p.position);
            maxPos = glm::max(maxPos, p.position);
        }

        // Tính tâm và kích thước hộp
        glm::vec3 center = (minPos + maxPos) * 0.5f;
        glm::vec3 size = (maxPos - minPos) * 0.5f;

        // Tìm cạnh lớn nhất để tạo hình lập phương (Octree hoạt động tốt nhất với hình lập phương)
        float maxDim = std::max({size.x, size.y, size.z});

        // Thêm một chút lề (padding) để đảm bảo hạt không nằm ngay sát biên gây lỗi
        glm::vec3 halfSize(maxDim + 1.0f);

        AABB rootBounds(center, halfSize);

        // Tạo cây mới mỗi frame.
        // Ước lượng số lượng node cần thiết (thường nhiều hơn số hạt một chút)
        Octree tree(rootBounds, particles.size() + 2000);
        tree.build(particles); // Hàm này đã bao gồm việc tính Mass Distribution
        // Tính lực
        OctreeNode *root = tree.getRoot();

        for (auto &p : particles)
        {
            // Reset gia tốc về 0 trước khi tính khung hình mới
            p.acceleration = glm::vec3(0.0f);

            // Gọi hàm đệ quy để tính tổng lực từ cây tác dụng lên hạt p
            calculateForce(p, root);
        }
        // Sử dụng phương pháp Semi-implicit Euler (như trong BruteForceSolver)
        for (auto &p : particles)
        {
            p.velocity += p.acceleration * dt;
            p.position += p.velocity * dt;
        }
    }

    void BarnesHutSolver::calculateForce(Core::Particle &p, OctreeNode *node)
    {
        // 1. Kiểm tra node rỗng hoặc không có khối lượng
        if (node == nullptr || node->totalMass <= 0.0f)
        {
            return;
        }

        // Tính vector từ hạt p đến khối tâm của node (r = r_node - r_p)
        glm::vec3 rVec = node->centerOfMass - p.position;
        float distSq = glm::dot(rVec, rVec);
        float dist = glm::sqrt(distSq);

        // 2. Trường hợp Node là LÁ (Leaf Node)
        if (node->isLeaf)
        {
            // Nếu node chứa chính hạt p thì bỏ qua (hạt không tự hút chính nó)
            if (node->particle != &p)
            {
                // Tính lực trực tiếp: F = G * m1 * m2 / r^2
                float forceMag = G * p.mass * node->particle->mass / (distSq + EPSILON * EPSILON);
                glm::vec3 force = forceMag * (rVec / dist);
                p.acceleration += force / p.mass;
            }
        }
        // 3. Trường hợp Node là NHÁNH (Internal Node)
        else
        {
            // Kích thước của vùng không gian node (chiều rộng cạnh = bán kính * 2)
            float s = node->boundary.halfSize.x * 2.0f;

            // Tiêu chuẩn Barnes-Hut: s / d < theta
            // Nếu kích thước node nhỏ hơn nhiều so với khoảng cách tới nó -> Coi như 1 điểm
            if (dist > 0.0f && (s / dist < THETA))
            {
                // XẤP XỈ: Tính lực hấp dẫn với khối tâm của toàn bộ node này
                float forceMag = G * p.mass * node->totalMass / (distSq + EPSILON * EPSILON);
                glm::vec3 force = forceMag * (rVec / dist);
                p.acceleration += force / p.mass;
            }
            else
            {
                // KHÔNG XẤP XỈ ĐƯỢC: Node quá gần hoặc quá to
                // Phải đi sâu xuống các node con để tính chi tiết hơn
                for (int i = 0; i < 8; ++i)
                {
                    if (node->children[i])
                    {
                        calculateForce(p, node->children[i]);
                    }
                }
            }
        }
    }
}
