#pragma once

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

class MainDirectionalLight {
public:
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 target;
    alignas(16) glm::vec3 direction;
    float intensity;

    MainDirectionalLight();
};