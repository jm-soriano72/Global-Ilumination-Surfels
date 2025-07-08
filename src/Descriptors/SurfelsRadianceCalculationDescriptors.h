#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include "Images/ImageCreator.h"
#include "Raytracing/RaytracingManager.h"
#include "PipelineDescriptors.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>

class SurfelsRadianceCalculationDescriptors : public PipelineDescriptors
{
public:
    void createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, VkBuffer surfelBuffer, std::vector<VkBuffer> lightsDataBuffer,
                           AccelerationStructure &topLevelAccelerationStructure, std::vector<VkBuffer> indexBufferList, std::vector<VkBuffer> vertexBufferList, 
                           std::vector<size_t> indexBufferSizeList, std::vector<size_t> vertexBufferSizeList, uint32_t numTextures, uint32_t numMaterials,
                           std::vector<ImageCreator> &diffuseImageCreators, std::vector<ImageCreator> &alphaImageCreators, 
                           std::vector<ImageCreator> &specularImageCreators, ImageCreator raysNoiseImage);
    void cleanupDescriptors(VkDevice device) override;

    VkDescriptorSetLayout getDescriptorSetLayout() override;
    VkDescriptorSet getDescriptorSet(int index) override;
};