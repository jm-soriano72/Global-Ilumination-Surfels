#include "LightsData.h"

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

LightsData::LightsData() {}

LightsData::LightsData(std::vector<glm::vec3> lPositions, std::vector<float> lIntensities, std::vector<glm::vec3> lColors)
{
    lights[0] = Light(lPositions[0], lIntensities[0], lColors[0]);
    lights[1] = Light(lPositions[1], lIntensities[1], lColors[1]);
    lights[2] = Light(lPositions[2], lIntensities[2], lColors[2]);
    lights[3] = Light(lPositions[3], lIntensities[3], lColors[3]);

    mainLight = MainDirectionalLight();
}