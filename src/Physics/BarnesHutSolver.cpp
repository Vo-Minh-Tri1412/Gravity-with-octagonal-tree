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

        // Bounding box cố định cho toàn bộ thiên hà
        AABB rootBounds(glm::vec3(0.0f), glm::vec3(500.0f));

        // Tái dùng cây mọi frame (reset O(1)).
        // Tạo lại nếu số hạt thay đổi — tránh pool overflow khi chuyển kịch bản.
        if (!m_tree || particles.size() != m_lastParticleCount)
        {
            m_lastParticleCount = particles.size();
            size_t capacity = particles.size() * 9 + 1000;
            m_tree = std::make_unique<Octree>(rootBounds, capacity);
        }

        m_tree->build(particles); // reset() O(1) + rebuild

        OctreeNode *root = m_tree->getRoot();

        for (auto &p : particles)
        {
            p.acceleration = glm::vec3(0.0f);
            calculateForce(p, root);
        }

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
            return;

        // Tính vector từ hạt p đến khối tâm của node
        glm::vec3 rVec = node->centerOfMass - p.position;
        float distSq = glm::dot(rVec, rVec);

        // 2. Node LÁ
        if (node->isLeaf)
        {
            if (node->particle != &p && distSq > 0.0f)
            {
                // sqrt chỉ gọi khi thực sự cần (leaf có hạt khác)
                float invDist = 1.0f / glm::sqrt(distSq);
                // a += G * M_other / (r² + ε²) * r̂
                float accelMag = G * node->particle->mass / (distSq + EPSILON * EPSILON);
                p.acceleration += accelMag * invDist * rVec;
            }
        }
        // 3. Node NHÁNH
        else
        {
            float s = node->boundary.halfSize.x * 2.0f;

            // Barnes-Hut: s/d < θ  ↔  s² < θ² * d²  (không cần sqrt)
            if (distSq > 0.0f && (s * s < THETA * THETA * distSq))
            {
                // Xấp xỉ: coi cả nhóm là một điểm
                // sqrt chỉ gọi khi đạt tiêu chuẩn xấp xỉ
                float invDist = 1.0f / glm::sqrt(distSq);
                float accelMag = G * node->totalMass / (distSq + EPSILON * EPSILON);
                p.acceleration += accelMag * invDist * rVec;
            }
            else
            {
                // Node quá gần hoặc quá to: đi sâu xuống con
                // KHÔNG gọi sqrt ở đây — tiết kiệm cho phần lớn node nội
                for (int i = 0; i < 8; ++i)
                {
                    if (node->children[i])
                        calculateForce(p, node->children[i]);
                }
            }
        }
    }
}
