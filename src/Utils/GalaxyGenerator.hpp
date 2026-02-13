#pragma once
#include <vector>
#include "../Core/Particle.hpp"

namespace Utils
{
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

    // Hàm sinh thiên hà trả về vector các hạt
    std::vector<Core::Particle> generateGalaxy(const GalaxyConfig &config = GalaxyConfig());
}