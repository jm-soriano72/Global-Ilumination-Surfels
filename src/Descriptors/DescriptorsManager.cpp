#include "DescriptorsManager.h"

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "GeometryDescriptors.h"
#include "ShadowMappingDescriptors.h"
#include "Config.h"

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

DescriptorsManager::DescriptorsManager() {}

void DescriptorsManager::createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<ImageCreator> &diffuseImageCreators,
                                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, VkImageView depthImageView, VkSampler depthSampler,
                                           std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffers, std::vector<VkBuffer> uniformShadowBuffers)
{
    shadowMappingDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, uniformShadowBuffers);
    geometryDescriptors.createDescriptors(device, numTextures, numMaterials, MAX_FRAMES_IN_FLIGHT, diffuseImageCreators, alphaImageCreators, specularImageCreators, depthImageView, depthSampler,
                                          uniformMVPBuffers, lightBuffers, mainLightDataBuffers);
}

void DescriptorsManager::createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t numTextures, uint32_t numMaterials, std::vector<ImageCreator> &diffuseImageCreators,
                                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, std::vector<VkBuffer> gUniformBuffers,
                                           std::vector<VkBuffer> ssaoProjUniformBuffers, std::vector<VkBuffer> ssaoParamsUniformBuffers, VkImageView positionImageView, VkImageView normalImageView,
                                           VkImageView albedoImageView, VkImageView colorSSAOImageView, VkImageView colorSSAOBlurImageView, ImageCreator noiseTexture)
{
    colorSampler = getSSAOColorSampler(device);

    gBufferDescriptors.createDescriptors(device, numTextures, numMaterials, MAX_FRAMES_IN_FLIGHT, gUniformBuffers, diffuseImageCreators, alphaImageCreators, specularImageCreators);
    ssaoDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, ssaoProjUniformBuffers, ssaoParamsUniformBuffers, colorSampler, positionImageView, normalImageView, noiseTexture);
    ssaoBlurDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, colorSampler, colorSSAOImageView);
    ssaoCompositionDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, ssaoParamsUniformBuffers, colorSampler, positionImageView, normalImageView, albedoImageView,
                                                 colorSSAOBlurImageView);
}

void DescriptorsManager::createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t numTextures, uint32_t numMaterials, std::vector<ImageCreator> &diffuseImageCreators,
                                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, std::vector<VkBuffer> gUniformBuffers,
                                           std::vector<VkBuffer> ssaoProjUniformBuffers, std::vector<VkBuffer> ssaoParamsUniformBuffers, VkImageView positionImageView, VkImageView normalImageView,
                                           VkImageView albedoImageView, VkImageView colorSSAOImageView, VkImageView colorSSAOBlurImageView, ImageCreator noiseTexture, std::vector<VkBuffer> uniformShadowBuffers,
                                           VkImageView depthImageView, VkSampler depthSampler, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffer, std::vector<VkBuffer> uniformMVPBuffers,
                                           VkImageView specularImageView)
{
    shadowMappingDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, uniformShadowBuffers);

    colorSampler = getSSAOColorSampler(device);

    gBufferDescriptors.createDescriptors(device, numTextures, numMaterials, MAX_FRAMES_IN_FLIGHT, gUniformBuffers, diffuseImageCreators, alphaImageCreators, specularImageCreators);
    ssaoDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, ssaoProjUniformBuffers, ssaoParamsUniformBuffers, colorSampler, positionImageView, normalImageView, noiseTexture);
    ssaoBlurDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, colorSampler, colorSSAOImageView);
    shadowsSSAOCompositionDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, ssaoParamsUniformBuffers, colorSampler, positionImageView, normalImageView, albedoImageView,
                                                        colorSSAOBlurImageView, depthImageView, depthSampler, lightBuffers, mainLightDataBuffer, uniformMVPBuffers, specularImageView);
}

