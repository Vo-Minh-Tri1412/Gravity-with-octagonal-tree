#pragma once
#include <glm/glm.hpp>
struct alignas(16) Particle {
    glm::vec3 pos;
    float mass;    // Khối tọa độ
    
    glm::vec3 vel;
    float padding1; // Khối vận tốc
    
    glm::vec3 acc;
    float padding2; // Khối gia tốc
};
// thêm vào các padding để tối ưu tốc độ đọc CPU 
static_assert(sizeof(Particle)) == 48; // Kiểm tra kích thước bộ nhớ của Particle đồng thời ràng buộc bộ nhớ để tối ưu hiệu năng

static_assert(alignof(Particle)) == 16; // Tương tự