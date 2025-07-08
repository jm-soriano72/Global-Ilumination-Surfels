#pragma once

#include "Camera/Camera.h"

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

struct UniformGBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(8) glm::vec2 nearFarPlanes;
};

class GUniformBuffer
{
private:
    // Buffers para almacenar las variables uniformes
    std::vector<VkBuffer> uniformBuffers;
    // Memoria para dicho buffer
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void *> uniformBuffersMapped;


public:
    void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height, Camera* camera);
    void updateUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height, Camera* camera);
    std::vector<VkBuffer> getUniformBuffers();
    void cleanup(VkDevice device);
};