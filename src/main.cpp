#include "Graphics/Renderer.hpp"
#include "Graphics/Camera.hpp"
#include "Physics/BarnesHutSolver.hpp"
#include "Physics/BruteForceSolver.hpp"
#include "Utils/GalaxyGenerator.hpp"
#include "Utils/Timer.hpp"

#include <memory>
#include <string>
#include <vector>

int main()
{
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    Graphics::Renderer renderer;
    renderer.Init(SCREEN_WIDTH, SCREEN_HEIGHT, "Gravity Simulation - Barnes-Hut Octree");

    Graphics::CameraWrapper camera;

    // ===== Kịch bản mặc định: Thiên Hà =====
    Utils::ScenarioType currentScenario = Utils::ScenarioType::GALAXY;
    Utils::GalaxyConfig galaxyConfig;
    std::vector<Core::Particle> particles = Utils::generateGalaxy(galaxyConfig);

    std::unique_ptr<Physics::ISolver> solver = std::make_unique<Physics::BarnesHutSolver>();
    std::string currentSolverName = "Barnes-Hut (O(N log N))";

    Utils::Timer physicsTimer;
    const float dt = 0.016f;

    while (!renderer.ShouldClose())
    {
        // --- INPUT ---
        camera.Update();

        // [1] Thiên Hà
        if (IsKeyPressed(KEY_ONE))
        {
            currentScenario = Utils::ScenarioType::GALAXY;
            particles = Utils::generateGalaxy(galaxyConfig);
            solver = std::make_unique<Physics::BarnesHutSolver>();
            currentSolverName = "Barnes-Hut (O(N log N))";
        }
        // [2] Hệ Mặt Trời
        if (IsKeyPressed(KEY_TWO))
        {
            currentScenario = Utils::ScenarioType::SOLAR_SYSTEM;
            particles = Utils::generateSolarSystem();
            solver = std::make_unique<Physics::BruteForceSolver>();
            currentSolverName = "Brute Force (O(N²))";
        }
        // [3] Hệ 2 vật thể
        if (IsKeyPressed(KEY_THREE))
        {
            currentScenario = Utils::ScenarioType::TWO_BODY;
            particles = Utils::generateTwoBody();
            solver = std::make_unique<Physics::BruteForceSolver>();
            currentSolverName = "Brute Force (O(N²))";
        }
        // [R] Reset kịch bản hiện tại
        if (IsKeyPressed(KEY_R))
        {
            switch (currentScenario)
            {
            case Utils::ScenarioType::GALAXY:
                particles = Utils::generateGalaxy(galaxyConfig);
                break;
            case Utils::ScenarioType::SOLAR_SYSTEM:
                particles = Utils::generateSolarSystem();
                break;
            case Utils::ScenarioType::TWO_BODY:
                particles = Utils::generateTwoBody();
                break;
            }
        }
        // [B] / [H] đổi solver thủ công
        if (IsKeyPressed(KEY_B))
        {
            solver = std::make_unique<Physics::BruteForceSolver>();
            currentSolverName = "Brute Force (O(N²))";
        }
        if (IsKeyPressed(KEY_H))
        {
            solver = std::make_unique<Physics::BarnesHutSolver>();
            currentSolverName = "Barnes-Hut (O(N log N))";
        }

        // --- PHYSICS ---
        physicsTimer.Start();
        if (solver)
            solver->solve(particles, dt);
        physicsTimer.Stop();

        // --- RENDER ---
        renderer.BeginScene(camera); // BeginDrawing + BeginMode3D
        renderer.RenderParticles(particles);
        renderer.EndScene(); // EndMode3D (BeginDrawing vẫn còn mở)

        renderer.RenderUI(GetFPS(), particles.size(),
                          Utils::getScenarioName(currentScenario),
                          currentSolverName,
                          physicsTimer.GetElapsedMS());
        renderer.EndFrame(); // EndDrawing — trình bày frame
    }

    renderer.Close();
    return 0;
}
