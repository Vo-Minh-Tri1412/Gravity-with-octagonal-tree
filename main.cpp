#include "raylib.h"
#include <string>
int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Raylib CPP Example");
    SetTargetFPS(60);

    // C++ khởi tạo struct
    Vector2 ballPosition = {(float)screenWidth / 2, (float)screenHeight / 2};

    while (!WindowShouldClose())
    {
        // Update logic
        if (IsKeyDown(KEY_RIGHT))
            ballPosition.x += 2.0f;
        if (IsKeyDown(KEY_LEFT))
            ballPosition.x -= 2.0f;
        if (IsKeyDown(KEY_UP))
            ballPosition.y -= 2.0f;
        if (IsKeyDown(KEY_DOWN))
            ballPosition.y += 2.0f;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Dùng thử std::string của C++ rồi chuyển sang C-string cho Raylib đọc
        std::string text = "Code C++ voi Raylib!";
        DrawText(text.c_str(), 10, 10, 20, DARKGRAY);

        DrawCircleV(ballPosition, 50, MAROON);
        DrawFPS(10, 40);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}