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

    // ===== HỆ 2 VẬT THỂ =====
    std::vector<Core::Particle> generateTwoBody();
}