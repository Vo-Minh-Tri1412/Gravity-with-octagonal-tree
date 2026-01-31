add_executable(gravity
    main.cpp
    Physics/BruteForceSolver.cpp
    Physics/BarnesHutSolver.cpp
    Utils/GalaxyGenerator.cpp
)

target_include_directories(gravity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
