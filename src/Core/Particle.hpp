#pragma once

#include <glm/glm.hpp>

namespace Core
{struct Particle //cái này chi chua du lieu, ko logic 
    {
        glm::vec3 position;     //vi tri 
        glm::vec3 velocity;     // chi van toc 
        glm::vec3 acceleration; // chi gia toc 
        float mass;             

        Particle(glm::vec3 pos = glm::vec3(0.0f), float m = 1.0f)
            :position(pos), velocity(0.0f), acceleration(0.0f), mass(m) {}};
}