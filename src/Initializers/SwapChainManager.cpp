#include "SwapChainManager.h"

#include "Tools/VulkanUtils.h"
#include "Images/ImageCreator.h"
#include "Render_Passes/Utils/DepthBuffer.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <cstdint>
#include <limits>
#include <algorithm>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

SwapChainManager::SwapChainManager() {}

void SwapChainManager::createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow *window)
{
    SwapChainSupportDetails swapChainSupport = VulkanUtils::querySwapChainSupport(physicalDevice, surface);
    // Se obtienen las características
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    swapChainImageFormat = surfaceFormat.format;
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);
    swapChainExtent = extent;
    // Se establece el número de imágenes que se tendrán. Es recomendable tener una más que el mínimo establecido
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // Se comprueba respecto al máximo número de imágenes permitido
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    // Estructura para su creación
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Se especifica cómo manejar las imágenes, entre las distintas familias
    // En esta caso, los gráficos y la presentación
    QueueFamilyIndices indices = VulkanUtils::findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        // La imagen será utilizada por varias familias que se la irán pasando
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        // La imagen primero la tiene una familia y después pasa a otra
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // Se puede aplicar una transformación a las imágenes
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;            // Se ignora el canal alfa
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // Se activa el clipping
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    // Se prepara a la lista de imágenes
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
}

void SwapChainManager::createImageViews(VkDevice device)
{
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = ImageCreator::createImageView(device, swapChainImages[i], swapChainImageFormat,
                                                               VK_IMAGE_ASPECT_COLOR_BIT, false);
    }
}

void SwapChainManager::recreateSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow *window,
                                         std::vector<VkFramebuffer> swapChainFramebuffers, DepthBuffer depthBufferCreator)
{
    // Redimensión del framebuffer
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    // Se evita tocar recursos que estén en uso
    vkDeviceWaitIdle(device);
    // Se vacía la swap chain actual
    cleanupSwapChain(device, depthBufferCreator, swapChainFramebuffers);
    // Se recrean todos los objetos involucrados (las Image Views conforman la Swap Chain)
    createSwapChain(device, physicalDevice, surface, window);
    createImageViews(device);
}

void SwapChainManager::cleanupSwapChain(VkDevice device, DepthBuffer depthBufferCreator, std::vector<VkFramebuffer> swapChainFramebuffers)
{
    // Se limpian los recursos de profundidad, para poder recrearlos
    depthBufferCreator.cleanupDepthResources(device);
    // Antes de destruir el Pipeline y el Render Pass, se eliminan los framebuffer asociados
    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    // Se eliminan las view images
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
    // Antes de eliminar el dispositivo lógico, se destruye el swap chain
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void SwapChainManager::cleanupSwapChain(VkDevice device)
{
    // Se eliminan las view images
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
    // Antes de eliminar el dispositivo lógico, se destruye el swap chain
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

VkSurfaceFormatKHR SwapChainManager::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    // Se busca si se tiene el formato deseado
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    // En caso de que no, se devuelve el primero
    return availableFormats[0];
}
// Presentación de las imágenes
VkPresentModeKHR SwapChainManager::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        // Se trata de buscar el modo que simula un triple buffer
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    // Si no, se selecciona una simple cola
    return VK_PRESENT_MODE_FIFO_KHR;
}
// Resolución óptima de las imágenes
VkExtent2D SwapChainManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
{
    // Solución al conflicto por la función max
    uint32_t maxValue = (std::numeric_limits<uint32_t>::max)();
    if (capabilities.currentExtent.width != maxValue)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};
        // Se colocan los valores dentro de los límites establecidos
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkSwapchainKHR SwapChainManager::getSwapChain() const { return this->swapChain; }
VkExtent2D SwapChainManager::getSwapChainExtent() const { return this->swapChainExtent; }
VkFormat SwapChainManager::getSCImageFormat() const { return this->swapChainImageFormat; }
size_t SwapChainManager::getNumImageViews() const { return this->swapChainImageViews.size(); }
VkImageView SwapChainManager::getImageView(int imageID) const { return this->swapChainImageViews[imageID]; }
