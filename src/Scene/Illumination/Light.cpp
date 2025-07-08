#include "Light.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

Light::Light() {
    position = glm::vec3(0.0);
    intensity = 1.0;
    color = glm::vec3(1.0);
}

Light::Light(glm::vec3 p, float i, glm::vec3 c) {
    position = p;
    intensity = i;
    color = c;
}