void DescriptorsManager::createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers,
                                           AccelerationStructure &topLevelAccelerationStructure, std::vector<ImageCreator> &diffuseImageCreators, std::vector<ImageCreator> &alphaImageCreators,
                                           std::vector<ImageCreator> &specularImageCreators)
{
    raytracingDescriptors.createDescriptors(device, numTextures, numMaterials, MAX_FRAMES_IN_FLIGHT, uniformMVPBuffers, lightBuffers,
                                            topLevelAccelerationStructure, diffuseImageCreators, alphaImageCreators, specularImageCreators);
}

void DescriptorsManager::createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t numTextures, uint32_t numMaterials, std::vector<ImageCreator> &diffuseImageCreators,
                                           std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, std::vector<VkBuffer> gUniformBuffers,
                                           std::vector<VkBuffer> ssaoProjUniformBuffers, std::vector<VkBuffer> ssaoParamsUniformBuffers, VkImageView positionImageView, VkImageView normalImageView,
                                           VkImageView albedoImageView, VkImageView colorSSAOImageView, VkImageView colorSSAOBlurImageView, ImageCreator noiseTexture, std::vector<VkBuffer> uniformShadowBuffers,
                                           VkImageView depthImageView, VkSampler depthSampler, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffer,
                                           std::vector<VkBuffer> uniformMVPBuffers, VkImageView specularImageView, VkBuffer surfelBuffer, VkBuffer surfelStatsBuffer, VkBuffer surfelGridBuffer,
                                           VkBuffer surfelCellBuffer, VkBuffer cameraUniformBuffer, AccelerationStructure &topLevelAccelerationStructure,
                                           std::vector<VkBuffer> indexBufferList, std::vector<VkBuffer> vertexBufferList, std::vector<size_t> indexBufferSizeList, std::vector<size_t> vertexBufferSizeList, 
                                           ImageCreator raysNoiseImage, VkImageView indirectDiffuseImageView, ImageCreator blueNoiseImage, VkImageView surfelsVisualizationImageView)
{
    shadowMappingDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, uniformShadowBuffers);

    colorSampler = getSSAOColorSampler(device);

    gBufferDescriptors.createDescriptors(device, numTextures, numMaterials, MAX_FRAMES_IN_FLIGHT, gUniformBuffers, diffuseImageCreators, alphaImageCreators, specularImageCreators);
    surfelsGenerationDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, surfelBuffer, surfelStatsBuffer, surfelGridBuffer, surfelCellBuffer, cameraUniformBuffer,
                                                   normalImageView, positionImageView, albedoImageView, blueNoiseImage);
    surfelsVisualizationDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, surfelBuffer, surfelGridBuffer, surfelCellBuffer, cameraUniformBuffer, positionImageView);
    surfelsRadianceCalculationDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, surfelBuffer, lightBuffers, topLevelAccelerationStructure, indexBufferList, vertexBufferList,
                                                            indexBufferSizeList, vertexBufferSizeList, numTextures, numMaterials, diffuseImageCreators, alphaImageCreators, specularImageCreators,
                                                            raysNoiseImage);
    surfelsIndirectShadingDescriptors.createDescriptors(device, topLevelAccelerationStructure, MAX_FRAMES_IN_FLIGHT, surfelBuffer, surfelGridBuffer, surfelCellBuffer, cameraUniformBuffer,
                                                        positionImageView, normalImageView);
    ssaoDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, ssaoProjUniformBuffers, ssaoParamsUniformBuffers, colorSampler, positionImageView, normalImageView, noiseTexture);
    ssaoBlurDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, colorSampler, colorSSAOImageView);
    surfelsCompositionDescriptors.createDescriptors(device, MAX_FRAMES_IN_FLIGHT, ssaoParamsUniformBuffers, colorSampler, positionImageView, normalImageView, albedoImageView,
                                                    colorSSAOBlurImageView, depthImageView, depthSampler, lightBuffers, mainLightDataBuffer, uniformMVPBuffers, specularImageView,
                                                    indirectDiffuseImageView, surfelsVisualizationImageView);
}

VkSampler DescriptorsManager::getSSAOColorSampler(VkDevice device)
{
    VkSampler colorSampler;
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vkCreateSampler(device, &samplerInfo, nullptr, &colorSampler);

    return colorSampler;
}

