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

    // ===== Cấu hình thí nghiệm vận tốc vũ trụ =====
    Utils::TwoBodyConfig twoBodyCfg;

    // ===== Kịch bản mặc định: Thiên Hà =====
    Utils::ScenarioType currentScenario = Utils::ScenarioType::GALAXY;
    Utils::GalaxyConfig galaxyConfig;
    std::vector<Core::Particle> particles = Utils::generateGalaxy(galaxyConfig);

    camera.Reset({0, 0, 0}, 450.0f);

    std::unique_ptr<Physics::ISolver> solver = std::make_unique<Physics::BarnesHutSolver>();
    std::string currentSolverName = "Barnes-Hut (O(N log N))";

    Utils::Timer physicsTimer;
    const float dt = 0.016f;

    // ===== Trạng thái thí nghiệm TWO_BODY =====
    float simTime = 0.0f;
    enum class OrbitStatus
    {
        BOUND,
        ESCAPED,
        CRASHED
    } orbitStatus = OrbitStatus::BOUND;

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
            camera.Reset({0, 0, 0}, 450.0f);
        }
        // [2] Hệ Mặt Trời
        if (IsKeyPressed(KEY_TWO))
        {
            currentScenario = Utils::ScenarioType::SOLAR_SYSTEM;
            particles = Utils::generateSolarSystem();
            solver = std::make_unique<Physics::BruteForceSolver>();
            currentSolverName = "Brute Force (O(N²))";
            camera.Reset({0, 0, 0}, 300.0f);
        }
        // [3] Thí nghiệm vận tốc vũ trụ
        if (IsKeyPressed(KEY_THREE))
        {
            currentScenario = Utils::ScenarioType::TWO_BODY;
            particles = Utils::generateTwoBody(twoBodyCfg);
            solver = std::make_unique<Physics::BruteForceSolver>();
            currentSolverName = "Brute Force (O(N\u00b2))";
            simTime = 0.0f;
            orbitStatus = OrbitStatus::BOUND;
            camera.Reset({0, 0, 0}, 80.0f, 0.0f, 0.3f);
        }
        // [R] Reset kịch bản hiện tại
        if (IsKeyPressed(KEY_R))
        {
            simTime = 0.0f;
            orbitStatus = OrbitStatus::BOUND;
            switch (currentScenario)
            {
            case Utils::ScenarioType::GALAXY:
                particles = Utils::generateGalaxy(galaxyConfig);
                break;
            case Utils::ScenarioType::SOLAR_SYSTEM:
                particles = Utils::generateSolarSystem();
                break;
            case Utils::ScenarioType::TWO_BODY:
                particles = Utils::generateTwoBody(twoBodyCfg);
                break;
            }
        }

        // ===== Điều chỉnh thí nghiệm (chỉ khi đang ở TWO_BODY) =====
        if (currentScenario == Utils::ScenarioType::TWO_BODY)
        {
            bool changed = false;
            // [UP]/[DOWN]: tăng/giảm tốc độ phóng ±0.1
            if (IsKeyPressed(KEY_UP))
            {
                twoBodyCfg.launchSpeed += 0.1f;
                changed = true;
            }
            if (IsKeyPressed(KEY_DOWN))
            {
                twoBodyCfg.launchSpeed -= 0.1f;
                changed = true;
            }
            if (twoBodyCfg.launchSpeed < 0.0f)
                twoBodyCfg.launchSpeed = 0.0f;
            // [LEFT]/[RIGHT]: xoay góc phóng ±5°
            if (IsKeyPressed(KEY_RIGHT))
            {
                twoBodyCfg.launchAngleDeg += 5.0f;
                changed = true;
            }
            if (IsKeyPressed(KEY_LEFT))
            {
                twoBodyCfg.launchAngleDeg -= 5.0f;
                changed = true;
            }
            if (changed)
            {
                particles = Utils::generateTwoBody(twoBodyCfg);
                simTime = 0.0f;
                orbitStatus = OrbitStatus::BOUND;
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

        // ===== Cập nhật trạng thái TWO_BODY =====
        if (currentScenario == Utils::ScenarioType::TWO_BODY && particles.size() >= 2)
        {
            simTime += dt;
            const glm::vec3 &rPos = particles[1].position;
            float dist = std::sqrt(rPos.x * rPos.x + rPos.y * rPos.y + rPos.z * rPos.z);
            if (dist < twoBodyCfg.earthRadius * 0.95f)
                orbitStatus = OrbitStatus::CRASHED;
            else if (dist > 800.0f)
                orbitStatus = OrbitStatus::ESCAPED;
            else
                orbitStatus = OrbitStatus::BOUND;
        }

        // --- RENDER ---
        renderer.BeginScene(camera); // BeginDrawing + BeginMode3D

        // Vẽ wireframe bề mặt Trái Đất khi ở chế độ TWO_BODY
        if (currentScenario == Utils::ScenarioType::TWO_BODY)
            DrawSphereWires({0, 0, 0}, twoBodyCfg.earthRadius, 12, 12, {50, 100, 60, 120});

        renderer.RenderParticles(particles);
        renderer.EndScene(); // EndMode3D (BeginDrawing vẫn còn mở)

        // ===== HUD thí nghiệm vận tốc vũ trụ =====
        if (currentScenario == Utils::ScenarioType::TWO_BODY)
        {
            float vEsc = Utils::getEscapeVelocity(twoBodyCfg);
            float vCirc = Utils::getCircularOrbitalSpeed(twoBodyCfg);
            Color statusColor;
            const char *statusStr;
            if (orbitStatus == OrbitStatus::ESCAPED)
            {
                statusStr = "THOAT KHOI";
                statusColor = RED;
            }
            else if (orbitStatus == OrbitStatus::CRASHED)
            {
                statusStr = "VA CHAM";
                statusColor = ORANGE;
            }
            else
            {
                statusStr = "QUY DAO";
                statusColor = LIME;
            }

            DrawText(TextFormat("v_launch = %.2f  |  v_escape = %.2f  |  v_circular = %.2f",
                                twoBodyCfg.launchSpeed, vEsc, vCirc),
                     10, 165, 18, LIME);
            DrawText(TextFormat("Goc phong = %.1f deg  |  t_sim = %.1f s  |  Trang thai: %s",
                                twoBodyCfg.launchAngleDeg, simTime, statusStr),
                     10, 190, 18, statusColor);
            DrawText("[UP/DOWN] Toc do  |  [LEFT/RIGHT] Goc phong  |  [R] Phong lai",
                     10, GetScreenHeight() - 55, 17, GRAY);
        }

        renderer.RenderUI(GetFPS(), particles.size(),
                          Utils::getScenarioName(currentScenario),
                          currentSolverName,
                          physicsTimer.GetElapsedMS());
        renderer.EndFrame(); // EndDrawing — trình bày frame
    }

    renderer.Close();
    return 0;
}
