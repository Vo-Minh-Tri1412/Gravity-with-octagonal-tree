#pragma once
#include <vector>
#include <string>
#include <raylib.h>
#include "../Core/Particle.hpp"
#include "Camera.hpp"

namespace Graphics
{
    class Renderer
    {
    public:
        void Init(int width, int height, const char *title);
        void Close();
        bool ShouldClose() const;

        void BeginScene(CameraWrapper &camera);
        void EndScene();

        void RenderParticles(const std::vector<Core::Particle> &particles);
        void RenderUI(int fps, size_t particleCount, const std::string &solverName, float physicsTime);
    };
}