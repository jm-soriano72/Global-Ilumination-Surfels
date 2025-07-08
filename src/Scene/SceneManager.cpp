#include "SceneManager.h"

#include "SponzaResources.h"
#include "Models/MeshContainer.h"
#include "Illumination/LightsData.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

SceneManager::SceneManager() {}

void SceneManager::loadSceneAssets(VkDevice device, VkPhysicalDevice physicalDevice,
                                   VkCommandPool commandPool, VkQueue graphicsQueue)
{
    createScene(device, physicalDevice, commandPool, graphicsQueue);
    addIllumination();
    createMaterials(device, physicalDevice, commandPool, graphicsQueue);
}

void SceneManager::createScene(VkDevice device, VkPhysicalDevice physicalDevice,
                               VkCommandPool commandPool, VkQueue graphicsQueue)
{
    // Se asigna el tamaño de la escena según el número de modelos
    sceneMeshes.resize(meshesPaths.size());
    // Se recorre el vector que contiene los directorios de los distintos modelos de la escena
    uint32_t materialIndex = 0;
    uint32_t *materialIndexPtr = &materialIndex;
    for (uint32_t i = 0; i < meshesPaths.size(); i++)
    {
        // Se cargan los datos del modelo y se almacenan
        sceneMeshes[i].loadVertexData(device, physicalDevice, commandPool, graphicsQueue, meshesPaths[i], materialIndexPtr);
    }
    std::cout << "Numero materiales total: " << (*materialIndexPtr);
}

void SceneManager::addIllumination()
{
    sceneLights = LightsData(lightsPositions, lightsIntensities, lightsColors);
}

void SceneManager::createMaterials(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
    materialsManager.prepareTextureMaterials(device, physicalDevice, commandPool, graphicsQueue);
}

void SceneManager::cleanup(VkDevice device)
{
    for (uint32_t i = 0; i < sceneMeshes.size(); i++)
    {
        sceneMeshes[i].vertexMeshesData.cleanup(device);
    }
    materialsManager.cleanup(device);
}
