#version 450 core

layout (location = 0) in vec3 aQuadPos;    // Basic quad vertices
layout (location = 1) in vec3 aInstancePos; // Center of particle
layout (location = 2) in vec4 aInstanceColor;

uniform mat4 uView;
uniform mat4 uProjection;
uniform float uSize;

out vec4 vColor;
out vec2 vUV;

void main() {
    // Billboarding Math:
    // Extract Right and Up vectors from View Matrix to make quad always face camera
    vec3 cameraRight = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 cameraUp    = vec3(uView[0][1], uView[1][1], uView[2][1]);

    vec3 vertexPos = aInstancePos 
                   + cameraRight * aQuadPos.x * uSize 
                   + cameraUp * aQuadPos.y * uSize;

    vColor = aInstanceColor;
    vUV = aQuadPos.xy + 0.5; // Map -0.5..0.5 to 0..1
    
    gl_Position = uProjection * uView * vec4(vertexPos, 1.0);
}