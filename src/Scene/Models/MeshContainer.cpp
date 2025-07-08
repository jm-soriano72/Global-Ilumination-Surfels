#include "MeshContainer.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

#include <vector>

void MeshContainer::loadVertexData(VkDevice device, VkPhysicalDevice physicalDevice,
    VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& filePath, uint32_t* materialIndex) {
        vertexMeshesData.createBufferData(device, physicalDevice, commandPool, graphicsQueue, filePath, materialIndex);
}