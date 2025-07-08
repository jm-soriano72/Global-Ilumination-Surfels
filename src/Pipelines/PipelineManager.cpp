#include "PipelineManager.h"

#include "Scene/Models/MeshLoader.h"
#include "Render_Passes/Utils/DepthBuffer.h"
#include "Config.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

void PipelineManager::createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout geometryDescriptorSetLayout, VkRenderPass geometryRenderPass,
                                      VkDescriptorSetLayout shadowMappingDescriptorSetLayout, VkRenderPass shadowMappingRenderPass)
{
    shadowMappingPipeline.createGraphicsPipeline(device, swapChainExtent, shadowMappingDescriptorSetLayout, shadowMappingRenderPass);
    geometryPipeline.createGraphicsPipeline(device, swapChainExtent, geometryDescriptorSetLayout, geometryRenderPass);
}

void PipelineManager::createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout gBufferDescriptorSetLayout, VkRenderPass gBufferRenderPass,
                                      VkDescriptorSetLayout ssaoDescriptorSetLayout, VkRenderPass ssaoRenderPass, VkDescriptorSetLayout ssaoBlurDescriptorSetLayout,
                                      VkRenderPass ssaoBlurRenderPass, VkDescriptorSetLayout ssaoCompositionDescriptorSetLayout, VkRenderPass ssaoCompositionRenderPass)
{
    gBufferPipeline.createGraphicsPipeline(device, swapChainExtent, gBufferDescriptorSetLayout, gBufferRenderPass);
    ssaoPipeline.createGraphicsPipeline(device, swapChainExtent, ssaoDescriptorSetLayout, ssaoRenderPass);
    ssaoBlurPipeline.createGraphicsPipeline(device, swapChainExtent, ssaoBlurDescriptorSetLayout, ssaoBlurRenderPass);
    ssaoCompositionPipeline.createGraphicsPipeline(device, swapChainExtent, ssaoCompositionDescriptorSetLayout, ssaoCompositionRenderPass);
}

void PipelineManager::createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout gBufferDescriptorSetLayout, VkRenderPass gBufferRenderPass,
                                      VkDescriptorSetLayout ssaoDescriptorSetLayout, VkRenderPass ssaoRenderPass, VkDescriptorSetLayout ssaoBlurDescriptorSetLayout,
                                      VkRenderPass ssaoBlurRenderPass, VkDescriptorSetLayout ssaoCompositionDescriptorSetLayout, VkRenderPass ssaoCompositionRenderPass,
                                      VkDescriptorSetLayout shadowMappingDescriptorSetLayout, VkRenderPass shadowMappingRenderPass)
{
    shadowMappingPipeline.createGraphicsPipeline(device, swapChainExtent, shadowMappingDescriptorSetLayout, shadowMappingRenderPass);
    gBufferPipeline.createGraphicsPipeline(device, swapChainExtent, gBufferDescriptorSetLayout, gBufferRenderPass);
    ssaoPipeline.createGraphicsPipeline(device, swapChainExtent, ssaoDescriptorSetLayout, ssaoRenderPass);
    ssaoBlurPipeline.createGraphicsPipeline(device, swapChainExtent, ssaoBlurDescriptorSetLayout, ssaoBlurRenderPass);
    ssaoCompositionPipeline.createGraphicsPipeline(device, swapChainExtent, ssaoCompositionDescriptorSetLayout, ssaoCompositionRenderPass);
}

void PipelineManager::createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout raytracingDescriptorSetLayout, VkRenderPass raytracingRenderPass)
{
    raytracingPipeline.createGraphicsPipeline(device, swapChainExtent, raytracingDescriptorSetLayout, raytracingRenderPass);
}

