#include "Camera.hpp"
#include <cmath>

namespace Graphics
{
    void CameraWrapper::rebuildPosition()
    {
        camera.position = {
            m_target.x + m_radius * cosf(m_elevation) * sinf(m_azimuth),
            m_target.y + m_radius * sinf(m_elevation),
            m_target.z + m_radius * cosf(m_elevation) * cosf(m_azimuth)};
        camera.target = m_target;
    }

    CameraWrapper::CameraWrapper()
    {
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;
        rebuildPosition();
    }

    void CameraWrapper::Reset(Vector3 target, float radius, float azimuth, float elevation)
    {
        m_target = target;
        m_radius = radius;
        m_azimuth = azimuth;
        m_elevation = elevation;
        rebuildPosition();
    }

    void CameraWrapper::Update()
    {
        // ---- Zoom: lăn chuột ----
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
        {
            m_radius -= wheel * m_radius * 0.08f;
            if (m_radius < 5.0f)
                m_radius = 5.0f;
            if (m_radius > 3000.0f)
                m_radius = 3000.0f;
        }

        // ---- Orbit: giữ chuột PHẢI + kéo ----
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            m_lastMouse = mouse;
            m_dragging = true;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
            m_dragging = false;

        if (m_dragging)
        {
            float dx = (mouse.x - m_lastMouse.x) * 0.005f;
            float dy = (mouse.y - m_lastMouse.y) * 0.005f;
            m_azimuth -= dx;
            m_elevation -= dy; // kéo xuống = nhìn từ cao hơn
            if (m_elevation > 1.55f)
                m_elevation = 1.55f;
            if (m_elevation < -1.55f)
                m_elevation = -1.55f;
            m_lastMouse = mouse;
        }

        rebuildPosition();
    }
}