#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "GeometryDescriptors.h"
#include "ShadowMappingDescriptors.h"
#include "GBufferDescriptors.h"
#include "SSAODescriptors.h"
#include "SSAOBlurDescriptors.h"
#include "SSAOCompositionDescriptors.h"
#include "ShadowsSSAOCompositionDescriptors.h"
#include "RaytracingDescriptors.h"
#include "SurfelsGenerationDescriptors.h"
#include "SurfelsVisualizationDescriptors.h"
#include "SurfelsRadianceCalculationDescriptors.h"
#include "IndirectDiffuseShadingDescriptors.h"
#include "SurfelsCompositionDescriptors.h"

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

class DescriptorsManager
{
private:
    GeometryDescriptors geometryDescriptors;
    ShadowMappingDescriptors shadowMappingDescriptors;

    GBufferDescriptors gBufferDescriptors;
    SSAODescriptors ssaoDescriptors;
    SSAOBlurDescriptors ssaoBlurDescriptors;
    SSAOCompositionDescriptors ssaoCompositionDescriptors;

    ShadowsSSAOCompositionDescriptors shadowsSSAOCompositionDescriptors;

    RaytracingDescriptors raytracingDescriptors;

    SurfelsGenerationDescriptors surfelsGenerationDescriptors;
    SurfelsVisualizationDescriptors surfelsVisualizationDescriptors;
    SurfelsRadianceCalculationDescriptors surfelsRadianceCalculationDescriptors;
    IndirectDiffuseShadingDescriptors surfelsIndirectShadingDescriptors;
    SurfelsCompositionDescriptors surfelsCompositionDescriptors;

    VkSampler colorSampler;
    VkSampler getSSAOColorSampler(VkDevice device);

public:
    DescriptorsManager();

    void createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<ImageCreator> &diffuseImageCreators,
                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, VkImageView depthImageView, VkSampler depthSampler,
                           std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffers, std::vector<VkBuffer> uniformShadowBuffers);
    void createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t numTextures, uint32_t numMaterials, std::vector<ImageCreator> &diffuseImageCreators,
                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, std::vector<VkBuffer> gUniformBuffers,
                           std::vector<VkBuffer> ssaoProjUniformBuffers, std::vector<VkBuffer> ssaoParamsUniformBuffers, VkImageView positionImageView, VkImageView normalImageView,
                           VkImageView albedoImageView, VkImageView colorSSAOImageView, VkImageView colorSSAOBlurImageView, ImageCreator noiseTexture);
    void createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t numTextures, uint32_t numMaterials, std::vector<ImageCreator> &diffuseImageCreators,
                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, std::vector<VkBuffer> gUniformBuffers,
                           std::vector<VkBuffer> ssaoProjUniformBuffers, std::vector<VkBuffer> ssaoParamsUniformBuffers, VkImageView positionImageView, VkImageView normalImageView,
                           VkImageView albedoImageView, VkImageView colorSSAOImageView, VkImageView colorSSAOBlurImageView, ImageCreator noiseTexture, std::vector<VkBuffer> uniformShadowBuffers,
                           VkImageView depthImageView, VkSampler depthSampler, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffer,
                           std::vector<VkBuffer> uniformMVPBuffers, VkImageView specularImageView);
    void createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers,
                           AccelerationStructure &topLevelAccelerationStructure, std::vector<ImageCreator> &diffuseImageCreators, std::vector<ImageCreator> &alphaImageCreators,
                           std::vector<ImageCreator> &specularImageCreators);
    void createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t numTextures, uint32_t numMaterials, std::vector<ImageCreator> &diffuseImageCreators,
                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, std::vector<VkBuffer> gUniformBuffers,
                           std::vector<VkBuffer> ssaoProjUniformBuffers, std::vector<VkBuffer> ssaoParamsUniformBuffers, VkImageView positionImageView, VkImageView normalImageView,
                           VkImageView albedoImageView, VkImageView colorSSAOImageView, VkImageView colorSSAOBlurImageView, ImageCreator noiseTexture, std::vector<VkBuffer> uniformShadowBuffers,
                           VkImageView depthImageView, VkSampler depthSampler, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffer,
                           std::vector<VkBuffer> uniformMVPBuffers, VkImageView specularImageView, VkBuffer surfelBuffer, VkBuffer surfelStatsBuffer, VkBuffer surfelGridBuffer,
                           VkBuffer surfelCellBuffer, VkBuffer cameraUniformBuffer, AccelerationStructure &topLevelAccelerationStructure,
                           std::vector<VkBuffer> indexBufferList, std::vector<VkBuffer> vertexBufferList, std::vector<size_t> indexBufferSizeList, std::vector<size_t> vertexBufferSizeList, 
                           ImageCreator raysNoiseImage, VkImageView indirectDiffuseImageView, ImageCreator blueNoiseImage, VkImageView surfelsVisualizationImageView);
    void cleanupDescriptors(VkDevice device);

    VkDescriptorSetLayout getGeometryDescriptorSetLayout();
    VkDescriptorSetLayout getShadowMappingDescriptorSetLayout();
    VkDescriptorSetLayout getGBufferDescriptorSetLayout();
    VkDescriptorSetLayout getSSAODescriptorSetLayout();
    VkDescriptorSetLayout getSSAOBlurDescriptorSetLayout();
    VkDescriptorSetLayout getSSAOCompositionDescriptorSetLayout();
    VkDescriptorSetLayout getShadowsSSAOCompositionDescriptorSetLayout();
    VkDescriptorSetLayout getRaytracingDescriptorSetLayout();
    VkDescriptorSetLayout getSurfelsGenerationDescriptorSetLayout();
    VkDescriptorSetLayout getSurfelsVisualizationDescriptorSetLayout();
    VkDescriptorSetLayout getSurfelsRadianceCalculationDescriptorSetLayout();
    VkDescriptorSetLayout getSurfelsIndirectLightingDescriptorSetLayout();
    VkDescriptorSetLayout getSurfelsCompositionDescriptorSetLayout();

    VkDescriptorSet getGeometryDescriptor(int index);
    VkDescriptorSet getShadowMappingDescriptor(int index);
    VkDescriptorSet getGBufferDescriptor(int index);
    VkDescriptorSet getSSAODescriptor(int index);
    VkDescriptorSet getSSAOBlurDescriptor(int index);
    VkDescriptorSet getSSAOCompositionDescriptor(int index);
    VkDescriptorSet getShadowsSSAOCompositionDescriptor(int index);
    VkDescriptorSet getRaytracingDescriptor(int index);
    VkDescriptorSet getSurfelsGenerationDescriptor(int index);
    VkDescriptorSet getSurfelsVisualizationDescriptor(int index);
    VkDescriptorSet getSurfelsRadianceCalculationDescriptor(int index);
    VkDescriptorSet getSurfelsIndirectLightingDescriptor(int index);
    VkDescriptorSet getSurfelsCompositionDescriptor(int index);
};