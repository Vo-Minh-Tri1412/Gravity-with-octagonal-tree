# 🌌 Gravity Simulation — Barnes-Hut & Octree

Mô phỏng lực hấp dẫn N-Body thời gian thực bằng C++20, sử dụng thuật toán **Barnes-Hut** và cấu trúc dữ liệu **Octree** để đạt hiệu năng $O(N \log N)$ thay vì $O(N^2)$ brute-force truyền thống. Chạy mượt với 4 000 hạt trên phần cứng phổ thông.

> **Môn học:** Cấu trúc Dữ liệu & Giải thuật — HCMUS  
> **Ngôn ngữ:** C++20 · **Đồ họa:** Raylib 5.5 · **Toán học:** GLM · **Build:** CMake + vcpkg

---

## 📑 Mục lục

1. [Tính năng nổi bật](#1-tính-năng-nổi-bật)
2. [Kiến trúc hệ thống](#2-kiến-trúc-hệ-thống)
3. [Cấu trúc thư mục](#3-cấu-trúc-thư-mục)
4. [Thuật toán & Cấu trúc dữ liệu](#4-thuật-toán--cấu-trúc-dữ-liệu)
5. [Ba kịch bản mô phỏng](#5-ba-kịch-bản-mô-phỏng)
6. [Hướng dẫn cài đặt & Build](#6-hướng-dẫn-cài-đặt--build)
7. [Điều khiển](#7-điều-khiển)
8. [Tối ưu hóa hiệu năng](#8-tối-ưu-hóa-hiệu-năng)

---

## 1. Tính năng nổi bật

| Tính năng | Chi tiết |
|---|---|
| **Thuật toán Barnes-Hut** | Tính lực hấp dẫn $O(N \log N)$ thay vì $O(N^2)$ |
| **Octree động** | Xây lại mỗi frame, tái sử dụng pool $O(1)$ reset |
| **Linear Allocator** | Memory pool không heap-alloc trong vòng lặp chính |
| **3 kịch bản** | Thiên Hà (4 000 hạt), Hệ Mặt Trời (9 hành tinh), Thí nghiệm vận tốc vũ trụ |
| **Camera quỹ đạo thủ công** | Tọa độ cầu — kéo chuột phải để xoay, cuộn để zoom |
| **Màu theo khối lượng** | Gradient đỏ → vàng → xanh lạnh theo loại thiên thể |
| **Đổi solver thời gian thực** | Chuyển Barnes-Hut ↔ Brute-Force ngay khi đang chạy |

---

## 2. Kiến trúc hệ thống

Dự án áp dụng tư tưởng **Data-Oriented Design (DOD)**: tách biệt hoàn toàn dữ liệu và logic xử lý.

```
┌─────────────────────────────────────────────────────┐
│                    Vòng lặp chính                   │
│                                                     │
│  INPUT         PHYSICS              RENDER          │
│  ──────        ───────              ──────          │
│  Bàn phím  →  Build Octree      →  BeginScene       │
│  Chuột     →  Tính lực (Solver) →  RenderParticles  │
│               Cập nhật pos/vel  →  EndScene         │
│                                 →  RenderUI         │
│                                 →  EndFrame         │
└─────────────────────────────────────────────────────┘
         ↕ Dữ liệu duy nhất: std::vector<Particle>
```

**Pipeline mỗi frame:**

1. **Input** — xử lý phím/chuột, cập nhật camera
2. **Octree::build()** — `reset()` pool O(1) → chèn tất cả hạt → tính khối tâm đệ quy
3. **Solver::solve()** — duyệt từng hạt, dùng Octree tính gia tốc , tích phân rời rạc
4. **Render** — vẽ 3D (`BeginMode3D…EndMode3D`) → vẽ 2D HUD → `EndDrawing`

---

## 3. Cấu trúc thư mục

```
Gravity-with-octagonal-tree/
├── CMakeLists.txt              # Cấu hình build chính (C++20, Release -O3)
├── vcpkg.json                  # Khai báo dependency: raylib, glm, gtest
├── src/
│   ├── main.cpp                # Entry point — vòng lặp chính, xử lý input
│   ├── Core/
│   │   ├── Particle.hpp        # Struct Particle (POD) + enum ParticleType
│   │   └── Allocator.hpp       # LinearAllocator<T> — memory pool template
│   ├── Structure/
│   │   ├── AABB.hpp            # Axis-Aligned Bounding Box + phân chia octant
│   │   ├── Octree.hpp          # Khai báo OctreeNode, class Octree
│   │   └── Octree.cpp          # build(), insert(), computeMassDistribution()
│   ├── Physics/
│   │   ├── ISolver.hpp         # Interface thuần ảo ISolver::solve()
│   │   ├── BarnesHutSolver.hpp # Solver O(N log N) — dùng Octree, θ=0.5
│   │   ├── BarnesHutSolver.cpp # calculateForce() đệ quy + tích phân Euler
│   │   ├── BruteForceSolver.hpp# Solver O(N²) — kiểm chứng
│   │   └── BruteForceSolver.cpp
│   ├── Graphics/
│   │   ├── Camera.hpp          # CameraWrapper — tọa độ cầu (azimuth/elevation)
│   │   ├── Camera.cpp          # Update() xử lý chuột; Reset() khi đổi kịch bản
│   │   ├── Renderer.hpp        # Khai báo Renderer: BeginScene/EndScene/EndFrame
│   │   └── Renderer.cpp        # Vẽ hạt theo type, màu theo khối lượng, HUD
│   └── Utils/
│       ├── GalaxyGenerator.hpp # ScenarioType, GalaxyConfig, TwoBodyConfig
│       ├── GalaxyGenerator.cpp # generateGalaxy / generateSolarSystem / generateTwoBody
│       └── Timer.hpp           # High-resolution timer đo thời gian physics (ms)
```

---

## 4. Thuật toán & Cấu trúc dữ liệu

### 4.1 Octree

Chia không gian 3D thành 8 ô con (octant) theo đệ quy. Mỗi **node lá** giữ đúng 1 hạt; khi node lá nhận hạt thứ 2, nó **phân chia** thành 8 con và đẩy cả hai hạt xuống.

```
OctreeNode {
    AABB boundary          // Hộp không gian quản lý
    OctreeNode* children[8]// 8 ô con (null nếu chưa phân chia)
    Particle* particle     // con trỏ hạt (chỉ có ở node lá)
    vec3 centerOfMass      // khối tâm tổng hợp
    float totalMass        // tổng khối lượng
    bool isLeaf
}
```

**Mã hóa octant** — bit-packing 3 bit `xyz`:

| Bit 2 (z) | Bit 1 (y) | Bit 0 (x) | Index |
|:---------:|:---------:|:---------:|:-----:|
| 0 | 0 | 0 | 0 |
| 0 | 0 | 1 | 1 |
| … | … | … | … |
| 1 | 1 | 1 | 7 |

### 4.2 Thuật toán Barnes-Hut

Với mỗi hạt $p$, duyệt cây từ gốc theo quy tắc:

$$\frac{s}{d} < \theta \implies \text{xấp xỉ cả nhóm thành 1 điểm}$$

- $s$ = kích thước cạnh của node  
- $d$ = khoảng cách từ $p$ đến khối tâm node  
- $\theta = 0.5$ (tiêu chuẩn, trade-off độ chính xác/tốc độ)

Tối ưu thêm: so sánh $s^2 < \theta^2 \cdot d^2$ để **tránh gọi `sqrt`** ở phần lớn node trung gian; `sqrt` chỉ tính khi thực sự áp dụng xấp xỉ hoặc gặp node lá có hạt khác.

**Công thức lực hấp dẫn:**

$$\vec{a} \mathrel{+}= \frac{G \cdot M_{node}}{r^2 + \varepsilon^2} \cdot \hat{r}$$

| Hằng số | Giá trị | Ý nghĩa |
|---|---|---|
| $G$ | `6.674e-2` | Hằng số hấp dẫn mô phỏng |
| $\varepsilon$ | `1e-3` | Softening — tránh chia cho 0 |
| $\theta$ | `0.5` | Ngưỡng xấp xỉ Barnes-Hut |

Tích phân vị trí dùng **phương pháp Euler**:

$$\vec{v}_{t+1} = \vec{v}_t + \vec{a}_t \cdot dt \qquad \vec{x}_{t+1} = \vec{x}_t + \vec{v}_{t+1} \cdot dt$$

### 4.3 LinearAllocator (Memory Pool)

```
LinearAllocator<OctreeNode>
├── pool: std::vector<OctreeNode>   // bộ nhớ liên tục, pre-allocated
├── currentIndex: size_t            // con trỏ đầu tự do
├── allocate(...) → O(1)            // emplace tại currentIndex, index++
└── reset()       → O(1)           // currentIndex = 0, không giải phóng bộ nhớ
```

**Tại sao quan trọng?**  
Octree rebuild mỗi frame. Nếu dùng `new`/`delete`, mỗi frame sẽ phân bổ/giải phóng **~3.8 MB** heap — gây phân mảnh và giảm FPS nghiêm trọng. Pool reset về `currentIndex = 0` trong $O(1)$, không có heap allocation nào trong vòng lặp chính.

> ⚠️ **Ràng buộc quan trọng:** `pool.reserve()` chỉ được gọi khi khởi tạo. Gọi trong `allocate()` sẽ kích hoạt `std::vector` realloc, làm tất cả con trỏ `children[]` đã trả về trở thành **dangling pointer**.

---

## 5. Ba kịch bản mô phỏng

### 🌀 Kịch bản 1 — Thiên Hà (phím `1`)

Sinh 4 000 hạt theo phân bố mật độ thực tế của thiên hà dạng đĩa:

- **Vùng lõi** (30% hạt, `r < 40`): mật độ cao, hạt nặng hơn, $r \propto \sqrt{u}$
- **Đĩa ngoài** (70% hạt): mật độ giảm dần, $r \propto t^{1.5}$
- Hệ tọa độ cầu → Descartes, sau đó **ép trục z × 0.1** để tạo hình đĩa phẳng
- Vận tốc quỹ đạo tiếp tuyến: $v = \sqrt{GM_{core}/r}$, thêm nhiễu Gaussian để tạo velocity dispersion tự nhiên
- **Solver khuyến nghị:** Barnes-Hut — `camera.Reset(r=450)`

### ☀️ Kịch bản 2 — Hệ Mặt Trời (phím `2`)

9 thiên thể (Mặt Trời + 8 hành tinh), vận tốc quỹ đạo tròn chính xác:

| Hành tinh | Khoảng cách (units) | Khối lượng |
|-----------|:-------------------:|:----------:|
| Mercury   | 20 | 0.05 |
| Venus     | 35 | 0.80 |
| Earth     | 50 | 1.00 |
| Mars      | 70 | 0.10 |
| Jupiter   | 120 | 50.0 |
| Saturn    | 160 | 30.0 |
| Uranus    | 210 | 14.0 |
| Neptune   | 260 | 17.0 |

Mặt Trời: `mass = 2000`, `type = STAR`. Vận tốc: $v = \sqrt{G \cdot M_{sun} / r}$, hướng vuông góc trục X.  
**Solver khuyến nghị:** Brute-Force — `camera.Reset(r=300)`

### 🚀 Kịch bản 3 — Thí nghiệm Vận tốc Vũ trụ (phím `3`)

Mô phỏng thực nghiệm phóng tên lửa từ bề mặt hành tinh, minh họa ba chế độ quỹ đạo:

| Trường hợp | Điều kiện | Kết quả |
|---|---|---|
| **Quỹ đạo (BOUND)** | $v < v_{escape}$ | Tên lửa bị hấp dẫn, bay theo elip/tròn |
| **Thoát khỏi (ESCAPED)** | $v \geq v_{escape}$ | Tên lửa thoát khỏi lực hấp dẫn |
| **Va chạm (CRASHED)** | $dist < 0.95 \cdot R$ | Tên lửa rơi trở lại hành tinh |

**Công thức vận tốc**

$$v_{escape} = \sqrt{\frac{2GM}{R}} \qquad v_{circular} = \sqrt{\frac{GM}{R}}$$

**Cấu hình mặc định (`TwoBodyConfig`):**

| Tham số | Giá trị mặc định | Ý nghĩa |
|---|:---:|---|
| `earthMass` | 500 | Khối lượng hành tinh |
| `earthRadius` | 10 | Bán kính bề mặt (1 unit ≈ 637 km) |
| `launchSpeed` | 2.58 | Tốc độ phóng ban đầu (~$v_{escape}$) |
| `launchAngleDeg` | 90° | Góc phóng (0° = xuyên tâm, 90° = tiếp tuyến) |

Tên lửa được đặt tại $(R, 0, 0)$ với vận tốc:

$$\vec{v} = v_{launch} \cdot (\cos\alpha,\ \sin\alpha,\ 0)$$

HUD hiển thị thời gian mô phỏng, $v_{launch}$, $v_{escape}$, $v_{circular}$, trạng thái quỹ đạo theo màu (xanh lá/đỏ/cam).

---

## 6. Hướng dẫn cài đặt & Build

### Yêu cầu hệ thống

| Công cụ | Phiên bản | Ghi chú |
|---|---|---|
| CMake | ≥ 3.15 | |
| MinGW-w64 (GCC) | ≥ 13 | Hoặc MSVC 2022 |
| vcpkg | latest | Cài tại `C:/vcpkg` để CMake tự nhận |
| Git | bất kỳ | |

### Bước 1 — Cài vcpkg (chỉ làm 1 lần)

```powershell
cd C:/
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
```

### Bước 2 — Clone dự án

```bash
git clone https://github.com/Vo-Minh-Tri1412/Gravity-with-octagonal-tree/
cd Gravity-with-octagonal-tree
```

### Bước 3 — Cấu hình & Build (Release)

```powershell
# Cấu hình CMake (tự động dùng vcpkg toolchain nếu cài tại C:/vcpkg)
cmake -B build -S . -G "MinGW Makefiles" `
      -DCMAKE_BUILD_TYPE=Release `
      -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"

# Build
cmake --build build --config Release -- -j4
```

Lần đầu chạy, vcpkg sẽ tự tải và biên dịch `raylib`, `glm`, `gtest` (~vài phút).

### Bước 4 — Chạy

```powershell
.\build\src\gravity.exe
```

### Build Debug (có thể dùng để chạy test)

```powershell
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ...
cmake --build build --config Debug
cd build ; ctest --output-on-failure
```

> **Lưu ý:** Build Debug (`-O0`) chậm hơn nhiều lần so với Release (`-O3 -march=native`). Luôn dùng Release để đánh giá hiệu năng thực.

---

## 7. Điều khiển

### Toàn bộ kịch bản

| Phím | Hành động |
|---|---|
| `1` | Chuyển sang **Thiên Hà** (Barnes-Hut, 4000 hạt) |
| `2` | Chuyển sang **Hệ Mặt Trời** (Brute-Force, 9 thiên thể) |
| `3` | Chuyển sang **Thí nghiệm Vận tốc Vũ trụ** |
| `R` | **Reset** kịch bản hiện tại về trạng thái ban đầu |
| `B` | Đổi solver → **Brute-Force** $O(N^2)$ |
| `H` | Đổi solver → **Barnes-Hut** $O(N \log N)$ |

### Camera (mọi kịch bản)

| Thao tác | Hành động |
|---|---|
| Kéo **chuột phải** | Xoay camera (azimuth / elevation) |
| **Cuộn chuột** | Zoom in / out (thay đổi bán kính quỹ đạo) |

### Thí nghiệm Vận tốc Vũ trụ (kịch bản 3)

| Phím | Hành động |
|---|---|
| `↑` / `↓` | Tăng / giảm tốc độ phóng ± 0.1 (tự reset thí nghiệm) |
| `→` / `←` | Tăng / giảm góc phóng ± 5° (tự reset thí nghiệm) |
| `R` | Phóng lại với cấu hình hiện tại |

---

## 📦 Dependency

| Thư viện | Phiên bản | Chức năng |
|---|---|---|
| [Raylib](https://www.raylib.com/) | 5.5 | Cửa sổ, render 3D, input |
| [GLM](https://github.com/g-truc/glm) | 1.x | Toán học vector/matrix |
| [GoogleTest](https://github.com/google/googletest) | latest | Unit testing |

Tất cả được quản lý tự động qua `vcpkg.json`.
