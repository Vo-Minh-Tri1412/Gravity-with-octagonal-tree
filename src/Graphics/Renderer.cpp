#include "Renderer.hpp"
#include <cmath>
#include <algorithm>

namespace Graphics
{
    // =================================================================
    // Màu hạt thiên hà theo khối lượng — giống màu sao thực tế:
    //   thấp  → đỏ/cam  (lùn đỏ M-type)
    //   trung → vàng-trắng (mặt trời G-type)
    //   cao   → xanh lạnh  (khổng lồ O/B-type)
    // =================================================================
    static Color massToColor(float mass, float minMass, float maxMass)
    {
        float t = (mass - minMass) / (maxMass - minMass + 1e-6f);
        // Gamma: làm sáng hạt nhỏ để luôn nhìn thấy được
        t = std::sqrt(std::min(std::max(t, 0.0f), 1.0f));

        Color c0 = {200, 80, 40, 255};   // lùn đỏ
        Color c1 = {240, 220, 150, 255}; // loại mặt trời
        Color c2 = {160, 185, 255, 255}; // khổng lồ xanh

        Color a, b;
        float u;
        if (t < 0.5f)
        {
            a = c0;
            b = c1;
            u = t / 0.5f;
        }
        else
        {
            a = c1;
            b = c2;
            u = (t - 0.5f) / 0.5f;
        }

        return {
            (unsigned char)(a.r + u * (b.r - a.r)),
            (unsigned char)(a.g + u * (b.g - a.g)),
            (unsigned char)(a.b + u * (b.b - a.b)),
            255};
    }

    // Màu hành tinh theo khối lượng
    static Color planetColor(float mass)
    {
        if (mass < 0.5f)
            return {175, 165, 155, 255}; // đất đá (Mercury, Mars)
        if (mass < 5.0f)
            return {70, 135, 210, 255}; // loại Trái Đất
        return {225, 185, 110, 255};    // khí khổng lồ (Jupiter, Saturn)
    }

    // =================================================================
    void Renderer::Init(int width, int height, const char *title)
    {
        InitWindow(width, height, title);
        SetTargetFPS(60);
    }

    void Renderer::Close() { CloseWindow(); }
    bool Renderer::ShouldClose() const { return WindowShouldClose(); }

    void Renderer::BeginScene(CameraWrapper &camera)
    {
        BeginDrawing();
        ClearBackground({5, 5, 15, 255}); // nền xanh đen vũ trụ
        BeginMode3D(camera.GetRaylibCamera());
    }

    void Renderer::EndScene()
    {
        EndMode3D();
        // KHÔNG gọi EndDrawing ở đây — RenderUI cần vẽ 2D vào cùng frame.
        // EndDrawing() được gọi trong EndFrame() bên dưới.
    }

    void Renderer::EndFrame()
    {
        EndDrawing();
    }

    void Renderer::RenderParticles(const std::vector<Core::Particle> &particles)
    {
        // Tính min/max mass của hạt GENERIC để normalize màu
        float minM = 1e9f, maxM = 0.0f;
        for (const auto &p : particles)
        {
            if (p.type == Core::ParticleType::GENERIC)
            {
                if (p.mass < minM)
                    minM = p.mass;
                if (p.mass > maxM)
                    maxM = p.mass;
            }
        }
        if (minM > maxM)
        {
            minM = 1.0f;
            maxM = 100.0f;
        }

        for (const auto &p : particles)
        {
            // Bỏ qua hạt NaN/Inf — tránh crash OpenGL driver
            if (!std::isfinite(p.position.x) ||
                !std::isfinite(p.position.y) ||
                !std::isfinite(p.position.z))
                continue;

            Vector3 pos = {p.position.x, p.position.y, p.position.z};

            switch (p.type)
            {
            // ---- Ngôi sao / Mặt Trời / Trái Đất (STAR) ----
            case Core::ParticleType::STAR:
            {
                float r = p.radius > 0.0f ? p.radius : std::cbrt(p.mass) * 0.8f;
                if (r < 3.0f)
                    r = 3.0f;
                DrawSphere(pos, r, {255, 245, 140, 255});
                DrawSphereWires(pos, r * 1.55f, 8, 8, {255, 160, 20, 30});
                break;
            }
            // ---- Hành tinh (PLANET) ----
            case Core::ParticleType::PLANET:
            {
                float r = p.radius > 0.0f ? p.radius : std::cbrt(p.mass) * 1.2f;
                if (r < 0.8f)
                    r = 0.8f;
                DrawSphere(pos, r, planetColor(p.mass));
                break;
            }
            // ---- Mặt Trăng (MOON) ----
            case Core::ParticleType::MOON:
            {
                float r = p.radius > 0.0f ? p.radius : std::cbrt(p.mass) * 0.7f;
                if (r < 0.4f)
                    r = 0.4f;
                DrawSphere(pos, r, {190, 190, 200, 255});
                break;
            }
            // ---- Tiểu hành tinh (ASTEROID) ----
            case Core::ParticleType::ASTEROID:
                DrawPoint3D(pos, {120, 110, 100, 255});
                break;

            // ---- Hạt thiên hà (GENERIC) ----
            default:
                DrawPoint3D(pos, massToColor(p.mass, minM, maxM));
                break;
            }
        }
    }

    void Renderer::RenderUI(int fps, size_t particleCount,
                            const std::string &scenarioName,
                            const std::string &solverName,
                            float physicsTime)
    {
        DrawFPS(10, 10);
        DrawText(TextFormat("Scenario : %s", scenarioName.c_str()), 10, 40, 20, YELLOW);
        DrawText(TextFormat("Solver   : %s", solverName.c_str()), 10, 70, 20, SKYBLUE);
        DrawText(TextFormat("Particles: %zu", particleCount), 10, 100, 20, GREEN);
        DrawText(TextFormat("Physics  : %.2f ms", physicsTime), 10, 130, 20, ORANGE);
        DrawText("[1] Galaxy  [2] Solar System  [3] Two-Body  |  [R] Reset  |  [B] BruteForce  [H] Barnes-Hut",
                 10, GetScreenHeight() - 30, 17, GRAY);
    }
}