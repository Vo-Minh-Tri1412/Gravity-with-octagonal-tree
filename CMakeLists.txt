cmake_minimum_required(VERSION 3.18)

# ============================================================================
# CUDA Configuration (must be before project())
# ============================================================================
option(USE_CUDA "Enable CUDA acceleration for physics" ON)

# Check if CUDA is available and we're using a compatible compiler
# Note: CUDA nvcc on Windows requires MSVC, not MinGW
include(CheckLanguage)
check_language(CUDA)

if(CMAKE_CUDA_COMPILER AND USE_CUDA AND NOT MINGW)
    set(CUDA_ENABLED TRUE)
    message(STATUS "CUDA compiler found: ${CMAKE_CUDA_COMPILER}")
else()
    set(CUDA_ENABLED FALSE)
    if(USE_CUDA AND MINGW)
        message(WARNING "CUDA requires MSVC on Windows - CUDA disabled for MinGW build")
        message(STATUS "To use CUDA, build with Visual Studio: cmake -G \"Visual Studio 17 2022\" ..")
    elseif(USE_CUDA)
        message(WARNING "CUDA requested but not found - building without CUDA support")
    endif()
endif()

# ============================================================================
# VCPKG Configuration (Auto-detect or use environment variable)
# ============================================================================
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
elseif(EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
elseif(EXISTS "$ENV{USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Set triplet for MinGW
if(MINGW)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic" CACHE STRING "Vcpkg target triplet")
endif()

# ============================================================================
# Project Info
# ============================================================================
if(CUDA_ENABLED)
    project(UNIVERSE VERSION 1.0.0 LANGUAGES CXX C CUDA)
else()
    project(UNIVERSE VERSION 1.0.0 LANGUAGES CXX C)
endif()

# C++ Standard: Modern C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# CUDA Settings (if enabled)
# ============================================================================
if(CUDA_ENABLED)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    
    # Auto-detect GPU architecture or use common ones
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
        # Support most modern GPUs: Pascal (60), Volta (70), Turing (75), Ampere (80, 86), Ada (89)
        set(CMAKE_CUDA_ARCHITECTURES "60;70;75;80;86;89")
    endif()
    
    message(STATUS "CUDA Architectures: ${CMAKE_CUDA_ARCHITECTURES}")
endif()

# ============================================================================
# Dependencies
# ============================================================================
find_package(OpenGL REQUIRED)
find_package(OpenMP)
find_package(glm CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

if(CUDA_ENABLED)
    find_package(CUDAToolkit REQUIRED)
endif()

# ============================================================================
# Include Directories
# ============================================================================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# Source Files
# ============================================================================
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
)

file(GLOB_RECURSE HEADERS
    "include/*.hpp"
)

# CUDA sources (if enabled)
if(CUDA_ENABLED)
    file(GLOB_RECURSE CUDA_SOURCES
        "src/cuda/*.cu"
    )
    file(GLOB_RECURSE CUDA_HEADERS
        "src/cuda/*.cuh"
    )
    list(APPEND SOURCES ${CUDA_SOURCES})
    list(APPEND HEADERS ${CUDA_HEADERS})
endif()

# ============================================================================
# Create Executable
# ============================================================================
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add CUDA compile definitions if enabled
if(CUDA_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_CUDA)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/cuda)
    
    # CUDA-specific flags
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
    )
endif()

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(${PROJECT_NAME} PRIVATE 
    OpenGL::GL 
    glm::glm
    raylib
    glad::glad
    glfw
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()

if(CUDA_ENABLED)
    target_link_libraries(${PROJECT_NAME} PRIVATE CUDA::cudart)
endif()

# ============================================================================
# Post-build: Copy DLLs and shaders
# ============================================================================
# Copy shaders if directory exists
if(EXISTS "${CMAKE_SOURCE_DIR}/shaders")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            ${CMAKE_BINARY_DIR}/shaders)
endif()

# ============================================================================
# Installation Info
# ============================================================================
message(STATUS "==============================================")
message(STATUS "Build Configuration:")
message(STATUS "  CMake Version: ${CMAKE_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(DEFINED VCPKG_TARGET_TRIPLET)
    message(STATUS "  VCPKG Triplet: ${VCPKG_TARGET_TRIPLET}")
endif()
if(CUDA_ENABLED)
    message(STATUS "  CUDA: ENABLED")
    message(STATUS "  CUDA Compiler: ${CMAKE_CUDA_COMPILER}")
    message(STATUS "  CUDA Architectures: ${CMAKE_CUDA_ARCHITECTURES}")
else()
    message(STATUS "  CUDA: DISABLED")
endif()
message(STATUS "==============================================")