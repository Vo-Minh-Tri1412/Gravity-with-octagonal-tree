
#include "../Utils/GalaxyGenerator.cpp"

// Struct định nghĩa AABB (hộp giới hạn)
struct AABB {
    glm::vec3 min;  // Góc min (x_min, y_min, z_min)
    glm::vec3 max;  // Góc max (x_max, y_max, z_max)
    
    AABB(glm::vec3 minPos, glm::vec3 maxPos) : min(minPos), max(maxPos) {}
    
    // Kiểm tra một điểm có nằm trong hộp không
    bool contains(const glm::vec3& point) const {
        return (point.x >= min.x && point.x <= max.x) &&
               (point.y >= min.y && point.y <= max.y) &&
               (point.z >= min.z && point.z <= max.z);
    }
};

// Test: Kiểm tra tất cả hạt nằm trong AABB
void testGalaxyAABB() {
    std::cout << "\n=== TEST: Galaxy Particles in AABB ===" << std::endl;
    
    // Cấu hình
    Core::GalaxyConfig config;
    config.totalParticles = 10000;
    config.galaxyRadius = 100.0f;
    config.includeBlackHole = false;
    
    // Sinh thiên hà
    std::vector<Core::Particle> particles = Core::generateGalaxy(config);
    
    // Định nghĩa AABB (hộp bao quanh thiên hà)
    // Vì z *= 0.1f nên chiều cao nhỏ hơn
    AABB boundingBox(
        glm::vec3(-config.galaxyRadius, -config.galaxyRadius, -config.galaxyRadius * 0.1f),
        glm::vec3(config.galaxyRadius, config.galaxyRadius, config.galaxyRadius * 0.1f)
    );
    
    // Kiểm tra từng hạt
    int particlesOutside = 0;
    int particlesInside = 0;
    
    for (const Core::Particle& p : particles) {
        if (boundingBox.contains(p.position)) {
            particlesInside++;
        } else {
            particlesOutside++;
            std::cout << "Particle outside AABB at: ("
                 << p.position.x << ", "
                 << p.position.y << ", "
                 << p.position.z << ")" << std::endl;
        }
    }
    
    std::cout << "Particles inside AABB: " << particlesInside << "/" << particles.size() << std::endl;
    std::cout << "Particles outside AABB: " << particlesOutside << "/" << particles.size() << std::endl;
    
    // Assert: Tất cả hạt phải nằm trong AABB
    assert(particlesOutside == 0 && "Some particles are outside AABB!");
    
    std::cout << "TEST PASSED: All particles in AABB" << std::endl;
}