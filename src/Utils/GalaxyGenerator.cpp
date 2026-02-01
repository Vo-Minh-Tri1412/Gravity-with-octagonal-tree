#include "../Core/Particle.hpp"
#include <random>
#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>

using namespace std;
using namespace glm;

namespace GalaxyGenerator {
struct GalaxyConfig {
    int numParticles = 1000;
    float radius = 500.0f;
    float coreRadius = 50.0f; // Bán kính tâm
    float armTwist = 3.0f; // Được tính toán trong phương trình góc quay của của hạt, awmtwist càng lớn thì hạt quay càng nhiều khi ra tới rìa
    int numArms = 4; // Số lượng xoắn ốc
    float armSpread = 0.5f; // Độ lan tỏa của các hạt
    float diskThickness = 10.0f; // Độ dày của vùng không gian đang xét tới
    float coreMass = 100000.0f; // Khối lượng trung tâm hố đen
    float G = 1.0f; // Hằng số hấp dẫn = 1 dể đơn giản hóa phép tính
    float scaleLength = 500.0f / 3.0f; // để xác định vùng ranh giới giữa nơi có mật độ hạt cao và thấp
    float concentrationPower = 0.5f; // Hệ số để phân phối khối lượng ở gần tâm
    float bulgeFraction = 0.2f; // tỉ lệ hạt trong phần trung tâm thiên hà so với tổng
    float velocityDispersion = 0.1f; // sử dụng để thêm một vận tốc ngẫu nhiên vào vận tốc cơ bản của hạt
};

vector<Particle> Generate(const GalaxyConfig& config) {
    vector<Particle> particles;
    particles.reserve(config.numParticles);              // Cấp phát trước bộ nhớ (tránh resize O(N), tối ưu cache).
    static random_device rd;                             // Nguồn entropy thật từ phần cứng (chỉ dùng seed).
    static mt19937 gen(rd());                            // Khởi tạo Mersenne Twister với seed từ random_device (static: chỉ khởi tạo 1 lần).
    uniform_real_distribution<float> dist01(0.0f, 1.0f); // Phân phối đều U[0, 1) (dùng cho phân bố bán kính và chọn arm).
    uniform_real_distribution<float> distAngle(0.0f, 2.0f * numbers::pi_v<float>); // Phân phối đều U[0, 2π) (dùng cho góc ngẫu nhiên).
    normal_distribution<float> distNormal(0.0f, 1.0f);   // Phân phối chuẩn N(0, 1) (dùng cho nhiễu Gaussian).
    for (int i = 0; i < config.numParticles; ++i) { 
        Particle p;                                      // Khởi tạo một hạt trống.
        float scaleLength = config.scaleLength;          // Lấy scale length từ config (r_d).
        float u = dist01(gen);                           // Sinh số ngẫu nhiên u ~ U[0, 1).
        float r = -scaleLength * log(1.0f - u + 1e-6f);  // Tính bán kính r theo Exponential Disk (Inverse Transform Sampling).
        r = min(r, config.radius);                       // Giới hạn bán kính r không vượt quá radius tối đa của thiên hà.
        int armIndex = static_cast<int>(dist01(gen) * config.numArms); // Chọn arm ngẫu nhiên mà hạt thuộc về.
        float angleStep = (2.0f * numbers::pi_v<float>) / config.numArms; // Góc cách đều giữa mỗi arm (2π / N_arms).
        float baseAngle = armIndex * angleStep;          // Góc cơ sở của arm đã chọn.
        float spiralAngle = config.armTwist * (r / config.radius); // Góc xoắn ốc (θ_spiral ∝ r, Logarithmic Spiral).
        float noise = distNormal(gen) * config.armSpread; // Thêm nhiễu ngẫu nhiên Gaussian (tạo độ rộng cánh tay).
        float finalAngle = baseAngle + spiralAngle + noise; // Góc polar cuối cùng của hạt (θ_final = θ_base + θ_spiral + θ_noise).
        p.pos.x = r * cos(finalAngle);
        p.pos.z = r * sin(finalAngle); 
        p.pos.y = distNormal(gen) * config.diskThickness; // Thiết lập độ cao y ~ N(0, diskThickness) (độ dày đĩa Gaussian).

        float distToCenter = length(p.pos);              // Tính khoảng cách thực 3D đến tâm (d = |r|).
        float epsilon = 1.0f;                            // Hằng số Softening (ε), ngăn vận tốc vô cực tại tâm.
        float velocityMag = sqrt((config.G * config.coreMass) / (distToCenter + epsilon)); // Tính độ lớn vận tốc Kepler (v = √(G·M / (d + ε))).
        
        vec3 up(0.0f, 1.0f, 0.0f);                       // Vector trục quay (Y-axis).
        vec3 tangent;                                    // Vector đơn vị tiếp tuyến (hướng vận tốc).
        
        if (distToCenter < 0.01f) {                      // Xử lý trường hợp hạt quá gần tâm (bulge).
            vec3 temp_tangent(distNormal(gen), 0.0f, distNormal(gen)); // Vận tốc ngẫu nhiên trong mặt phẳng XZ (quỹ đạo hỗn loạn).
            tangent = normalize(temp_tangent);           // Chuẩn hóa vector vận tốc ngẫu nhiên.
        } else {
            tangent = normalize(cross(up, p.pos));       // Tính vector tiếp tuyến = normalize(trục quay × vị trí) (vuông góc với up và r).
        }
        
        p.vel = tangent * velocityMag;                   // Vận tốc quỹ đạo = vector tiếp tuyến × độ lớn vận tốc.
        
        float dispersion = config.velocityDispersion * velocityMag; // Tính độ lớn nhiễu vận tốc (σ = %Dispersion × |v_orbital|).
        p.vel.x += distNormal(gen) * dispersion;         // Thêm nhiễu Gaussian N(0, σ) vào vận tốc trục X.
        p.vel.y += distNormal(gen) * dispersion * 0.5f;  // Thêm nhiễu Gaussian N(0, 0.5σ) vào vận tốc trục Y (Anisotropic Dispersion).
        p.vel.z += distNormal(gen) * dispersion;         // Thêm nhiễu Gaussian N(0, σ) vào vận tốc trục Z.
        
        p.mass = 1.0f;                                   // Thiết lập khối lượng mặc định (equal-mass).
        p.acc = vec3(0.0f, 0.0f, 0.0f);                  // Khởi tạo gia tốc bằng 0.
        
        particles.push_back(p);                          // Thêm hạt đã sinh vào vector.
    }
    
    return particles;                                    // Trả về vector hạt đã sinh (dùng move semantics/RVO).
}
}