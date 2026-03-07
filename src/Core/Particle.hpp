#pragma once
#include <glm/glm.hpp>

namespace Core
{
    // Loại hạt — renderer dùng để quyết định cách vẽ
    enum class ParticleType : uint8_t
    {
        GENERIC = 0, // hạt thiên hà  → DrawPoint3D, màu theo khối lượng
        STAR = 1,    // ngôi sao / mặt trời → sphere vàng sáng
        PLANET = 2,  // hành tinh          → sphere màu theo khối lượng
        MOON = 3,    // mặt trăng           → sphere xám nhỏ
        ASTEROID = 4 // tiểu hành tinh      → DrawPoint3D xám
    };

    struct Particle
    {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        float mass;
        float radius = 0.0f; // 0 = auto from mass; >0 = explicit visual radius
        ParticleType type;

        Particle(glm::vec3 pos = glm::vec3(0.0f), float m = 1.0f)
            : position(pos), velocity(0.0f), acceleration(0.0f),
              mass(m), radius(0.0f), type(ParticleType::GENERIC) {}
    };
}