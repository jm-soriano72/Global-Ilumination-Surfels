#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <vector>

class SynchronizationObjects
{

private:

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t MAX_FRAMES_IN_FLIGHT;

public:

    SynchronizationObjects(uint32_t maxFrames);
    SynchronizationObjects();

    void createSyncObjects(VkDevice device);
    void cleanup(VkDevice device);

    const VkFence *getFence(uint32_t currentFrame);
    const VkSemaphore getImageSemaphore(uint32_t currentFrame);
    const VkSemaphore getRenderSemaphore(uint32_t currentFrame);
};