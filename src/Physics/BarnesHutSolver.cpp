#include "BarnesHutSolver.hpp"
#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>
namespace Physics
{void BarnesHutSolver::solve(std::vector<Core::Particle> &particles, float dt)
    {
        if (particles.empty())
            return;
//cập nhật code lần 2
        // B1: Cập nhật nửa bước vận tốc & toàn bước vị trí 
        for (auto &p : particles)
        {p.velocity += 0.5f * p.acceleration * dt;
            p.position += p.velocity * dt;
            p.acceleration = glm::vec3(0.0f); //reset gia tốc
        }
        // B2:dựng Octree tại vị trí mới
        glm::vec3 minPos = particles[0].position;
        glm::vec3 maxPos = particles[0].position;
        for (const auto &p : particles)
        { minPos = glm::min(minPos, p.position);
            maxPos = glm::max(maxPos, p.position);
        }
        glm::vec3 center = (minPos + maxPos) * 0.5f;
        glm::vec3 size = (maxPos - minPos) * 0.5f;
        float maxDim = std::max({size.x, size.y, size.z});
        glm::vec3 halfSize(maxDim + 1.0f);
        AABB rootBounds(center, halfSize);
        Octree tree(rootBounds, particles.size() + 2000);
        tree.build(particles); 
        
        OctreeNode *root = tree.getRoot();

        for (auto &p : particles)
        {calculateForce(p, root);}

        // B3:nửa bước vận tốc còn lại
        for (auto &p : particles)
        { p.velocity += 0.5f * p.acceleration * dt;}}

    void BarnesHutSolver::calculateForce(Core::Particle &p, OctreeNode *node)
    {
        if (node == nullptr || node->totalMass <= 0.0f)
        {return;}

        glm::vec3 rVec = node->centerOfMass - p.position;
        float distSq = glm::dot(rVec, rVec);
        float dist = glm::sqrt(distSq);

        if (node->isLeaf)
        {if (node->particle != &p)
            {float forceMag = G * p.mass * node->particle->mass / (distSq + EPSILON * EPSILON);
                glm::vec3 force = forceMag * (rVec / dist);
                p.acceleration += force / p.mass;
            }}
        else
        {
            float s = node->boundary.halfSize.x * 2.0f;
            if (dist > 0.0f && (s / dist < THETA))
            {float forceMag = G * p.mass * node->totalMass / (distSq + EPSILON * EPSILON);//epsilon đã gthich ở brutrforcesolver.cpp
                glm::vec3 force = forceMag * (rVec / dist);
                p.acceleration += force / p.mass;
            }
            else
            {for (int i = 0; i < 8; ++i)
                {if (node->children[i])
                    {calculateForce(p, node->children[i]);}}}}}}
