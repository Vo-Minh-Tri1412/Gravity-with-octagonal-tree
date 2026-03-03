// src/Utils/ScenarioGenerator.cpp

#include "ScenarioGenerator.hpp"
#include "GalaxyGenerator.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>

namespace Core
{
    // ========================================
    // SCENARIO 1: HỆ 2 VẬT THỂ
    // ========================================
    std::vector<Particle> generateTwoBodyScenario()
    {
        std::vector<Particle> particles;
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "   TWO-BODY SYSTEM GENERATOR" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // Vật thể 1: Trái Đất (tại tâm)
        Particle earth(glm::vec3(0.0f, 0.0f, 0.0f), 100.0f);
        earth.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        particles.push_back(earth);
        std::cout << "✓ Central Body (Earth):" << std::endl;
        std::cout << "  - Mass: " << earth.mass << std::endl;
        std::cout << "  - Position: (0, 0, 0)" << std::endl;
        
        // Vật thể 2: Chọn loại
        std::cout << "\nSelect orbiting object:" << std::endl;
        std::cout << "1. Moon (natural satellite)" << std::endl;
        std::cout << "2. Artificial Satellite" << std::endl;
        std::cout << "3. Asteroid" << std::endl;
        std::cout << "4. Custom Object" << std::endl;
        std::cout << "Enter choice (1-4): ";
        
        int choice;
        std::cin >> choice;
        
        Particle orbitingObject;
        float distance;
        std::string objectName;
        
        switch (choice)
        {
            case 1: // Moon
                objectName = "Moon";
                distance = 50.0f;
                orbitingObject = Particle(glm::vec3(distance, 0.0f, 0.0f), 1.0f);
                std::cout << "\n✓ Moon selected" << std::endl;
                break;
                
            case 2: // Satellite
                objectName = "Satellite";
                distance = 10.0f;
                orbitingObject = Particle(glm::vec3(distance, 0.0f, 0.0f), 0.001f);
                std::cout << "\n✓ Artificial Satellite selected" << std::endl;
                break;
                
            case 3: // Asteroid
                objectName = "Asteroid";
                distance = 30.0f;
                orbitingObject = Particle(glm::vec3(distance, 0.0f, 0.0f), 0.1f);
                std::cout << "\n✓ Asteroid selected" << std::endl;
                break;
                
            case 4: // Custom
            {
                objectName = "Custom Object";
                std::cout << "\n--- Custom Object Configuration ---" << std::endl;
                std::cout << "Enter distance from Earth: ";
                std::cin >> distance;
                
                float customMass;
                std::cout << "Enter mass: ";
                std::cin >> customMass;
                
                orbitingObject = Particle(glm::vec3(distance, 0.0f, 0.0f), customMass);
                std::cout << "✓ Custom object created" << std::endl;
                break;
            }
                
            default:
                std::cout << "\nInvalid choice! Using Moon as default." << std::endl;
                objectName = "Moon";
                distance = 50.0f;
                orbitingObject = Particle(glm::vec3(distance, 0.0f, 0.0f), 1.0f);
        }
        
        // Tính vận tốc quỹ đạo
        float G = 1.0f;
        float orbitalSpeed = std::sqrt(G * earth.mass / distance);
        orbitingObject.velocity = glm::vec3(0.0f, orbitalSpeed, 0.0f);
        
        particles.push_back(orbitingObject);
        
        std::cout << "\n✓ Orbiting Object (" << objectName << "):" << std::endl;
        std::cout << "  - Mass: " << orbitingObject.mass << std::endl;
        std::cout << "  - Distance: " << distance << std::endl;
        std::cout << "  - Orbital speed: " << orbitalSpeed << std::endl;
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "Generated: " << particles.size() << " bodies" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return particles;
    }

