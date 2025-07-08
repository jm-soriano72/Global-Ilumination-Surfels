#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

class RenderPass
{
protected:
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;

    DepthBuffer depthBufferCreator;

public:
    RenderPass();

    virtual void createRenderPass(VkDevice device, VkFormat format, VkPhysicalDevice physicalDevice) = 0;
    virtual void createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                            VkQueue graphicsQueue, VkExtent2D swapChainExtent) = 0;
    virtual void cleanup(VkDevice device) = 0;
    virtual void cleanupFramebuffers(VkDevice device) = 0;

    VkRenderPass getRenderPass();
    VkFramebuffer getFramebuffer(int index);
    std::vector<VkFramebuffer> getFramebuffers();
    DepthBuffer getDepthBufferCreator();

};