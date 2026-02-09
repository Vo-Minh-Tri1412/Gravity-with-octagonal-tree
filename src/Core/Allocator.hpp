#pragma once
#include <vector>
#include <memory>
#include <cassert>
// đây gọi là memory pool, có nghĩa là reset toàn bộ thì chỉ việc dời con trỏ về không.
template <typename T> 
class LinearAllocator {
public:
// mặc định cấp sẵn chỗ 10.000 phần tử
explicit LinearAllocator(size_t initialCapacity = 10000) {
        pool.reserve(initialCapacity);
    }


template<typename... Args>// cấp phát một đối tượng T mới 
    T* allocate(Args&&... args) {

        // nếu pool đầy,mở rộng gấp đôi
        if (currentIndex >= pool.size()) {
            if (currentIndex >= pool.capacity()) {
                pool.reserve(pool.capacity() * 2);}
            pool.emplace_back(std::forward<Args>(args)...);} 
            else {
            new (&pool[currentIndex]) T(std::forward<Args>(args)...);}
        return &pool[currentIndex++];
    }

    // xóa nhanh cho frame tiếp theo
    void reset() {
        currentIndex = 0;
    }

    [[nodiscard]] size_t getUsedCount() const { return currentIndex; }
    [[nodiscard]] size_t getCapacity() const { return pool.capacity(); }



    private:
    std::vector<T> pool;
    size_t currentIndex = 0;
};


// Empty
