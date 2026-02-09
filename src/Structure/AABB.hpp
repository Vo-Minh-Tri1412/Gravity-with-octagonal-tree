#pragma once
#include <glm/glm.hpp>

struct AABB {
    glm::vec3 center;
    glm::vec3 halfSize; // width/2, height/2, depth/2
    AABB() : center(0.0f), halfSize(0.0f) {}
    AABB(const glm::vec3& c, const glm::vec3& hs) : center(c), halfSize(hs) {}

//chẹc xem 1 điểm có nằm trong hộp không
[[nodiscard]] bool contains(const glm::vec3& point) const {
        return (point.x >= center.x - halfSize.x && point.x <=center.x + halfSize.x &&
                point.y >= center.y - halfSize.y && point.y <=center.y + halfSize.y &&
                point.z >= center.z - halfSize.z && point.z <=center.z + halfSize.z);
    }

//tiếp theo lấy hộp con tại index từ 0-7
[[nodiscard]] AABB getOctant(int index) const {
        glm::vec3 newHalf = halfSize * 0.5f;
        glm::vec3 offset;

        //quy định: 0 thành x,1 thành y, 2 thành z
        offset.x = (index & 1) ? newHalf.x : -newHalf.x;
        offset.y = (index & 2) ? newHalf.y : -newHalf.y;
        offset.z = (index & 4) ? newHalf.z : -newHalf.z;
        return AABB(center + offset, newHalf);}};


