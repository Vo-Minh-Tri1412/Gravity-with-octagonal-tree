#pragma once

#include <glm/glm.hpp>
//CODE NÀY ĐƯỢC UPDATE ĐỂ TỐI ƯU CPU Cache
namespace Core
struct alignas(32) Particle //alignas(32):bộ nhớ của struct này phải bắt đầu ở ĐC chia hết cho 32
    {
        glm::vec3 position; //12 bytes
        float mass; // 4 byte => vừa đẹp 16 bytes.
        glm::vec3 velocity;//12 bytes
        float padding1; //4 byte -> Biến padding này thêm vào để lấp đầy 16 bytes tiếp theo.
        // mấu chốt giúp bộ nhớ xếp thẳng hàng                 
        glm::vec3 acceleration; //12 bytes
        float padding2;// 4 byte=>tương tự
        //constructor giúp khởi tạo giá trị mặc định.
        //MNG nhớ khởi tạo luôn mấy biến padding bằng 0.0f để bộ nhớ sạch, không bị dính giá trị rác lúc cấp phát.
        Particle(glm::vec3 pos = glm::vec3(0.0f), float m = 1.0f)
        : position(pos), mass(m), velocity(0.0f), padding1(0.0f), acceleration(0.0f), padding2(0.0f) {}
    };
