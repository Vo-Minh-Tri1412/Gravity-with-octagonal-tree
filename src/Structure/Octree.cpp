#include "Octree.hpp"
#include "../Core/Particle.hpp"
Octree::Octree(const AABB &rootBounds, size_t maxCapacity)
    : initialBounds(rootBounds), nodePool(maxCapacity), root(nullptr) {}

// hàm nay xây dựng lại cây mỗi khung hình, sẽ gọi liên tục trong vòng lặp
void Octree::build(std::vector<Core::Particle> &particles)
{
    nodePool.reset(); // thay vì delete từng node mnhf chỉ cần reset con trỏ pool về 0=>Big O: O(1)

    root = nodePool.allocate(initialBounds); // node gốc này bao trùm toàn bộ không gian .

    for (auto &p : particles)
    {
        insert(root, &p);
    }
    computeMassDistribution(root);
}

void Octree::insert(OctreeNode *node, Core::Particle *p)
{
    if (!node->boundary.contains(p->position))
    {
        return;
    } // check biên
    // hạt bay ra ngoài vùng quản lý của node này thf bỏ qua
    if (node->isLeaf)
    { // nếu đây là node lá
        if (node->particle == nullptr)
        { // node đang trống:đặt hạt và kts thúc.
            node->particle = p;
        }
        else
        {
            Core::Particle *oldParticle = node->particle;
            node->particle = nullptr;
            node->isLeaf = false;
            for (int i = 0; i < 8; ++i)
            { // th node có hạt thì mình sẽ chia thành 8 node con
                node->children[i] = nodePool.allocate(node->boundary.getOctant(i));
            }
            insert(node, oldParticle); // chèn lại hạt cũ vào các node vừa tạo ỏ trên.
            insert(node, p);
        }
    }
    else
    {
        glm::vec3 center = node->boundary.center;
        glm::vec3 pos = p->position;
        int octantIndex = 0;
        if (pos.x > center.x)
            octantIndex |= 1; // x > tâm thf mnhf + 1
        if (pos.y > center.y)
            octantIndex |= 2; // y > tâm thf mnhf cộng 2
        if (pos.z > center.z)
            octantIndex |= 4; // z > tâm thì cộng 4

        insert(node->children[octantIndex], p);
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
        {
            node->centerOfMass /= node->totalMass;
        }
    }
}
