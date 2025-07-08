#pragma once

#include "Scene/Models/MeshLoader.h"
#include "Render_Passes/Utils/DepthBuffer.h"
#include "GeometryPipeline.h"
#include "ShadowMappingPipeline.h"
#include "GBufferPipeline.h"
#include "SSAOPipeline.h"
#include "SSAOBlurPipeline.h"
#include "SSAOCompositionPipeline.h"
#include "RaytracingPipeline.h"
#include "SurfelsGenerationPipeline.h"
#include "SurfelsVisualizationPipeline.h"
#include "SurfelsRadianceCalculationPipeline.h"
#include "IndirectDiffuseShadingPipeline.h"
#include "SurfelsCompositionPipeline.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

class PipelineManager
{
private:
    GeometryPipeline geometryPipeline;
    ShadowMappingPipeline shadowMappingPipeline;

    GBufferPipeline gBufferPipeline;
    SSAOPipeline ssaoPipeline;
    SSAOBlurPipeline ssaoBlurPipeline;
    SSAOCompositionPipeline ssaoCompositionPipeline;

    RaytracingPipeline raytracingPipeline;

    SurfelsGenerationPipeline surfelsGenerationPipeline;
    SurfelsVisualizationPipeline surfelsVisualizationPipeline;
    SurfelsRadianceCalculationPipeline surfelsRadianceCalculationPipeline;
    IndirectDiffuseShadingPipeline surfelsIndirectLightingPipeline;
    SurfelsCompositionPipeline surfelsCompositionPipeline;

public:
    void createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout geometryDescriptorSetLayout, VkRenderPass geometryRenderPass,
                         VkDescriptorSetLayout shadowMappingDescriptorSetLayout, VkRenderPass shadowMappingRenderPass);
    void createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout gBufferDescriptorSetLayout, VkRenderPass gBufferRenderPass,
                         VkDescriptorSetLayout ssaoDescriptorSetLayout, VkRenderPass ssaoRenderPass, VkDescriptorSetLayout ssaoBlurDescriptorSetLayout,
                         VkRenderPass ssaoBlurRenderPass, VkDescriptorSetLayout ssaoCompositionDescriptorSetLayout, VkRenderPass ssaoCompositionRenderPass);
    void createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout gBufferDescriptorSetLayout, VkRenderPass gBufferRenderPass,
                         VkDescriptorSetLayout ssaoDescriptorSetLayout, VkRenderPass ssaoRenderPass, VkDescriptorSetLayout ssaoBlurDescriptorSetLayout,
                         VkRenderPass ssaoBlurRenderPass, VkDescriptorSetLayout ssaoCompositionDescriptorSetLayout, VkRenderPass ssaoCompositionRenderPass,
                         VkDescriptorSetLayout shadowMappingDescriptorSetLayout, VkRenderPass shadowMappingRenderPass);
    void createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout raytracingDescriptorSetLayout, VkRenderPass raytracingRenderPass);
    void createPipelines(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout gBufferDescriptorSetLayout, VkRenderPass gBufferRenderPass,
                         VkDescriptorSetLayout ssaoDescriptorSetLayout, VkRenderPass ssaoRenderPass, VkDescriptorSetLayout ssaoBlurDescriptorSetLayout,
                         VkRenderPass ssaoBlurRenderPass, VkDescriptorSetLayout surfelsCompositionDescriptorSetLayout, VkRenderPass surfelsCompositionRenderPass,
                         VkDescriptorSetLayout shadowMappingDescriptorSetLayout, VkRenderPass shadowMappingRenderPass, VkDescriptorSetLayout surfelsGenerationDescriptorSetLayout,
                         VkRenderPass surfelsVisualizationRenderPass, VkDescriptorSetLayout surfelsVisualizationDescriptorSetLayout, VkDescriptorSetLayout surfelsRadianceCalculationDescriptorSetLayout,
                         VkRenderPass surfelsIndirectLightingRenderPass, VkDescriptorSetLayout surfelsIndirectLightingDescriptorSetLayout);

    void cleanup(VkDevice device);

    VkPipelineLayout getGeometryPipelineLayout();
    VkPipeline getGeometryPipeline();
    VkPipelineLayout getShadowMappingPipelineLayout();
    VkPipeline getShadowMappingPipeline();

    VkPipelineLayout getGBufferPipelineLayout();
    VkPipeline getGBufferPipeline();
    VkPipelineLayout getSSAOPipelineLayout();
    VkPipeline getSSAOPipeline();
    VkPipelineLayout getSSAOBlurPipelineLayout();
    VkPipeline getSSAOBlurPipeline();
    VkPipelineLayout getSSAOCompositionPipelineLayout();
    VkPipeline getSSAOCompositionPipeline();

    VkPipelineLayout getRaytracingPipelineLayout();
    VkPipeline getRaytracingPipeline();

    VkPipelineLayout getSurfelsGenerationPipelineLayout();
    VkPipeline getSurfelsGenerationPipeline();
    VkPipelineLayout getSurfelsVisualizationPipelineLayout();
    VkPipeline getSurfelsVisualizationPipeline();
    VkPipelineLayout getSurfelsRadianceCalculationPipelineLayout();
    VkPipeline getSurfelsRadianceCalculationPipeline();
    VkPipelineLayout getSurfelsIndirectLightingPipelineLayout();
    VkPipeline getSurfelsIndirectLightingPipeline();
    VkPipelineLayout getSurfelsCompositionPipelineLayout();
    VkPipeline getSurfelsCompositionPipeline();
    VkPipelineLayout getHorizontalBlurPipelineLayout();
    VkPipeline getHorizontalBlurPipeline();
    VkPipelineLayout getVerticalBlurPipelineLayout();
    VkPipeline getVerticalBlurPipeline();
};