#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "ShadowMappingUniformBuffer.h"

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

class RaytracingUniformBuffer
{

private:
    // Matrices de transformación
    std::vector<VkBuffer> uniformMVPBuffers;
    std::vector<VkDeviceMemory> uniformMVPBuffersMemory;
    std::vector<void *> uniformMVPBuffersMapped;
    // Fuentes de luz
    std::vector<VkBuffer> lightBuffers;
    std::vector<VkDeviceMemory> lightBuffersMemory;
    std::vector<void *> lightBuffersMapped;

public:
    void createRaytracingUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT,
                                      uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights);
    void updateRaytracingUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights);
    void cleanup(VkDevice device);

    std::vector<VkBuffer> getMVPBuffers();
    std::vector<VkBuffer> getLightsBuffers();

private:
    void createMVPUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height, Camera *camera);
    void createLightsBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, LightsData sceneLights);

    void updateMVPUniformBuffer(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera);
    void updateLightsBuffer(uint32_t currentImage, LightsData sceneLights);
};