    // ========================================
    // SCENARIO 2: HỆ MẶT TRỜI
    // ========================================
    std::vector<Particle> generateSolarSystemScenario()
    {
        std::vector<Particle> particles;
        
        std::cout << "\n=== Generating Solar System ===" << std::endl;
        
        // Mặt Trời
        Particle sun(glm::vec3(0.0f, 0.0f, 0.0f), 1000.0f);
        sun.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        particles.push_back(sun);
        std::cout << "  - Sun: mass=" << sun.mass << std::endl;
        
        // Dữ liệu hành tinh
        struct PlanetData {
            std::string name;
            float distance;
            float mass;
        };
        
        std::vector<PlanetData> planets = {
            {"Mercury", 10.0f,  0.1f},
            {"Venus",   15.0f,  0.5f},
            {"Earth",   20.0f,  1.0f},
            {"Mars",    25.0f,  0.3f},
            {"Jupiter", 40.0f,  20.0f},
            {"Saturn",  50.0f,  15.0f},
            {"Uranus",  60.0f,  8.0f},
            {"Neptune", 70.0f,  10.0f}
        };
        
        float G = 1.0f;
        
        for (const auto& planetData : planets)
        {
            glm::vec3 position(planetData.distance, 0.0f, 0.0f);
            Particle planet(position, planetData.mass);
            
            float v = std::sqrt(G * sun.mass / planetData.distance);
            planet.velocity = glm::vec3(0.0f, v, 0.0f);
            
            particles.push_back(planet);
            
            std::cout << "  - " << planetData.name 
                      << ": distance=" << planetData.distance 
                      << ", mass=" << planetData.mass 
                      << ", speed=" << v << std::endl;
        }
        
        std::cout << "Generated: " << particles.size() << " bodies" << std::endl;
        
        return particles;
    }

    // ========================================
    // SCENARIO 3: THIÊN HÀ
    // ========================================
    std::vector<Particle> generateGalaxyScenario()
    {
        std::cout << "\n=== Generating Galaxy ===" << std::endl;
        
        GalaxyConfig config;
        config.totalParticles = 10000;
        config.galaxyRadius = 100.0f;
        config.coreRadius = 20.0f;
        config.coreMassFraction = 0.3f;
        config.minMass = 0.1f;
        config.maxMass = 10.0f;
        config.maxVelocity = 5.0f;
        
        std::vector<Particle> particles = generateGalaxy(config);
        
        std::cout << "Generated: " << particles.size() << " particles" << std::endl;
        
        return particles;
    }

    // ========================================
    // SCENARIO 4: CUSTOM (TỰ THIẾT KẾ)
    // ========================================
    CustomScenarioConfig createCustomScenario()
    {
        CustomScenarioConfig config;
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "      CREATE CUSTOM SCENARIO" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // Đặt tên
        std::cout << "Enter scenario name: ";
        std::cin.ignore();
        std::getline(std::cin, config.name);
        
        // Mô tả
        std::cout << "Enter description (optional, press Enter to skip): ";
        std::getline(std::cin, config.description);
        
        // Số lượng bodies
        int numBodies;
        std::cout << "How many bodies/particles? ";
        std::cin >> numBodies;
        
        if (numBodies <= 0) {
            std::cout << "Invalid number! Defaulting to 1." << std::endl;
            numBodies = 1;
        }
        
        // Thêm từng body
        std::cout << "\n--- Adding bodies ---" << std::endl;
        
        for (int i = 0; i < numBodies; ++i)
        {
            std::cout << "\nBody #" << (i + 1) << ":" << std::endl;
            
            float x, y, z;
            std::cout << "  Position (x y z): ";
            std::cin >> x >> y >> z;
            glm::vec3 position(x, y, z);
            
            float mass;
            std::cout << "  Mass: ";
            std::cin >> mass;
            
            float vx, vy, vz;
            std::cout << "  Velocity (vx vy vz): ";
            std::cin >> vx >> vy >> vz;
            glm::vec3 velocity(vx, vy, vz);
            
            Particle p(position, mass);
            p.velocity = velocity;
            
            config.particles.push_back(p);
            
            std::cout << "  ✓ Body #" << (i + 1) << " added!" << std::endl;
        }
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "✓ Custom scenario created: \"" << config.name << "\"" << std::endl;
        std::cout << "  Total bodies: " << config.particles.size() << std::endl;
        std::cout << "========================================" << std::endl;
        
        return config;
    }
    
