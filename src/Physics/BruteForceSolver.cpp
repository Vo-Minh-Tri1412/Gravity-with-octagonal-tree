#include "BruteForceSolver.hpp"
#include <glm/glm.hpp>

namespace Physics
{
    // Hằng số hấp dẫn có điều chỉnh để phù hợp với đơn vị sử dụng trong mô phỏng
    constexpr float G = 6.67430e-2f;

    // hằng số này để thêm vào mô phỏng khoảng cách nhằm tránh chia cho 0
    constexpr float EPSILON = 1e-3f;
//cập nhật:thay đổi thuật toán tích phân từ Semi-implicit Euler sang Velocity Verlet:
    void BruteForceSolver::solve(std::vector<Core::Particle> &particles, float dt)
    {
        const size_t n = particles.size();

       //B1: Cập nhật nửa bước vận tốc và toàn bước vị trí
       for (size_t i = 0; i < n; ++i)
        {
            particles[i].velocity += 0.5f * particles[i].acceleration * dt;
            particles[i].position += particles[i].velocity * dt;
            particles[i].acceleration = glm::vec3(0.0f); 
        }
        // B2:Tính lực hấp dẫn mới tại vị trí mới
   for (size_t i = 0; i < n; ++i)
        {for (size_t j = i + 1; j < n; ++j)
            {glm::vec3 r = particles[j].position - particles[i].position;
               // cộng thêm epsion bình phương,nếu không cộng , lỡ r = 0 thì chia cho 0 lỗi
                float distSq = glm::dot(r, r) + EPSILON * EPSILON;
                float dist = glm::sqrt(distSq);

           // lực hấp dẫn:F = G * m1 * m2 / r^2:
                float forceMag = G * particles[i].mass * particles[j].mass / distSq;
                glm::vec3 force = forceMag * (r / dist);//Vector lực= X vô hướng *vector chỉ phương
                particles[i].acceleration += force / particles[i].mass;
                particles[j].acceleration -= force / particles[j].mass;}}
        // B3: đôi mới:dùng gia tốc ở bước 2 cộng bù nửa bước vận tốc cuối cùng.
      for (size_t i = 0; i < n; ++i)
        { particles[i].velocity += 0.5f * particles[i].acceleration * dt;}}}

