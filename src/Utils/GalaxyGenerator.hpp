#pragma once
#include <vector>
#include <string>
#include "../Core/Particle.hpp"

namespace Utils
{
    // ===== LOẠI KỊCH BẢN =====
    enum class ScenarioType
    {
        GALAXY,       // Thiên hà (4000 hạt — Barnes-Hut)
        SOLAR_SYSTEM, // Hệ Mặt Trời (9 hành tinh)
        TWO_BODY      // Hệ 2 vật thể (Trái Đất + Mặt Trăng)
    };
    std::string getScenarioName(ScenarioType type);

    // ===== THIÊN HÀ =====
    struct GalaxyConfig
    {
        int totalParticles = 4000;
        float galaxyRadius = 300.0f;
        float coreRadius = 40.0f;
        float coreMassFraction = 0.5f;
        float minMass = 1.0f;
        float maxMass = 100.0f;
        float maxVelocity = 15.0f;
        unsigned int seed = 12345;
    };
    std::vector<Core::Particle> generateGalaxy(const GalaxyConfig &config = GalaxyConfig());

    // ===== HỆ MẶT TRỜI =====
    std::vector<Core::Particle> generateSolarSystem();

    // ===== THÍ NGHIỆM VẬN TỐC VŨ TRỤ =====
    struct TwoBodyConfig
    {
        float earthMass = 500.0f;     // khối lượng Trái Đất (đơn vị mô phỏng)
        float earthRadius = 10.0f;    // bán kính Trái Đất (1 unit ≈ 637 km)
        float launchSpeed = 2.58f;    // tốc độ phóng (~v_escape)
        float launchAngleDeg = 90.0f; // 0° = xuyên tâm, 90° = tiếp tuyến
    };
    float getEscapeVelocity(const TwoBodyConfig &cfg);       // √(2GM/R)
    float getCircularOrbitalSpeed(const TwoBodyConfig &cfg); // √(GM/R)
    std::vector<Core::Particle> generateTwoBody(const TwoBodyConfig &cfg = TwoBodyConfig());
}