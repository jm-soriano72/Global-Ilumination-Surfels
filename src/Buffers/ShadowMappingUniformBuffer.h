#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <chrono>
#include <vector>
#include <array>

extern struct UniformDataOffscreen
{
    glm::mat4 depthMVP; // Matriz de transformación desde el punto de vista de la luz
} uniformDataOffscreen;

class ShadowMappingUniformBuffer
{
private:
    // Buffers para almacenar las variables uniformes
    std::vector<VkBuffer> uniformOffscreenBuffers;
    // Memoria para dicho buffer
    std::vector<VkDeviceMemory> uniformOffscreenBuffersMemory;
    std::vector<void *> uniformOffscreenBuffersMapped;

    // INFORMACIÓN DE LA FUENTE DE ILUMINACIÖN
    float zNear = 500.0f;
    float zFar = 5000.0f;

public:
    void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, MainDirectionalLight light,
                              uint32_t width, uint32_t height);
    void updateUniformBuffersOffscreen(uint32_t currentImage, MainDirectionalLight light, uint32_t width, uint32_t height);
    std::vector<VkBuffer> getUniformBuffers();
    void cleanup(VkDevice device);
};