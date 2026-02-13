#include "Camera.hpp"

namespace Graphics
{
    CameraWrapper::CameraWrapper()
    {
        camera.position = {0.0f, 200.0f, 400.0f}; // Vị trí ban đầu nhìn từ trên cao xuống
        camera.target = {0.0f, 0.0f, 0.0f};       // Nhìn vào tâm thiên hà
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;
    }

    void CameraWrapper::Update()
    {
        // Sử dụng chế độ camera orbital mặc định của Raylib
        // Giữ chuột phải để xoay, lăn chuột để zoom
        UpdateCamera(&camera, CAMERA_ORBITAL);
    }
}