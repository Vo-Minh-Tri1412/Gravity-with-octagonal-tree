#include "Renderer.hpp"

namespace Graphics
{
    void Renderer::Init(int width, int height, const char *title)
    {
        InitWindow(width, height, title);
        SetTargetFPS(60);
    }

    void Renderer::Close()
    {
        CloseWindow();
    }

    bool Renderer::ShouldClose() const
    {
        return WindowShouldClose();
    }

    void Renderer::BeginScene(CameraWrapper &camera)
    {
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera.GetRaylibCamera());

        // Vẽ lưới trục tọa độ để dễ nhìn
        DrawGrid(100, 10.0f);
    }

    void Renderer::EndScene()
    {
        EndMode3D();
        EndDrawing();
    }

    void Renderer::RenderParticles(const std::vector<Core::Particle> &particles)
    {
        // Vẽ các hạt dưới dạng điểm (pixel) để tối ưu hiệu năng
        for (const auto &p : particles)
        {
            Vector3 pos = {p.position.x, p.position.y, p.position.z};

            // Màu sắc dựa trên vận tốc hoặc khối lượng (ở đây để màu trắng/vàng nhạt)
            Color color = (p.mass > 50.0f) ? YELLOW : WHITE;
            if (p.mass > 50.0f)
                DrawPoint3D(pos, RED); // Hạt nhân nặng màu đỏ
            else
                DrawPoint3D(pos, color);
        }
    }

    void Renderer::RenderUI(int fps, size_t particleCount, const std::string &solverName, float physicsTime)
    {
        DrawFPS(10, 10);
        DrawText(TextFormat("Particles: %zu", particleCount), 10, 40, 20, GREEN);
        DrawText(TextFormat("Solver: %s", solverName.c_str()), 10, 70, 20, SKYBLUE);
        DrawText(TextFormat("Physics Time: %.2f ms", physicsTime), 10, 100, 20, ORANGE);
        DrawText("Controls: [R] Reset | [B] BruteForce | [H] Barnes-Hut", 10, GetScreenHeight() - 30, 20, GRAY);
    }
}