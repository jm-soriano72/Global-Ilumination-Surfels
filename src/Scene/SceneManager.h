#pragma once

#include "Models/MeshContainer.h"
#include "Illumination/LightsData.h"
#include "MaterialsManager.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

class SceneManager
{

public:
    std::vector<MeshContainer> sceneMeshes;
    LightsData sceneLights;
    MaterialsManager materialsManager;

    SceneManager();

    void loadSceneAssets(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
    void createScene(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
    void addIllumination();
    void createMaterials(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
    void cleanup(VkDevice device);
};