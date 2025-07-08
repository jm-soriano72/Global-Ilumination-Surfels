#pragma once

#include "Light.h"
#include "MainDirectionalLight.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

class LightsData
{
public:
    std::array<Light, 4> lights;
    MainDirectionalLight mainLight;

    LightsData();
    LightsData(std::vector<glm::vec3> lPositions, std::vector<float> lIntensities, std::vector<glm::vec3> lColors);
};