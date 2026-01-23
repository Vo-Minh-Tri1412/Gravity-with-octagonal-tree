#include "Octree.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

// Implementation of Barnes-Hut Linear Octree

Octree::Octree()
{
    // Reserve memory to avoid reallocations during frame
    m_nodes.reserve(200000);
}

void Octree::build(const std::vector<glm::vec3> &positions, const std::vector<float> &masses)
{
    m_nodes.clear();
    if (positions.empty())
        return;

    // 1. Compute bounding box
    glm::vec3 minB(std::numeric_limits<float>::max());
    glm::vec3 maxB(std::numeric_limits<float>::lowest());

    for (const auto &pos : positions)
    {
        minB = glm::min(minB, pos);
        maxB = glm::max(maxB, pos);
    }

    // Make it a cube (essential for octree)
    glm::vec3 center = (minB + maxB) * 0.5f;
    glm::vec3 size = maxB - minB;
    float maxDim = std::max({size.x, size.y, size.z});
    glm::vec3 halfSize = glm::vec3(maxDim * 0.5f + 0.1f); // padding

    // 2. Create Root Node
    m_nodes.emplace_back();
    m_nodes[0].minBound = center - halfSize;
    m_nodes[0].maxBound = center + halfSize;
    m_nodes[0].childrenStart = -1;
    m_nodes[0].particleCount = 0;
    m_nodes[0].totalMass = 0.0f;
    m_nodes[0].centerOfMass = glm::vec3(0.0f);

    // 3. Insert Particles
    // Note: This is sequential build. Parallel build is harder.
    for (size_t i = 0; i < positions.size(); ++i)
    {
        insert(0, positions[i], masses[i]);
    }
}

void Octree::insert(int32_t nodeIdx, const glm::vec3 &pos, float mass)
{
    // Reference stability warning: m_nodes might realloc if we push_back.
    // However, we use indices, which are stable UNLESS we realloc and move memory.
    // But since we are recursive, reference 'node' might become invalid if m_nodes reallocs inside recursion.
    // ALWAYS use index to access node.

    // 1. Update COM and Mass for the current node
    // This happens for both internal and leaf nodes on the path
    OctreeNode *node = &m_nodes[nodeIdx];

    float newTotalMass = node->totalMass + mass;
    if (newTotalMass > 0.0f)
    {
        node->centerOfMass = (node->centerOfMass * node->totalMass + pos * mass) / newTotalMass;
    }
    else
    {
        node->centerOfMass = pos;
    }
    node->totalMass = newTotalMass;
    node->particleCount++;

    // Case A: Internal Node (already subdivided)
    if (node->childrenStart != -1)
    {
        glm::vec3 center = (node->minBound + node->maxBound) * 0.5f;
        int octant = 0;
        if (pos.x > center.x)
            octant |= 1;
        if (pos.y > center.y)
            octant |= 2;
        if (pos.z > center.z)
            octant |= 4;

        insert(node->childrenStart + octant, pos, mass);
        return;
    }

    // Case B: Leaf Node, Empty
    // Just updated mass/com above, so we are essentially done.
    // But we need to ensure we track that this leaf holds a particle.
    if (node->particleCount == 1)
    {
        // First particle in this leaf.
        // We use centerOfMass as the particle position effectively.
        return;
    }

    // Case C: Leaf Node, Occupied (count > 1) -> Subdivide!
    // We must:
    // 1. Create 8 children.
    // 2. Move the *existing* particle (which is at node->centerOfMass roughly? No.)
    //    Problem: We didn't store the exact position of the first particle separately.
    //    Correction for this specific simplified struct:
    //    If particleCount was 1, centerOfMass WAS the exact position of the single particle.
    //    We need to extract that "old" particle and move it to a child.

    // Backtrack math to get old particle pos:
    // Current node->centerOfMass includes 'pos'.
    // oldTotalMass = node->totalMass - mass;
    // oldPos = (node->centerOfMass * node->totalMass - pos * mass) / oldTotalMass;

    glm::vec3 oldPos = node->centerOfMass; // Default fallback
    if (node->totalMass - mass > 0.0001f)
    {
        oldPos = (node->centerOfMass * node->totalMass - pos * mass) / (node->totalMass - mass);
    }
    float oldMass = node->totalMass - mass;

    // Allocate children
    int32_t firstChildIdx = (int32_t)m_nodes.size();
    m_nodes.resize(m_nodes.size() + 8);

    // Re-fetch pointer because resize invalidated it
    node = &m_nodes[nodeIdx];
    node->childrenStart = firstChildIdx;

    glm::vec3 center = (node->minBound + node->maxBound) * 0.5f;
    glm::vec3 half = (node->maxBound - node->minBound) * 0.5f;

    // Init children bounds
    for (int i = 0; i < 8; ++i)
    {
        OctreeNode &child = m_nodes[firstChildIdx + i];

        glm::vec3 childMin = node->minBound;
        if (i & 1)
            childMin.x = center.x;
        if (i & 2)
            childMin.y = center.y;
        if (i & 4)
            childMin.z = center.z;

        child.minBound = childMin;
        child.maxBound = childMin + half;
        child.childrenStart = -1;
        child.particleCount = 0;
        child.totalMass = 0.0f;
    }

    // Re-insert OLD particle into appropriate child
    int octantOld = 0;
    if (oldPos.x > center.x)
        octantOld |= 1;
    if (oldPos.y > center.y)
        octantOld |= 2;
    if (oldPos.z > center.z)
        octantOld |= 4;
    insert(firstChildIdx + octantOld, oldPos, oldMass);

    // Insert NEW particle into appropriate child
    int octantNew = 0;
    if (pos.x > center.x)
        octantNew |= 1;
    if (pos.y > center.y)
        octantNew |= 2;
    if (pos.z > center.z)
        octantNew |= 4;
    insert(firstChildIdx + octantNew, pos, mass);
}

glm::vec3 Octree::calculateForce(const glm::vec3 &pos, float theta) const
{
    glm::vec3 force(0.0f);
    if (m_nodes.empty())
        return force;

    // Stack for traversal (simulating recursion)
    // Stack stores node indices
    // Max depth of octree usually < 20, so stack size 64 is safe
    int32_t stack[128];
    int stackPtr = 0;
    stack[stackPtr++] = 0; // Push root

    while (stackPtr > 0)
    {
        int32_t idx = stack[--stackPtr];
        const OctreeNode &node = m_nodes[idx];

        if (node.particleCount == 0)
            continue;

        glm::vec3 dir = node.centerOfMass - pos;
        float distSq = glm::dot(dir, dir);
        float dist = std::sqrt(distSq);

        // Self-interaction check (approximate using small epsilon)
        if (dist < 0.001f)
            continue;

        float size = (node.maxBound.x - node.minBound.x);

        // Barnes-Hut Criterion: s / d < theta
        bool isFarEnough = (size / dist) < theta;
        bool isLeaf = (node.childrenStart == -1);

        if (isFarEnough || isLeaf)
        {
            // Compute force
            float softening = 0.5f; // prevent infinity
            float strength = (1.0f * node.totalMass) / std::pow(distSq + softening * softening, 1.5f);
            force += dir * strength;
        }
        else
        {
            // Not far enough, open node (push children)
            int32_t firstChild = node.childrenStart;
            for (int i = 0; i < 8; ++i)
            {
                stack[stackPtr++] = firstChild + i;
            }
        }
    }

    return force;
}