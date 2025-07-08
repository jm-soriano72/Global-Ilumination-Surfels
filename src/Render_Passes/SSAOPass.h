#pragma once

#define SSAO_NOISE_DIM 8
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"
#include "RenderPass.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

class SSAOPass : public RenderPass
{
private:
    // Imagen para el attachment
    ImageCreator colorImage;
    // Imagen con la textura del ruido
    ImageCreator noiseImage;

    void createImageAttachments(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D swapChainExtent);
    void createNoiseImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

public:
    SSAOPass();

    void createRenderPass(VkDevice device, VkFormat format, VkPhysicalDevice physicalDevice) override;
    void createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                            VkQueue graphicsQueue, VkExtent2D swapChainExtent) override;
    void cleanup(VkDevice device) override;
    void cleanupFramebuffers(VkDevice device) override;

    VkImageView getColorImageView();
    ImageCreator getNoiseImage();
};