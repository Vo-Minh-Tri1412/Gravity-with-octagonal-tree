#pragma once
#include <glm/glm.hpp>

// Physics simulation system

struct Body
{
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;

    // TODO: Add more body properties
};

class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    // TODO: Implement physics methods
};
