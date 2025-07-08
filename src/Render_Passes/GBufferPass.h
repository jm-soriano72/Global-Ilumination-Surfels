#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"
#include "Images/ImageCreator.h"
#include "RenderPass.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

class GBufferPass : public RenderPass
{
private:
    // Imágenes para los attachment
    ImageCreator positionImage;
    ImageCreator normalImage;
    ImageCreator albedoImage;
    ImageCreator specularImage;

    void createImageAttachments(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D swapChainExtent);

public:
    GBufferPass();

    void createRenderPass(VkDevice device, VkFormat format, VkPhysicalDevice physicalDevice) override;
    void createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                            VkQueue graphicsQueue, VkExtent2D swapChainExtent) override;
    void cleanup(VkDevice device) override;
    void cleanupFramebuffers(VkDevice device) override;    

    VkImageView getPositionImageView();
    VkImageView getNormalImageView();
    VkImageView getAlbedoImageView();
    VkImageView getSpecularImageView();
};