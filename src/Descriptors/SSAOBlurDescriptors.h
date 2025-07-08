#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include "PipelineDescriptors.h"

#include <vector>

class SSAOBlurDescriptors : public PipelineDescriptors
{
public:
    void createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, VkSampler colorSampler, VkImageView colorImageView);
    void cleanupDescriptors(VkDevice device) override;

    VkDescriptorSetLayout getDescriptorSetLayout() override;
    VkDescriptorSet getDescriptorSet(int index) override;
};