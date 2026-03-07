#pragma once
#include <raylib.h>

namespace Graphics
{
    class CameraWrapper
    {
    public:
        CameraWrapper();
        void Update();
        // Gọi khi chuyển kịch bản — đặt lại góc nhìn
        void Reset(Vector3 target, float radius,
                   float azimuth = 0.0f, float elevation = 0.3f);
        Camera3D &GetRaylibCamera() { return camera; }

    private:
        Camera3D camera;
        Vector3 m_target = {0, 0, 0};
        float m_radius = 450.0f;
        float m_azimuth = 0.0f;
        float m_elevation = 0.3f;
        Vector2 m_lastMouse = {0, 0};
        bool m_dragging = false;
        void rebuildPosition(); // cập nhật camera.position từ tọa độ cầu
    };
}