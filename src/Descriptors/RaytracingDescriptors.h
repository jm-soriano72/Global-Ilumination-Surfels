#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "Buffers/GeometryUniformBuffer.h"
#include "Raytracing/RaytracingManager.h"
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

class RaytracingDescriptors : public PipelineDescriptors
{
public:
    void createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers,
                           AccelerationStructure &topLevelAccelerationStructure, std::vector<ImageCreator> &diffuseImageCreators, std::vector<ImageCreator> &alphaImageCreators,
                           std::vector<ImageCreator> &specularImageCreators);
    void cleanupDescriptors(VkDevice device) override;

    VkDescriptorSetLayout getDescriptorSetLayout() override;
    VkDescriptorSet getDescriptorSet(int index) override;
};