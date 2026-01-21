#include "raylib.h"
int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "N-Body Simulation - Barnes-Hut Algorithm");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("N-Body Simulation", 10, 10, 30, WHITE);
        DrawText("Press ESC to exit", 10, 50, 20, GRAY);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}