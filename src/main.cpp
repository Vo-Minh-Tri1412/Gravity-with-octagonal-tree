#include <iostream>
#include <raylib.h>
#include <glm/glm.hpp>

// #include "Physics.hpp"
// #include "Octree.hpp"
// #include "Renderer.hpp"
// #include "Galaxy.hpp"

int main()
{
    // 1. Test GLM: Tạo một vector vị trí
    glm::vec2 position(400.0f, 300.0f);
    std::cout << "Testing GLM: Vector created at (" << position.x << ", " << position.y << ")" << std::endl;

    // 2. Test Raylib: Mở cửa sổ và vẽ
    InitWindow(800, 600, "Test Raylib & GLM Integration");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Congrats! Raylib and GLM are working!", 190, 200, 20, LIGHTGRAY);
        DrawCircle((int)position.x, (int)position.y, 50.0f, MAROON);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
