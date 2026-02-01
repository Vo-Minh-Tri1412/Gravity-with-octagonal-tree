# Tìm thư viện từ vcpkg
find_package(glm CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
add_executable(gravity
    main.cpp
    Physics/BruteForceSolver.cpp
    Physics/BarnesHutSolver.cpp
    Utils/GalaxyGenerator.cpp
)

target_include_directories(gravity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link thư viện
target_link_libraries(gravity PRIVATE 
    glm::glm
    raylib
)
# Tìm thư viện từ vcpkg
find_package(glm CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)