#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "ShadowMappingDescriptors.h"
#include "PipelineDescriptors.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>

class GeometryDescriptors : public PipelineDescriptors
{
private:
    void createDescriptorSetLayout(VkDevice device, const uint32_t numMaterials, const uint32_t numTextures);
    void createDescriptorPool(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT);
    void createDescriptorSets(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<ImageCreator> &diffuseImageCreators,
                              std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, VkImageView depthImageView, VkSampler depthSampler,
                              std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffers);

public:
    void createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<ImageCreator> &diffuseImageCreators,
                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, VkImageView depthImageView, VkSampler depthSampler,
                           std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffers);

    void cleanupDescriptors(VkDevice device) override;

    VkDescriptorSetLayout getDescriptorSetLayout() override;
    VkDescriptorSet getDescriptorSet(int index) override;
};