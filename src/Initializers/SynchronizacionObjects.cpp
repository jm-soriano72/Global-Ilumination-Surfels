#include "SynchronizationObjects.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <vector>

SynchronizationObjects::SynchronizationObjects(uint32_t maxFrames)
{
    MAX_FRAMES_IN_FLIGHT = maxFrames;
}

SynchronizationObjects::SynchronizationObjects()
{
    MAX_FRAMES_IN_FLIGHT = 2;
}

void SynchronizationObjects::createSyncObjects(VkDevice device)
{

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void SynchronizationObjects::cleanup(VkDevice device)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
}

const VkFence* SynchronizationObjects::getFence(uint32_t currentFrame) { return &this->inFlightFences[currentFrame]; }
const VkSemaphore SynchronizationObjects::getImageSemaphore(uint32_t currentFrame) { return this->imageAvailableSemaphores[currentFrame]; }
const VkSemaphore SynchronizationObjects::getRenderSemaphore(uint32_t currentFrame) { return this->renderFinishedSemaphores[currentFrame]; }