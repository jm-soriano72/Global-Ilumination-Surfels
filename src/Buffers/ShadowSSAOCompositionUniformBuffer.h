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

class ShadowSSAOCompositionUniformBuffer
{

private:
    // Fuentes de luz
    std::vector<VkBuffer> lightBuffers;
    std::vector<VkDeviceMemory> lightBuffersMemory;
    std::vector<void *> lightBuffersMapped;
    // Matriz de transformación de la luz
    std::vector<VkBuffer> mainLightDataBuffers;
    std::vector<VkDeviceMemory> mainLightDataBuffersMemory;
    std::vector<void *> mainLightDataBuffersMapped;
    // Matrices de transformación
    std::vector<VkBuffer> uniformMVPBuffers;
    std::vector<VkDeviceMemory> uniformMVPBuffersMemory;
    std::vector<void *> uniformMVPBuffersMapped;

public:
    void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT,
                                      uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights);
    void updateUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights);
    void cleanup(VkDevice device);

    std::vector<VkBuffer> getLightsBuffers();
    std::vector<VkBuffer> getMainLightBuffers();
    std::vector<VkBuffer> getMVPBuffers();

private:
    void createLightsBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, LightsData sceneLights);
    void createMainLightDataBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT);
    void createMVPUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height, Camera *camera);

    void updateLightsBuffer(uint32_t currentImage, LightsData sceneLights);
    void updateMainLightDataBuffer(uint32_t currentImage);
    void updateMVPUniformBuffer(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera);
};