    std::vector<Particle> generateCustomScenario(const CustomScenarioConfig& config)
    {
        std::cout << "\n=== Generating Custom Scenario: " << config.name << " ===" << std::endl;
        
        if (!config.description.empty()) {
            std::cout << "Description: " << config.description << std::endl;
        }
        
        std::cout << "Bodies: " << config.particles.size() << std::endl;
        
        return config.particles;
    }

    // ========================================
    // MANAGER
    // ========================================
    std::vector<Particle> generateScenario(ScenarioType type, CustomScenarioConfig* customConfig)
    {
        switch (type)
        {
            case ScenarioType::TWO_BODY:
                return generateTwoBodyScenario();
                
            case ScenarioType::SOLAR_SYSTEM:
                return generateSolarSystemScenario();
                
            case ScenarioType::GALAXY:
                return generateGalaxyScenario();
                
            case ScenarioType::CUSTOM:
                if (customConfig != nullptr) {
                    return generateCustomScenario(*customConfig);
                } else {
                    std::cout << "Error: Custom config is null!" << std::endl;
                    return std::vector<Particle>();
                }
                
            default:
                return generateGalaxyScenario();
        }
    }
    
    std::vector<Particle> generateScenario(ScenarioType type)
    {
        return generateScenario(type, nullptr);
    }

    // ========================================
    // UI FUNCTIONS
    // ========================================
    std::string getScenarioName(ScenarioType type)
    {
        switch (type)
        {
            case ScenarioType::TWO_BODY:     return "Two-Body System";
            case ScenarioType::SOLAR_SYSTEM: return "Solar System";
            case ScenarioType::GALAXY:       return "Galaxy";
            case ScenarioType::CUSTOM:       return "Custom Scenario";
            default:                         return "Unknown";
        }
    }
    
