/*
  OpenGL Loader (GLAD).
  NOTE: This is a placeholder for the generated glad.c file.
  To compile this project, you MUST:
  1. Go to https://glad.dav1d.de/
  2. Select C/C++
  3. Select OpenGL Version 4.5
  4. Profile: Core
  5. Generate
  6. Copy 'src/glad.c' content into this file.
  7. Copy 'include/glad' and 'include/KHR' folders into your 'include/' directory.

  (The full glad.c is too large to include in this text response, but CMake expects it here).
*/

#include <stdio.h>
#include <stdlib.h>

// Minimal dummy implementation to satisfy linker if you try to build without downloading
// BUT IT WILL FAIL AT RUNTIME. PLEASE DOWNLOAD REAL GLAD.C
#include <glad/glad.h>

int gladLoadGLLoader(GLADloadproc load)
{
    // This is fake. Download real glad.c
    printf("ERROR: You are using a placeholder glad.c. Please download the real one from https://glad.dav1d.de/\n");
    return 0;
}
