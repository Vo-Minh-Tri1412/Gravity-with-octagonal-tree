#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include <stdexcept>
// đây gọi là memory pool, có nghĩa là reset toàn bộ thì chỉ việc dời con trỏ về không.
template <typename T>
class LinearAllocator
{
public:
    // mặc định cấp sẵn chỗ 10.000 phần tử
    explicit LinearAllocator(size_t initialCapacity = 10000)
    {
        pool.reserve(initialCapacity);
    }

    template <typename... Args> // cấp phát một đối tượng T mới
    T *allocate(Args &&...args)
    {
        // QUAN TRỌNG: KHÔNG được gọi reserve() ở đây vì sẽ realloc buffer
        // làm tất cả con trỏ đã trả về trước đó (node->children[]) thành dangling pointer.
        // Nếu pool đầy, tăng capacity khi khởi tạo thay vì mở rộng ở đây.
        assert(currentIndex < pool.capacity() &&
               "LinearAllocator overflow! Tăng capacity khi khởi tạo Octree.");

        if (currentIndex >= pool.size())
        {
            // emplace_back an toàn vì size < capacity (không realloc)
            pool.emplace_back(std::forward<Args>(args)...);
        }
        else
        {
            new (&pool[currentIndex]) T(std::forward<Args>(args)...);
        }
        return &pool[currentIndex++];
    }

    // xóa nhanh cho frame tiếp theo
    void reset()
    {
        currentIndex = 0;
    }

    [[nodiscard]] size_t getUsedCount() const { return currentIndex; }
    [[nodiscard]] size_t getCapacity() const { return pool.capacity(); }

private:
    std::vector<T> pool;
    size_t currentIndex = 0;
};

// Empty
