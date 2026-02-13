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
    // 1. Cấu hình & Khởi tạo
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    Graphics::Renderer renderer;
    renderer.Init(SCREEN_WIDTH, SCREEN_HEIGHT, "Gravity Simulation - Barnes-Hut Octree");

    Graphics::CameraWrapper camera;

    // 2. Dữ liệu (Data)
    Utils::GalaxyConfig galaxyConfig;
    galaxyConfig.totalParticles = 4000; // Số lượng hạt

    std::vector<Core::Particle> particles = Utils::generateGalaxy(galaxyConfig);

    // 3. Hệ thống Vật lý (Physics System)
    // Mặc định dùng Barnes-Hut
    std::unique_ptr<Physics::ISolver> solver = std::make_unique<Physics::BarnesHutSolver>();
    std::string currentSolverName = "Barnes-Hut (O(N log N))";

    Utils::Timer physicsTimer;
    const float dt = 0.016f; // Delta time cố định (60 FPS)

    // 4. Vòng lặp chính (Main Loop)
    while (!renderer.ShouldClose())
    {
        // --- INPUT ---
        camera.Update();

        // [R] Reset thiên hà
        if (IsKeyPressed(KEY_R))
        {
            particles = Utils::generateGalaxy(galaxyConfig);
        }

        // [B] Chuyển sang Brute Force
        if (IsKeyPressed(KEY_B))
        {
            solver = std::make_unique<Physics::BruteForceSolver>();
            currentSolverName = "Brute Force (O(N^2))";
        }

        // [H] Chuyển sang Barnes-Hut
        if (IsKeyPressed(KEY_H))
        {
            solver = std::make_unique<Physics::BarnesHutSolver>();
            currentSolverName = "Barnes-Hut (O(N log N))";
        }

        // --- PHYSICS UPDATE ---
        physicsTimer.Start();
        if (solver)
        {
            solver->solve(particles, dt);
        }
        physicsTimer.Stop();

        // --- RENDER ---
        renderer.BeginScene(camera);
        renderer.RenderParticles(particles);
        renderer.EndScene();

        renderer.RenderUI(GetFPS(), particles.size(), currentSolverName, physicsTimer.GetElapsedMS());
    }

    renderer.Close();
    return 0;
}