void DescriptorsManager::cleanupDescriptors(VkDevice device)
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        shadowMappingDescriptors.cleanupDescriptors(device);
        geometryDescriptors.cleanupDescriptors(device);
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        gBufferDescriptors.cleanupDescriptors(device);
        ssaoDescriptors.cleanupDescriptors(device);
        ssaoBlurDescriptors.cleanupDescriptors(device);
        ssaoCompositionDescriptors.cleanupDescriptors(device);
        vkDestroySampler(device, colorSampler, nullptr);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        shadowMappingDescriptors.cleanupDescriptors(device);
        gBufferDescriptors.cleanupDescriptors(device);
        ssaoDescriptors.cleanupDescriptors(device);
        ssaoBlurDescriptors.cleanupDescriptors(device);
        shadowsSSAOCompositionDescriptors.cleanupDescriptors(device);
        vkDestroySampler(device, colorSampler, nullptr);
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        raytracingDescriptors.cleanupDescriptors(device);
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        shadowMappingDescriptors.cleanupDescriptors(device);
        gBufferDescriptors.cleanupDescriptors(device);
        surfelsGenerationDescriptors.cleanupDescriptors(device);
        surfelsVisualizationDescriptors.cleanupDescriptors(device);
        surfelsRadianceCalculationDescriptors.cleanupDescriptors(device);
        surfelsIndirectShadingDescriptors.cleanupDescriptors(device);
        ssaoDescriptors.cleanupDescriptors(device);
        ssaoBlurDescriptors.cleanupDescriptors(device);
        surfelsCompositionDescriptors.cleanupDescriptors(device);
        vkDestroySampler(device, colorSampler, nullptr);
    }
}

VkDescriptorSetLayout DescriptorsManager::getGeometryDescriptorSetLayout()
{
    return geometryDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getShadowMappingDescriptorSetLayout()
{
    return shadowMappingDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getGBufferDescriptorSetLayout()
{
    return gBufferDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getSSAODescriptorSetLayout()
{
    return ssaoDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getSSAOBlurDescriptorSetLayout()
{
    return ssaoBlurDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getSSAOCompositionDescriptorSetLayout()
{
    return ssaoCompositionDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getShadowsSSAOCompositionDescriptorSetLayout()
{
    return shadowsSSAOCompositionDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getRaytracingDescriptorSetLayout()
{
    return raytracingDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getSurfelsGenerationDescriptorSetLayout()
{
    return surfelsGenerationDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getSurfelsVisualizationDescriptorSetLayout()
{
    return surfelsVisualizationDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getSurfelsRadianceCalculationDescriptorSetLayout()
{
    return surfelsRadianceCalculationDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getSurfelsIndirectLightingDescriptorSetLayout()
{
    return surfelsIndirectShadingDescriptors.getDescriptorSetLayout();
}

VkDescriptorSetLayout DescriptorsManager::getSurfelsCompositionDescriptorSetLayout()
{
    return surfelsCompositionDescriptors.getDescriptorSetLayout();
}

VkDescriptorSet DescriptorsManager::getGeometryDescriptor(int index)
{
    return geometryDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getShadowMappingDescriptor(int index)
{
    return shadowMappingDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getGBufferDescriptor(int index)
{
    return gBufferDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getSSAODescriptor(int index)
{
    return ssaoDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getSSAOBlurDescriptor(int index)
{
    return ssaoBlurDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getSSAOCompositionDescriptor(int index)
{
    return ssaoCompositionDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getShadowsSSAOCompositionDescriptor(int index)
{
    return shadowsSSAOCompositionDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getRaytracingDescriptor(int index)
{
    return raytracingDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getSurfelsGenerationDescriptor(int index)
{
    return surfelsGenerationDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getSurfelsVisualizationDescriptor(int index)
{
    return surfelsVisualizationDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getSurfelsRadianceCalculationDescriptor(int index)
{
    return surfelsRadianceCalculationDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getSurfelsIndirectLightingDescriptor(int index)
{
    return surfelsIndirectShadingDescriptors.getDescriptorSet(index);
}

VkDescriptorSet DescriptorsManager::getSurfelsCompositionDescriptor(int index)
{
    return surfelsCompositionDescriptors.getDescriptorSet(index);
}
