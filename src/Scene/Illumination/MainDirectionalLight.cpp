#include "MainDirectionalLight.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

MainDirectionalLight::MainDirectionalLight() {
    position = glm::vec3(800.0, 2500.0, -200);
    target = glm::vec3(0.0, 0.0, 0.0);
    intensity = 1.0f;

    direction = glm::normalize(target - position);
}