#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Physics.hpp"

struct ParticleData
{
    glm::vec3 position;
    glm::vec4 color;
    float size;
};

class Renderer
{
public:
    void init();
    void render(const std::vector<Body> &bodies, const glm::mat4 &view, const glm::mat4 &proj, float time);

private:
    unsigned int m_vao, m_vbo, m_instanceVBO;
    unsigned int m_shaderProgram;   // Particles
    unsigned int m_bhShaderProgram; // Black Hole

    unsigned int createShader(const char *vertexSrc, const char *fragmentSrc);

    // Helper to draw the black hole
    void renderBlackHole(const glm::mat4 &view, const glm::mat4 &proj, float time);
};