#include "BufferCreator.h"

#define VMA_IMPLEMENTATION

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <array>
#include <stdexcept>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

VmaAllocator BufferCreator::allocator = VK_NULL_HANDLE;

void BufferCreator::initVMA(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;

    vmaCreateAllocator(&allocatorInfo, &allocator);
}

void BufferCreator::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    // Información para crear el buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;   // Tamaño del buffer
    bufferInfo.usage = usage; // Uso
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Creación del buffer
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    // Obtención de las características de memoria necesarias
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateFlagsInfo allocFlagsInfo{};
    allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);
    allocInfo.pNext = &allocFlagsInfo;
    // Asignación de memoria
    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t BufferCreator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice)
{

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    // Se busca un tipo de memoria que coincida con el tipo del filtro
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    // Si no se ha salido del programa, quiere decir que no se ha encontrado
    throw std::runtime_error("failed to find suitable memory type!");
}

// Creación de buffer con VMA, para agilizar el proceso de creación de recursos para los surfels
void BufferCreator::createBufferVMA(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& bufferAllocation)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = allocSize; 
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocinfo = {};
	vmaallocinfo.usage = memoryUsage;

    vmaCreateBuffer(allocator, &bufferInfo, &vmaallocinfo,
		&buffer, 
		&bufferAllocation, 
		nullptr);
}