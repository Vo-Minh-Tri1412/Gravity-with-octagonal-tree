#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Octree.hpp"

struct Body
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    // Physics properties
    float mass;
    bool isStatic;

    // Aesthetic properties (New)
    glm::vec4 originalColor; // Color based on star type (Red Giant, Blue Dwarf, etc.)
    float size;              // Visual size multiplier
};

class PhysicsSystem
{
public:
    void setBodies(const std::vector<Body> &bodies);
    void step(float dt);
    const std::vector<Body> &getBodies() const { return m_bodies; }

    // Getters for GPU physics
    float getG() const { return G; }
    float getBlackHoleMass() const { return m_blackHoleMass; }

private:
    std::vector<Body> m_bodies;
    Octree m_octree;
    float G = 1.0f;
    float m_blackHoleMass = 10000.0f;
};