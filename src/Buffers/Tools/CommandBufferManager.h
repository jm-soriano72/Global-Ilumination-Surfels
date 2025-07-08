#pragma once

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <array>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

class CommandBufferManager
{
public:
    static VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device);
    static void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkDevice device, VkCommandPool commandPool);
};