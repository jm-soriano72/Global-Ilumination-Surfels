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

class BufferCreator
{
public:
	static VmaAllocator allocator;
	// Función para inicializar VMA
	static void initVMA(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
	// Función para crear un buffer a bajo nivel
	static void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage,
							 VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
	// Función para encontrar el tipo de memoria adecuado para la aplicaicón, teniendo en cuenta los requerimientos de esta y del buffer
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
	// Función para crear un buffer a alto nivel
	static void createBufferVMA(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer &buffer, VmaAllocation &bufferAllocation);
};