void PipelineManager::createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout gBufferDescriptorSetLayout, VkRenderPass gBufferRenderPass,
                                      VkDescriptorSetLayout ssaoDescriptorSetLayout, VkRenderPass ssaoRenderPass, VkDescriptorSetLayout ssaoBlurDescriptorSetLayout,
                                      VkRenderPass ssaoBlurRenderPass, VkDescriptorSetLayout surfelsCompositionDescriptorSetLayout, VkRenderPass surfelsCompositionRenderPass,
                                      VkDescriptorSetLayout shadowMappingDescriptorSetLayout, VkRenderPass shadowMappingRenderPass, VkDescriptorSetLayout surfelsGenerationDescriptorSetLayout,
                                      VkRenderPass surfelsVisualizationRenderPass, VkDescriptorSetLayout surfelsVisualizationDescriptorSetLayout, VkDescriptorSetLayout surfelsRadianceCalculationDescriptorSetLayout,
                                      VkRenderPass surfelsIndirectLightingRenderPass, VkDescriptorSetLayout surfelsIndirectLightingDescriptorSetLayout)
{
    shadowMappingPipeline.createGraphicsPipeline(device, swapChainExtent, shadowMappingDescriptorSetLayout, shadowMappingRenderPass);
    gBufferPipeline.createGraphicsPipeline(device, swapChainExtent, gBufferDescriptorSetLayout, gBufferRenderPass);
    ssaoPipeline.createGraphicsPipeline(device, swapChainExtent, ssaoDescriptorSetLayout, ssaoRenderPass);
    ssaoBlurPipeline.createGraphicsPipeline(device, swapChainExtent, ssaoBlurDescriptorSetLayout, ssaoBlurRenderPass);
    surfelsCompositionPipeline.createGraphicsPipeline(device, swapChainExtent, surfelsCompositionDescriptorSetLayout, surfelsCompositionRenderPass);

    surfelsGenerationPipeline.createGraphicsPipeline(device, surfelsGenerationDescriptorSetLayout);
    surfelsVisualizationPipeline.createGraphicsPipeline(device, swapChainExtent, surfelsVisualizationDescriptorSetLayout, surfelsVisualizationRenderPass);
    surfelsRadianceCalculationPipeline.createGraphicsPipeline(device, surfelsRadianceCalculationDescriptorSetLayout);
    surfelsIndirectLightingPipeline.createGraphicsPipeline(device, swapChainExtent, surfelsIndirectLightingDescriptorSetLayout, surfelsIndirectLightingRenderPass);
}

void PipelineManager::cleanup(VkDevice device)
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        geometryPipeline.cleanup(device);
        shadowMappingPipeline.cleanup(device);
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        gBufferPipeline.cleanup(device);
        ssaoPipeline.cleanup(device);
        ssaoBlurPipeline.cleanup(device);
        ssaoCompositionPipeline.cleanup(device);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        shadowMappingPipeline.cleanup(device);
        gBufferPipeline.cleanup(device);
        ssaoPipeline.cleanup(device);
        ssaoBlurPipeline.cleanup(device);
        ssaoCompositionPipeline.cleanup(device);
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        raytracingPipeline.cleanup(device);
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        shadowMappingPipeline.cleanup(device);
        gBufferPipeline.cleanup(device);
        ssaoPipeline.cleanup(device);
        ssaoBlurPipeline.cleanup(device);
        surfelsCompositionPipeline.cleanup(device);
        surfelsGenerationPipeline.cleanup(device);
        surfelsVisualizationPipeline.cleanup(device);
        surfelsRadianceCalculationPipeline.cleanup(device);
        surfelsIndirectLightingPipeline.cleanup(device);
    }
}

VkPipelineLayout PipelineManager::getGeometryPipelineLayout()
{
    return geometryPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getGeometryPipeline()
{
    return geometryPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getShadowMappingPipelineLayout()
{
    return shadowMappingPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getShadowMappingPipeline()
{
    return shadowMappingPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getGBufferPipelineLayout()
{
    return gBufferPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getGBufferPipeline()
{
    return gBufferPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getSSAOPipelineLayout()
{
    return ssaoPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getSSAOPipeline()
{
    return ssaoPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getSSAOBlurPipelineLayout()
{
    return ssaoBlurPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getSSAOBlurPipeline()
{
    return ssaoBlurPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getSSAOCompositionPipelineLayout()
{
    return ssaoCompositionPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getSSAOCompositionPipeline()
{
    return ssaoCompositionPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getRaytracingPipelineLayout()
{
    return raytracingPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getRaytracingPipeline()
{
    return raytracingPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getSurfelsGenerationPipelineLayout()
{
    return surfelsGenerationPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getSurfelsGenerationPipeline()
{
    return surfelsGenerationPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getSurfelsVisualizationPipelineLayout()
{
    return surfelsVisualizationPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getSurfelsVisualizationPipeline()
{
    return surfelsVisualizationPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getSurfelsRadianceCalculationPipelineLayout()
{
    return surfelsRadianceCalculationPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getSurfelsRadianceCalculationPipeline()
{
    return surfelsRadianceCalculationPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getSurfelsIndirectLightingPipelineLayout()
{
    return surfelsIndirectLightingPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getSurfelsIndirectLightingPipeline()
{
    return surfelsIndirectLightingPipeline.getGraphicsPipeline();
}

VkPipelineLayout PipelineManager::getSurfelsCompositionPipelineLayout()
{
    return surfelsCompositionPipeline.getPipelineLayout();
}

VkPipeline PipelineManager::getSurfelsCompositionPipeline()
{
    return surfelsCompositionPipeline.getGraphicsPipeline();
}