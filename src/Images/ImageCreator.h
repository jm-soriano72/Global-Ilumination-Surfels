#pragma once

#include "Buffers/Tools/BufferCreator.h"
#include "Buffers/Tools/CommandBufferManager.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <array>

class ImageCreator
{

public:
    static const int MIP_MAP_LEVELS = 6;

    // Buffer intermedios para la carga de texturas
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    // Objetos para facilitar el acceso a las texturas desde los shaders
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    VkImageView textureImageView;
    VkSampler textureSampler = VK_NULL_HANDLE;

    void createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const char *texturePath);
    void createNoiseTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
                                 std::vector<glm::vec4> *noiseValues, uint32_t width, uint32_t height);
    void createRaysNoiseTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
                                     std::vector<glm::vec2> *noiseValues, uint32_t width, uint32_t height);
    static void createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                            uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory, char *name);
    static void transitionImageLayout(VkCommandPool commandPool, VkDevice device, VkQueue graphicsQueue,
                                      VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkCommandPool commandPool, VkDevice device, VkQueue graphicsQueue,
                           VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    static VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, bool mipMapEnabled);
    void createTextureImageView(VkDevice device);
    void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice);
    static bool hasStencilComponent(VkFormat format);
    void cleanup(VkDevice device);

private:
    void generateMipmaps(VkCommandPool commandPool, VkDevice device, VkQueue graphicsQueue,
                         VkFormat imageFormat, uint32_t texWidth, uint32_t texHeight);
};