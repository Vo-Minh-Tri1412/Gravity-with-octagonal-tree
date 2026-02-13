#pragma once
#include <raylib.h>

namespace Graphics
{
    class CameraWrapper
    {
    public:
        CameraWrapper();
        void Update();
        Camera3D &GetRaylibCamera() { return camera; }

    private:
        Camera3D camera;
    };
}