    ScenarioType showScenarioMenu()
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "      SELECT SIMULATION SCENARIO" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "1. Two-Body System (2 bodies)" << std::endl;
        std::cout << "2. Solar System (9 bodies)" << std::endl;
        std::cout << "3. Galaxy (10,000 particles)" << std::endl;
        std::cout << "4. Custom Scenario (You design it!)" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Enter choice (1-4): ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice)
        {
            case 1: return ScenarioType::TWO_BODY;
            case 2: return ScenarioType::SOLAR_SYSTEM;
            case 3: return ScenarioType::GALAXY;
            case 4: return ScenarioType::CUSTOM;
            default:
                std::cout << "Invalid choice! Defaulting to Galaxy." << std::endl;
                return ScenarioType::GALAXY;
        }
    }
    
    bool askToChangeScenario()
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Change scenario? (y/n): ";
        
        char choice;
        std::cin >> choice;
        
        return (choice == 'y' || choice == 'Y');
    }
    
    void printScenarioInfo(const std::vector<Particle>& particles, ScenarioType type)
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "  " << getScenarioName(type) << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Total particles: " << particles.size() << std::endl;
        
        glm::vec3 com = calculateCenterOfMass(particles);
        std::cout << "Center of mass: (" 
                  << com.x << ", " 
                  << com.y << ", " 
                  << com.z << ")" << std::endl;
        
        float totalMass = 0.0f;
        for (const Particle& p : particles) {
            totalMass += p.mass;
        }
        std::cout << "Total mass: " << totalMass << std::endl;
        
        std::cout << "========================================" << std::endl;
    }
    
    void printCustomScenarioInfo(const CustomScenarioConfig& config)
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "  CUSTOM: " << config.name << std::endl;
        std::cout << "========================================" << std::endl;
        
        if (!config.description.empty()) {
            std::cout << "Description: " << config.description << std::endl;
        }
        
        std::cout << "Total bodies: " << config.particles.size() << std::endl;
        
        glm::vec3 com = calculateCenterOfMass(config.particles);
        std::cout << "Center of mass: (" 
                  << com.x << ", " 
                  << com.y << ", " 
                  << com.z << ")" << std::endl;
        
        float totalMass = 0.0f;
        for (const Particle& p : config.particles) {
            totalMass += p.mass;
        }
        std::cout << "Total mass: " << totalMass << std::endl;
        
        std::cout << "\nBodies:" << std::endl;
        for (size_t i = 0; i < config.particles.size(); ++i) {
            const Particle& p = config.particles[i];
            std::cout << "  #" << (i + 1) 
                      << ": pos=(" << p.position.x << "," << p.position.y << "," << p.position.z << ")"
                      << ", mass=" << p.mass
                      << ", vel=(" << p.velocity.x << "," << p.velocity.y << "," << p.velocity.z << ")"
                      << std::endl;
        }
        
        std::cout << "========================================" << std::endl;
    }

    // ========================================
    // SAVE FUNCTION
    // ========================================
    void saveCustomScenario(const CustomScenarioConfig& config, const std::string& filename)
    {
        std::ofstream file(filename);
        
        if (!file.is_open()) {
            std::cout << "❌ Error: Cannot create file!" << std::endl;
            return;
        }
        
        file << "# Custom N-Body Scenario" << std::endl;
        file << "# Generated by N-Body Simulation" << std::endl;
        file << std::endl;
        
        file << "NAME: " << config.name << std::endl;
        
        if (!config.description.empty()) {
            file << "DESCRIPTION: " << config.description << std::endl;
        }
        file << std::endl;
        
        file << "BODIES: " << config.particles.size() << std::endl;
        file << std::endl;
        
        file << "# Format: x y z mass vx vy vz" << std::endl;
        
        for (size_t i = 0; i < config.particles.size(); ++i) {
            const Particle& p = config.particles[i];
            
            file << "BODY_" << (i + 1) << ": "
                 << p.position.x << " " 
                 << p.position.y << " " 
                 << p.position.z << " "
                 << p.mass << " "
                 << p.velocity.x << " " 
                 << p.velocity.y << " " 
                 << p.velocity.z << std::endl;
        }
        
        file.close();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "✓ Scenario saved successfully!" << std::endl;
        std::cout << "  File: " << filename << std::endl;
        std::cout << "========================================" << std::endl;
    }
    
    bool askToSaveScenario()
    {
        std::cout << "\nDo you want to save this custom scenario? (y/n): ";
        
        char choice;
        std::cin >> choice;
        
        return (choice == 'y' || choice == 'Y');
    }

    // ========================================
    // DEMO FUNCTION 
    // ========================================
    void runScenarioDemo()
    {
        std::cout << "========================================" << std::endl;
        std::cout << "    N-BODY SIMULATION DEMO" << std::endl;
        std::cout << "========================================" << std::endl;
        
        bool running = true;
        CustomScenarioConfig customConfig;
        
        while (running)
        {
            // Hiển thị menu
            ScenarioType selectedScenario = showScenarioMenu();
            
            std::vector<Particle> particles;
            
            // Xử lý custom scenario
            if (selectedScenario == ScenarioType::CUSTOM)
            {
                customConfig = createCustomScenario();
                particles = generateScenario(selectedScenario, &customConfig);
                printCustomScenarioInfo(customConfig);
                
                // Hỏi có muốn lưu không
                if (askToSaveScenario())
                {
                    std::string filename;
                    std::cout << "Enter filename (e.g., my_scenario.txt): ";
                    std::cin >> filename;
                    
                    saveCustomScenario(customConfig, filename);
                }
            }
            else
            {
                // Scenarios có sẵn
                particles = generateScenario(selectedScenario);
                printScenarioInfo(particles, selectedScenario);
            }
            
            // Hiển thị thông báo
            std::cout << "\n--- Particle Data Generated ---" << std::endl;
            std::cout << "You now have " << particles.size() << " particles ready." << std::endl;
            std::cout << "This data can be used by Physics/Graphics modules later." << std::endl;
            
            // Hỏi có muốn đổi scenario không
            if (!askToChangeScenario())
            {
                running = false;
            }
        }
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "  Exiting demo. Goodbye!" << std::endl;
        std::cout << "========================================" << std::endl;
    }
}