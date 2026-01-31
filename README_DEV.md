# 🌌 Gravity Simulation with Octree (Barnes-Hut)

Chào mừng đến với dự án mô phỏng trọng lực N-Body sử dụng thuật toán Barnes-Hut và cấu trúc dữ liệu Octree. Đây là tài liệu hướng dẫn toàn diện (Encyclopedia) cho dự án.

---

## 📂 1. Cấu trúc Dự án & Nhiệm vụ File

Dưới đây là bản đồ chi tiết về vị trí và chức năng của từng file trong dự án.

### 🏗️ Root Directory
*   `CMakeLists.txt`: File cấu hình build chính của CMake. Quản lý các dependency và thiết lập chuẩn C++ (C++20).
*   `vcpkg.json`: (Nếu có) Khai báo các thư viện bên thứ 3 cần thiết.
*   `.gitignore`: Những file Git nên bỏ qua (như thư mục build, file tạm).

### 🎨 Assets (`assets/`)
Nơi chứa tài nguyên tĩnh.
*   `shaders/`: Chứa mã nguồn GLSL cho đồ họa (Vertex/Fragment shaders).
*   `configs/`: Các file `.json` để cấu hình tham số mô phỏng (Hằng số G, số lượng hạt, dt...) mà không cần compile lại code.
*   `textures/`: Hình ảnh texture cho các hạt hoặc nền.

### 📚 Documentation (`docs/`)
*   `architecture.md`: Sơ đồ kiến trúc hệ thống, luồng dữ liệu.
*   `api_reference.md`: Tài liệu chi tiết về các hàm và class.

### 🧪 Quality Assurance (`tests/`)
Đảm bảo code chạy đúng.
*   `CMakeLists.txt`: Cấu hình build cho test.
*   `core_tests.cpp`: Test các class cơ bản (Vector3, Matrix, Math utils).
*   `physics_tests.cpp`: Test tính đúng đắn của công thức lực hấp dẫn.
*   `octree_tests.cpp`: Test việc chia không gian và chèn hạt vào Octree.

### 💻 Source Code (`src/`)
Mã nguồn chính, được chia thành các module độc lập.

#### 🔹 Core (`src/Core/`) - *Nền tảng*
Không phụ thuộc vào module khác.
*   `Particle.hpp`: Struct định nghĩa một hạt (vị trí, vận tốc, khối lượng).
*   `MathTypes.hpp`: Định nghĩa các kiểu toán học (Vector3, Matrix3x3).
*   `Allocator.hpp`: Quản lý bộ nhớ (Memory Pool) để tối ưu hiệu năng khi tạo/xóa hạt liên tục.

#### 🔹 Physics (`src/Physics/`) - *Logic Vật lý*
*   `ISolver.hpp`: Interface chung cho các thuật toán giải bài toán N-Body.
*   `BruteForceSolver.cpp`: Thuật toán ngây thơ O(N^2) để so sánh kết quả.
*   `BarnesHutSolver.cpp`: Thuật toán tối ưu O(N log N) dùng Octree.

#### 🔹 Structure (`src/Structure/`) - *Cấu trúc dữ liệu*
*   `Octree.hpp`: Cài đặt cây bát phân.
*   `AABB.hpp`: Axis-Aligned Bounding Box - Hộp bao quanh để xác định vùng không gian.

#### 🔹 Graphics (`src/Graphics/`) - *Hiển thị*
*   `Renderer.hpp`: Quản lý việc vẽ các hạt lên màn hình (OpenGL/Vulkan).
*   `Camera.hpp`: Quản lý góc nhìn, zoom, di chuyển.

#### 🔹 Utils (`src/Utils/`) - *Tiện ích*
*   `GalaxyGenerator.cpp`: Sinh dữ liệu hạt ban đầu (hình xoắn ốc, hình cầu...).
*   `Timer.hpp`: Đo thời gian thực thi để kiểm tra hiệu năng (FPS, Physics time).

#### 🔹 Main
*   `main.cpp`: Entry Point. Chỉ làm nhiệm vụ khởi tạo hệ thống và vòng lặp chính.

---

## 🚀 2. Hướng dẫn Cài đặt & Build (Từ con số 0)

Dành cho người mới bắt đầu, chưa cài đặt công cụ gì trên máy tính (Windows).

### Bước 1: Cài đặt Công cụ Cần thiết
1.  **Git**: Tải và cài đặt Git SCM. Khi cài cứ Next hết là được.
2.  **Visual Studio Code (VS Code)**: Tải và cài đặt VS Code.
    *   Sau khi cài xong, mở VS Code, vào tab Extensions (bên trái), cài các extension:
        *   *C/C++* (Microsoft)
        *   *CMake Tools* (Microsoft)
3.  **Compiler & CMake**:
    *   Cách dễ nhất: Cài đặt Visual Studio Community 2022.
    *   Khi cài đặt, chọn workload **"Desktop development with C++"**. Nó sẽ tự cài MSVC compiler và CMake cho bạn.

### Bước 2: Cài đặt Công cụ vcpkg (Chỉ làm 1 lần)
Để dự án tự động tải thư viện, bạn cần có công cụ `vcpkg` nằm tại `C:/vcpkg`.
*Nếu bạn đã có folder `C:/vcpkg` rồi thì bỏ qua bước này.*

1.  Mở thư mục ổ C ( `C:/`).
2.  Mở Terminal (PowerShell hoặc CMD) tại đó.
3.  Chạy lệnh:
    ```cmd
    git clone https://github.com/microsoft/vcpkg
    cd vcpkg
    .\vcpkg\bootstrap-vcpkg.bat
    ```
4.  (Tùy chọn) Tích hợp vào hệ thống: `.\vcpkg\vcpkg integrate install`

### Bước 3: Tải và Build Dự án
1.  **Clone code về máy**:
    ```bash
    git clone <link-repo-của-nhóm>
    cd Gravity-with-octagonal-tree
    ```
2.  **Cấu hình CMake**:
    *   Mở folder dự án bằng VS Code.
    *   VS Code sẽ hỏi chọn Kit (Compiler). Chọn **Visual Studio Community 2022 Release - amd64** (hoặc GCC nếu bạn dùng MinGW).
    *   Nếu VS Code không tự nhận vcpkg, bạn cần thêm tham số vào `settings.json` hoặc chạy lệnh cmake thủ công:
    ```bash
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
    ```
    *(Thay `C:/dev/vcpkg` bằng đường dẫn nơi bạn cài vcpkg ở Bước 2)*.


3.  **Build**:
    *   Bấm nút **Build** trên thanh trạng thái dưới đáy VS Code, hoặc chạy:
    ```bash
    cmake --build build --config Release
    ```

