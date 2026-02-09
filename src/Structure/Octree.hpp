#pragma once
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "../Core/Particle.hpp"
#include "../Core/Allocator.hpp"
#include "AABB.hpp"

struct Particle;
//node cây bát phân
struct OctreeNode {
    //không gian vật lýlý:hộp giới hạn không gian mà Node này quản lý 
    AABB boundary;
    std::array<OctreeNode*, 8> children{};
    //nếu đây là node lá , nó sẽ giữ con trỏ đến hạt thực tế
    //nếu đây là node nhánh , biến này sẽ là null
    Particle* particle;




    glm::vec3 centerOfMass;
    float totalMass;



    bool isLeaf;// true = node lá, false = Node nhánh.


    explicit OctreeNode(const AABB& bounds) //khởi tạo
        : boundary(bounds), particle(nullptr), 
          centerOfMass(0.0f), totalMass(0.0f), isLeaf(true) {
        children.fill(nullptr);///khởi tạo toàn bộ 8 con là null
        }};



class Octree {
public:
    Octree(const AABB& rootBounds, size_t maxCapacity = 50000);
    ~Octree() = default;
    void build(std::vector<Particle>& particles);
    // xóa cây cũ+xây cây mới từ danh sách hạt hiện tại+tính toán Center of Mass cho toàn bộ cây
    [[nodiscard]] OctreeNode* getRoot() const { return root; }



private:
    OctreeNode* root;
    LinearAllocator<OctreeNode> nodePool; 
    AABB initialBounds;
    //node đầy ->chia nhỏ thành 8 con-->đẩy hạt xuống con
    void insert(OctreeNode* node, Particle* p);
};

