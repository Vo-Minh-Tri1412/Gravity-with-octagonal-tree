# Tìm thư viện từ vcpkg
find_package(glm CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
add_executable(gravity
    main.cpp
    # Core
    Core/Particle.hpp
    Core/Allocator.hpp
    
    # Structure
    Structure/AABB.hpp
    Structure/Octree.hpp
    Structure/Octree.cpp          
    
    # Physics
    Physics/ISolver.hpp
    Physics/BarnesHutSolver.hpp
    Physics/BarnesHutSolver.cpp   
    Physics/BruteForceSolver.hpp
    Physics/BruteForceSolver.cpp  
    
    # Graphics
    Graphics/Camera.hpp
    Graphics/Camera.cpp           
    Graphics/Renderer.hpp
    Graphics/Renderer.cpp         
    
    # Utils
    Utils/GalaxyGenerator.hpp
    Utils/GalaxyGenerator.cpp     
    Utils/Timer.hpp
)


target_link_libraries(gravity PRIVATE raylib glm::glm)


target_include_directories(gravity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link thư viện
target_link_libraries(gravity PRIVATE 
    glm::glm
    raylib
)
# Tìm thư viện từ vcpkg
find_package(glm CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)