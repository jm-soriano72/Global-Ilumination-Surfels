#include "DepthBuffer.h"

#include "Images/ImageCreator.h"

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

void DepthBuffer::createDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, 
    VkExtent2D swapChainExtent) {
    // Primero se busca un formato que soporte información para la profundidad
    VkFormat depthFormat = findDepthFormat(physicalDevice);
    // Se crea la imagen (recurso de memoria) que almacena la información, utilizando el creador de la clase principal
    ImageCreator::createImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height,
        depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage, depthImageMemory, "DepthImage");
    // Se crea la vista de la imagen, para poder tratar con ella y su contenido
    depthImageView = ImageCreator::createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, false);
    // Transición entre los usos de la imagen
    ImageCreator::transitionImageLayout(commandPool, device, graphicsQueue, depthImage, depthFormat, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

void DepthBuffer::createShadowDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height) {
    // Primero se busca un formato que soporte información para la profundidad
    VkFormat depthFormat = DepthBuffer::findDepthFormat(physicalDevice);
    // Se crea la imagen y se reserva memoria para ella
    ImageCreator::createImage(device, physicalDevice, width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              depthImage, depthImageMemory, "ShadowMap");
    // Se crea su image view asociada
    depthImageView = ImageCreator::createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, false);
}

// Función para seleccionar un formato con un componente de profundiad
VkFormat DepthBuffer::findDepthFormat(VkPhysicalDevice physicalDevice) {
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        physicalDevice
    );
}

// Función para encontrar un formato óptimo para la imagen que va almacenar la información de la profundidad
VkFormat DepthBuffer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, 
    VkFormatFeatureFlags features, VkPhysicalDevice physicalDevice) {

    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        // Se comprueba si las propiedades son adecuadas
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    // Si no se encuentra ninguno, se lanzará una excepción
    throw std::runtime_error("failed to find supported format!");
}

void DepthBuffer::cleanupDepthResources(VkDevice device) {
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
}