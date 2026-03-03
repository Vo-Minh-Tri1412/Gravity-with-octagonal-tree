#pragma once

#include "../Core/Particle.hpp"
#include <vector>
#include <string>

namespace Core
{
    // Enum định nghĩa các loại scenario
    enum class ScenarioType
    {
        TWO_BODY,        // Trái Đất + 1 vật thể
        SOLAR_SYSTEM,    // Hệ Mặt Trời
        GALAXY,          // Thiên hà
        CUSTOM           // Tự thiết kế
    };

    // Struct lưu custom scenario
    struct CustomScenarioConfig
    {
        std::string name;
        std::vector<Particle> particles;
        std::string description;
    };

    // Các hàm generate scenarios
    std::vector<Particle> generateTwoBodyScenario();
    std::vector<Particle> generateSolarSystemScenario();
    std::vector<Particle> generateGalaxyScenario();
    CustomScenarioConfig createCustomScenario();
    std::vector<Particle> generateCustomScenario(const CustomScenarioConfig& config);
    
    // Manager
    std::vector<Particle> generateScenario(ScenarioType type);
    std::vector<Particle> generateScenario(ScenarioType type, CustomScenarioConfig* customConfig);
    
    // UI functions
    std::string getScenarioName(ScenarioType type);
    ScenarioType showScenarioMenu();
    bool askToChangeScenario();
    void printScenarioInfo(const std::vector<Particle>& particles, ScenarioType type);
    void printCustomScenarioInfo(const CustomScenarioConfig& config);
    
    // Save function
    void saveCustomScenario(const CustomScenarioConfig& config, const std::string& filename);
    bool askToSaveScenario();
    
    // Demo function
    void runScenarioDemo();
}