#pragma once

#include "Images/ImageCreator.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <array>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

class DepthBuffer {

public:

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	void createDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D swapChainExtent);	
	void createShadowDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height);

	static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
	static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physicalDevice);

	void cleanupDepthResources(VkDevice device);

};