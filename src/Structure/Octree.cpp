#include "Octree.hpp"
#include "../Core/Particle.hpp"
Octree::Octree(const AABB &rootBounds, size_t maxCapacity)
    : initialBounds(rootBounds), nodePool(maxCapacity), root(nullptr) {}

void Octree::build(std::vector<Core::Particle> &particles)
{
    nodePool.reset();
    root = nodePool.allocate(initialBounds);
    for (auto &p : particles)
    {
        insert(root, &p);
    }
    computeMassDistribution(root);
}

void Octree::insert(OctreeNode *node, Core::Particle *p, int depth)
{
    if (depth > MAX_DEPTH)
        return;

    if (!node->boundary.contains(p->position))
        return;

    if (node->isLeaf)
    {
        if (node->particle == nullptr)
        {
            node->particle = p;
        }
        else
        {
            Core::Particle *oldParticle = node->particle;
            node->particle = nullptr;
            node->isLeaf = false;

            for (int i = 0; i < 8; ++i)
                node->children[i] = nodePool.allocate(node->boundary.getOctant(i));

            insert(node, oldParticle, depth + 1);
            insert(node, p, depth + 1);
        }
    }
    else
    {
        glm::vec3 center = node->boundary.center;
        int octantIndex = 0;
        if (p->position.x > center.x)
            octantIndex |= 1;
        if (p->position.y > center.y)
            octantIndex |= 2;
        if (p->position.z > center.z)
            octantIndex |= 4;
        insert(node->children[octantIndex], p, depth + 1);
    }
}

void Octree::computeMassDistribution(OctreeNode *node)
{
    if (node->isLeaf)
    {
        if (node->particle)
        {
            node->centerOfMass = node->particle->position;
            node->totalMass = node->particle->mass;
        }
        else
        {
            node->centerOfMass = glm::vec3(0.0f);
            node->totalMass = 0.0f;
        }
    }
    else
    {
        node->centerOfMass = glm::vec3(0.0f);
        node->totalMass = 0.0f;

        for (int i = 0; i < 8; ++i)
        {
            if (node->children[i])
            {
                computeMassDistribution(node->children[i]);
                node->totalMass += node->children[i]->totalMass;
                node->centerOfMass += node->children[i]->centerOfMass * node->children[i]->totalMass;
            }
        }

        if (node->totalMass > 0.0f)
            node->centerOfMass /= node->totalMass;
    }
}
