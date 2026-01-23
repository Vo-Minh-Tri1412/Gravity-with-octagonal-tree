#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <array>

// 1. Data Structures: Linear Octree (The Core)
// Priority: Cache Coherence (Flat Layout)

struct OctreeNode
{
    glm::vec3 centerOfMass{0.0f};
    float totalMass{0.0f};

    glm::vec3 minBound;
    glm::vec3 maxBound;

    // Index into std::vector<OctreeNode>
    // -1 indicates Leaf node
    int32_t childrenStart{-1};

    // Number of particles if leaf (optional optimization)
    int32_t particleCount{0};
};

class Octree
{
public:
    Octree();

    // Rebuilds the tree from scratch every frame
    void build(const std::vector<glm::vec3> &positions, const std::vector<float> &masses);

    // Calculate force on a single particle
    glm::vec3 calculateForce(const glm::vec3 &pos, float theta = 0.5f) const;

    const std::vector<OctreeNode> &getNodes() const { return m_nodes; }

private:
    std::vector<OctreeNode> m_nodes;

    // Helper to insert recursively (but storing linearly)
    void insert(int32_t nodeIdx, const glm::vec3 &pos, float mass);
};