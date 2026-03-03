#pragma once
#include <vector>
#include <memory>
#include <cassert>
//CODE CẬP NHẬT CHO MMR POOL LẦN THỨ 2:CODE cũ dùng một mảng std::vector dài, khi đầy nó tự mở rộng =>làm dời toàn bộ dữ liệu sang chỗ khác =>lỗi con trỏ .
//CẬP NHẬT:đổi sang cấp phát theo từng BLOCK cố định. Đầy cục này thì tạo cục mới,cục cũ giữ nguyên vị trí.

template <typename T> 
class LinearAllocator {
public://vẫn mặc định sức chứa ban đầu là 10.000 phần tử(1 blck)
explicit LinearAllocator(size_t initialCapacity = 10000) 
        : blockSize(initialCapacity), currentBlock(0), currentIndex(0) {
            blocks.emplace_back(std::make_unique<T[]>(blockSize));
    }//cấp phát sãn 1 block đầu


    template<typename... Args>
    T* allocate(Args&&... args)//tra xem block hiện tại đã xài hết chưa
     {if (currentIndex >= blockSize) {
            currentBlock++; // nếu rồi thì nhảy sang block tiếp theo
            //th chưa có sẵn block tiếp theo thì nhờ  os cấp thêm 1 block mới
            if (currentBlock >= blocks.size()) {
                blocks.emplace_back(std::make_unique<T[]>(blockSize));
            }
            currentIndex = 0;}
            T* ptr = &blocks[currentBlock][currentIndex++];


            new (ptr) T(std::forward<Args>(args)...);
        return ptr;}//thay vì gọi new thông thường để xin ó cấp ram,ép nó chạy constructor của T và khởi tạo dữ liệu trực tiếp lên chính cái địa chỉ đps.
    

        void reset() {
        currentBlock = 0;
        currentIndex = 0; }//hết frame là xóa sạch, thay vì xóa các node thì chỉ cần cho 2 cái này về 0.


    [[nodiscard]] size_t getUsedCount() const { 
        return currentBlock * blockSize + currentIndex; 
    [[nodiscard]] size_t getCapacity() const { 
        return blocks.size() * blockSize; }}

private:
    size_t blockSize; //kích thước cố định 
    std::vector<std::unique_ptr<T[]>> blocks; 
    size_t currentBlock = 0; 
    size_t currentIndex = 0;};