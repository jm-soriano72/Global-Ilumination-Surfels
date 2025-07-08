#pragma once

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

class Light
{
public:
    alignas(16) glm::vec3 position; // Posición
    float intensity;                // Intensidad
    alignas(16) glm::vec3 color;    // Color

    Light();
    Light(glm::vec3 p, float i, glm::vec3 c);
};