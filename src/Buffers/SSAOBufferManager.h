#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#define SSAO_KERNEL_SIZE 64
#define SSAO_RADIUS 37.5

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <chrono>
#include <vector>
#include <array>

struct UniformSSAOBufferObject
{
    alignas(16) glm::mat4 proj;
};

struct UniformSSAOBufferKernel
{
    std::array<glm::vec4, SSAO_KERNEL_SIZE> kernels;
};

class SSAOUniformBuffer
{
private:
    // Buffers para almacenar la matriz de proyección
    std::vector<VkBuffer> uniformProjectionBuffers;
    std::vector<VkDeviceMemory> uniformProjectionBuffersMemory;
    std::vector<void *> uniformProjectionBuffersMapped;

    // Buffers para almacenar los parámetros del SSAO
    std::vector<VkBuffer> uniformSSAOBuffers;
    std::vector<VkDeviceMemory> uniformSSAOBuffersMemory;
    std::vector<void *> uniformSSAOBuffersMapped;

    void createUniformProjBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height);
    void createUniformSSAOBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height);

public:
    void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height);
    void updateUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height);
    std::vector<VkBuffer> getProjectionUniformBuffers();
    std::vector<VkBuffer> getSSAOUniformBuffers();
    void cleanup(VkDevice device);
};