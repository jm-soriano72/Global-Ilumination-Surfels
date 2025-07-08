#pragma once

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

class SwapChainManager
{
private:
    VkSwapchainKHR swapChain;            
    std::vector<VkImage> swapChainImages; 
    VkFormat swapChainImageFormat;       
    VkExtent2D swapChainExtent;          

    std::vector<VkImageView> swapChainImageViews; 

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);

public:

    SwapChainManager();

    void createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow *window);
    void createImageViews(VkDevice device);
    void recreateSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow *window,
                           std::vector<VkFramebuffer> swapChainFramebuffers, DepthBuffer depthBufferCreator);
    void cleanupSwapChain(VkDevice device, DepthBuffer depthBufferCreator, std::vector<VkFramebuffer> swapChainFramebuffers);
    void cleanupSwapChain(VkDevice device);

    VkSwapchainKHR getSwapChain() const;
    VkExtent2D getSwapChainExtent() const;
    VkFormat getSCImageFormat() const;
    size_t getNumImageViews() const;
    VkImageView getImageView(int imageID) const;
};