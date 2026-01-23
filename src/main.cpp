#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>

#include "Physics.hpp"
#include "PhysicsGPU.hpp"
#include "Renderer.hpp"
#include "Galaxy.hpp"

// ============================================================================
// GPU Selection for Discrete Graphics
// ============================================================================
#ifdef _WIN32
extern "C"
{
    // NVIDIA Optimus: Request high-performance GPU
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    // AMD PowerXpress: Request high-performance GPU
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const int PARTICLE_COUNT = 25000;

// Physics Backend Selection
enum class PhysicsBackend
{
    CPU_BARNES_HUT, // O(N log N) - uses Octree, runs on CPU with OpenMP
    GPU_DIRECT      // O(N²) - direct N-body on GPU with CUDA (faster for N < 100K)
};
PhysicsBackend currentBackend = PhysicsBackend::GPU_DIRECT;
bool useGPU = true; // Will be set based on CUDA availability

// Camera State
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float camYaw = -90.0f;
float camPitch = 0.0f;
float camDist = 50.0f;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
double lastTime = 0.0;
int frameCount = 0;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle between GPU and CPU physics with 'G' key
    static bool gKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        if (!gKeyPressed)
        {
            useGPU = !useGPU;
            std::cout << "[Input] Physics Backend: " << (useGPU ? "GPU (CUDA)" : "CPU (Barnes-Hut)") << std::endl;
        }
        gKeyPressed = true;
    }
    else
    {
        gKeyPressed = false;
    }
}

int main()
{
    std::cout << "============================================" << std::endl;
    std::cout << "      Nebula Architect C++ - CUDA Edition   " << std::endl;
    std::cout << "============================================" << std::endl;

    // Check CUDA availability and print device info
    if (PhysicsGPU::checkCUDAAvailable())
    {
        PhysicsGPU::printDeviceInfo();
        useGPU = true;
    }
    else
    {
        std::cout << "[CUDA] Not available, using CPU physics" << std::endl;
        useGPU = false;
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Nebula Architect C++ (CUDA Edition)", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending
    glDisable(GL_DEPTH_TEST);

    // Physics systems
    PhysicsSystem cpuPhysics; // CPU Barnes-Hut (O(N log N))
    PhysicsGPU gpuPhysics;    // GPU Direct N-body (O(N²) but parallelized)

    Renderer renderer;
    std::vector<Body> bodies;

    std::cout << "\n[Galaxy] Generating " << PARTICLE_COUNT << " particles..." << std::endl;
    Galaxy::generate(bodies, PARTICLE_COUNT);

    // Initialize both physics backends
    cpuPhysics.setBodies(bodies);

    if (useGPU && gpuPhysics.isGPUAvailable())
    {
        gpuPhysics.initialize(bodies);
    }
    else
    {
        useGPU = false;
    }

    renderer.init();

    std::cout << "\n============================================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "  G   - Toggle GPU/CPU physics" << std::endl;
    std::cout << "============================================\n"
              << std::endl;

    // Timing and performance metrics
    float physicsTimeMs = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // FPS Counter Logic
        frameCount++;
        if (currentFrame - lastTime >= 1.0)
        {
            std::string backend = useGPU ? "GPU-CUDA" : "CPU-Barnes-Hut";
            std::ostringstream title;
            title << "Nebula Architect C++ | FPS: " << frameCount
                  << " | Bodies: " << bodies.size()
                  << " | Physics: " << backend
                  << " (" << std::fixed << std::setprecision(2) << physicsTimeMs << " ms)";
            glfwSetWindowTitle(window, title.str().c_str());
            frameCount = 0;
            lastTime = currentFrame;
        }

        processInput(window);

        // Physics - choose backend based on useGPU flag
        float dt = std::min(deltaTime, 0.05f);

        if (useGPU && gpuPhysics.isInitialized())
        {
            // GPU physics path
            gpuPhysics.step(dt, cpuPhysics.getG(), cpuPhysics.getBlackHoleMass());
            gpuPhysics.downloadBodies(bodies);
            cpuPhysics.setBodies(bodies); // Sync CPU state for rendering
            physicsTimeMs = gpuPhysics.getLastKernelTimeMs();
        }
        else
        {
            // CPU physics path (Barnes-Hut Octree)
            auto start = std::chrono::high_resolution_clock::now();
            cpuPhysics.step(dt);
            auto end = std::chrono::high_resolution_clock::now();
            physicsTimeMs = std::chrono::duration<float, std::milli>(end - start).count();
        }

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float camX = sin(currentFrame * 0.15f) * camDist;
        float camZ = cos(currentFrame * 0.15f) * camDist;
        float camY = 25.0f + sin(currentFrame * 0.05f) * 10.0f;

        glm::mat4 view = glm::lookAt(
            glm::vec3(camX, camY, camZ),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

        renderer.render(cpuPhysics.getBodies(), view, projection, currentFrame);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}