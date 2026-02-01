#include "BruteForceSolver.hpp"
#include <glm/glm.hpp>

namespace Physics
{
    // Hằng số hấp dẫn có điều chỉnh để phù hợp với đơn vị sử dụng trong mô phỏng
    constexpr float G = 6.67430e-2f;

    // hằng số này để thêm vào mô phỏng khoảng cách nhằm tránh chia cho 0
    constexpr float EPSILON = 1e-3f;

    void BruteForceSolver::solve(std::vector<Core::Particle> &particles, float dt)
    {
        const size_t n = particles.size();

        // Bước 1: Reset gia tốc
        for (size_t i = 0; i < n; ++i)
        {
            particles[i].acceleration = glm::vec3(0.0f);
        }

        // Bước 2: Tính lực giữa mọi cặp (O(N²))
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = i + 1; j < n; ++j)
            {
                // Vector từ i đến j
                glm::vec3 r = particles[j].position - particles[i].position;

                // Khoảng cách (với softening để tránh chia 0)
                float distSq = glm::dot(r, r) + EPSILON * EPSILON;
                float dist = glm::sqrt(distSq);

                // Tính độ lớn lực: F = G * m1 * m2 / r^2 (định luật vạn vật hấp dẫn)
                float forceMag = G * particles[i].mass * particles[j].mass / distSq;

                // Vector lực ( đại lượng vô hướng force/dist nhân với một vector sẽ cho ra vector lực )
                glm::vec3 force = forceMag * (r / dist);

                // Định luật 3 Newton: Lực tác dụng lên của i lên j là bằng và ngược chiều lực tác dụng lên j lên i (vectorF_ij = -vectorF_ji)
                // Định luật 2 Newton: a = F/m
                particles[i].acceleration += force / particles[i].mass;
                particles[j].acceleration -= force / particles[j].mass;
            }
        }

        // Bước 3: Tích phân vận tốc và vị trí (ở đây chúng ta không thể mô tả chính xác vì vốn dĩ khái niệm tích phân trong vật lý là liên tục, thuật toán này có thể sẽ được update sau nếu như chúng ta cần độ chính xác cao hơn)
        for (size_t i = 0; i < n; ++i)
        {
            particles[i].velocity += particles[i].acceleration * dt;
            particles[i].position += particles[i].velocity * dt;
        }
    }
}
