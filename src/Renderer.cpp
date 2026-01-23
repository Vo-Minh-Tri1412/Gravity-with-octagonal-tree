#include "Renderer.hpp"
#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// --- BLACK HOLE SHADER ---
// FIX: Compute UVs from position (aPos.xy + 0.5) instead of requiring a missing attribute
const char *bhVertexSrc = R"(
    #version 450 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    out vec2 vUV;
    out vec3 vPos;

    void main() {
        // Map quad range [-0.5, 0.5] to [0, 1] for UVs
        vUV = aPos.xy + 0.5;
        vPos = aPos;
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    }
)";

const char *bhFragSrc = R"(
    #version 450 core
    in vec2 vUV;
    in vec3 vPos;
    out vec4 FragColor;

    uniform float uTime;
    uniform int uType; // 0 = Disk, 1 = Hole

    void main() {
        vec2 center = vec2(0.5);
        float dist = distance(vUV, center);
        
        if (uType == 1) { 
            // EVENT HORIZON
            if (dist > 0.48) discard;
            float rim = smoothstep(0.4, 0.48, dist) * 0.8; // Brighter rim
            FragColor = vec4(vec3(rim), 1.0); 
        } 
        else {
            // ACCRETION DISK
            if (dist > 0.5 || dist < 0.15) discard;
            
            float angle = atan(vUV.y - 0.5, vUV.x - 0.5);
            float spiral = angle + 12.0 * dist - uTime * 3.0;
            float noise = sin(spiral * 15.0) * 0.5 + 0.5;
            
            float brightness = 1.0 - smoothstep(0.15, 0.5, dist);
            brightness = pow(brightness, 1.5); // More uniform brightness
            
            vec3 colorInner = vec3(0.5, 0.9, 1.0); // Bright Cyan
            vec3 colorOuter = vec3(0.8, 0.2, 1.0); // Bright Purple
            
            vec3 finalColor = mix(colorOuter, colorInner, brightness);
            finalColor += vec3(noise * 0.3); // Add noise detail
            
            float alpha = brightness * (0.8 + noise * 0.2);
            FragColor = vec4(finalColor, alpha);
        }
    }
)";

void Renderer::init()
{
    // 1. PARTICLE SHADERS (Brighter & Softer)
    const char *partVertexSrc = R"(
        #version 450 core
        layout (location = 0) in vec3 aQuadPos;    
        layout (location = 1) in vec3 aInstancePos; 
        layout (location = 2) in vec4 aInstanceColor;
        layout (location = 3) in float aInstanceSize; 

        uniform mat4 uView;
        uniform mat4 uProjection;
        uniform float uGlobalScale;

        out vec4 vColor;
        out vec2 vUV;

        void main() {
            vec3 cameraRight = vec3(uView[0][0], uView[1][0], uView[2][0]);
            vec3 cameraUp    = vec3(uView[0][1], uView[1][1], uView[2][1]);
            
            float finalSize = aInstanceSize * uGlobalScale;
            
            vec3 vertexPos = aInstancePos 
                           + cameraRight * aQuadPos.x * finalSize 
                           + cameraUp * aQuadPos.y * finalSize;
                           
            vColor = aInstanceColor;
            vUV = aQuadPos.xy + 0.5; 
            gl_Position = uProjection * uView * vec4(vertexPos, 1.0);
        }
    )";

    const char *partFragSrc = R"(
        #version 450 core
        in vec4 vColor;
        in vec2 vUV;
        out vec4 FragColor;

        void main() {
            vec2 d = vUV - 0.5;
            float dist = length(d);
            if (dist > 0.5) discard;
            
            // Softer glow logic
            float glow = 1.0 - smoothstep(0.0, 0.5, dist);
            glow = pow(glow, 1.5); // Lower power = wider, softer glow
            
            // Increased brightness multiplier
            FragColor = vec4(vColor.rgb, vColor.a * glow * 2.5);
        }
    )";

    m_shaderProgram = createShader(partVertexSrc, partFragSrc);
    m_bhShaderProgram = createShader(bhVertexSrc, bhFragSrc);

    // 2. Setup Particle VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    float quadVertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f};

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // Instance Buffer
    glGenBuffers(1, &m_instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 50000 * sizeof(ParticleData), nullptr, GL_STREAM_DRAW);

    // Attributes
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, color));
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, size));
    glVertexAttribDivisor(3, 1);
}

void Renderer::render(const std::vector<Body> &bodies, const glm::mat4 &view, const glm::mat4 &proj, float time)
{
    // 1. Draw Black Hole first
    renderBlackHole(view, proj, time);

    // 2. Draw Particles
    glUseProgram(m_shaderProgram);

    unsigned int viewLoc = glGetUniformLocation(m_shaderProgram, "uView");
    unsigned int projLoc = glGetUniformLocation(m_shaderProgram, "uProjection");
    unsigned int scaleLoc = glGetUniformLocation(m_shaderProgram, "uGlobalScale");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1f(scaleLoc, 0.15f);

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    // Use glBufferSubData or Map
    ParticleData *ptr = (ParticleData *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (ptr)
    {
#pragma omp parallel for
        for (size_t i = 0; i < bodies.size(); ++i)
        {
            ptr[i].position = bodies[i].position;
            ptr[i].color = bodies[i].originalColor;
            ptr[i].size = bodies[i].size;
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    glBindVertexArray(m_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)bodies.size());
}

void Renderer::renderBlackHole(const glm::mat4 &view, const glm::mat4 &proj, float time)
{
    glUseProgram(m_bhShaderProgram);

    unsigned int modelLoc = glGetUniformLocation(m_bhShaderProgram, "uModel");
    unsigned int viewLoc = glGetUniformLocation(m_bhShaderProgram, "uView");
    unsigned int projLoc = glGetUniformLocation(m_bhShaderProgram, "uProjection");
    unsigned int timeLoc = glGetUniformLocation(m_bhShaderProgram, "uTime");
    unsigned int typeLoc = glGetUniformLocation(m_bhShaderProgram, "uType");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1f(timeLoc, time);

    // Disable instance attributes to use the same VAO for simple quad drawing
    glBindVertexArray(m_vao);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    // Disk
    glUniform1i(typeLoc, 0);
    glm::mat4 modelDisk = glm::mat4(1.0f);
    modelDisk = glm::rotate(modelDisk, glm::radians(90.0f), glm::vec3(1, 0, 0));
    modelDisk = glm::scale(modelDisk, glm::vec3(12.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDisk));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Hole
    glUniform1i(typeLoc, 1);
    glm::mat4 modelHole = glm::mat4(1.0f);
    glm::mat4 invView = glm::inverse(view);
    modelHole[0] = invView[0];
    modelHole[1] = invView[1];
    modelHole[2] = invView[2];
    modelHole[3] = glm::vec4(0, 0, 0, 1);
    modelHole = glm::scale(modelHole, glm::vec3(3.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelHole));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
}

unsigned int Renderer::createShader(const char *vertexSrc, const char *fragmentSrc)
{
    unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexSrc, NULL);
    glCompileShader(vShader);
    unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentSrc, NULL);
    glCompileShader(fShader);
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vShader);
    glAttachShader(prog, fShader);
    glLinkProgram(prog);
    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return prog;
}