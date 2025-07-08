#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include "Images/ImageCreator.h"
#include "PipelineDescriptors.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>

class SSAODescriptors : public PipelineDescriptors
{
public:
    void createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<VkBuffer> uniformProjectionBuffers, std::vector<VkBuffer> uniformSSAOParamsBuffers,
                           VkSampler colorSampler, VkImageView positionImageView, VkImageView normalImageView, ImageCreator noiseTexture);
    void cleanupDescriptors(VkDevice device) override;

    VkDescriptorSetLayout getDescriptorSetLayout() override;
    VkDescriptorSet getDescriptorSet(int index) override;
};