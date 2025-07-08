#pragma once

#include "GeometryPass.h"
#include "ShadowMappingPass.h"
#include "GBufferPass.h"
#include "SSAOPass.h"
#include "SSAOBlurPass.h"
#include "SSAOCompositionPass.h"
#include "RaytracingPass.h"
#include "SurfelsVisualizationPass.h"
#include "IndirectDiffusePass.h"
#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

class RenderPassesManager
{
private:
    ShadowMappingPass shadowMappingPassManager;
    GeometryPass geometryPassManager;

    GBufferPass gBufferPassManager;
    SSAOPass ssaoPassManager;
    SSAOBlurPass ssaoBlurPassManager;
    SSAOCompositionPass ssaoCompositionPassManager;

    RaytracingPass raytracingPassManager;

    SurfelsVisualizationPass surfelsVisualizationPassManager;
    IndirectDiffusePass indirectDiffusePassManager;

public:
    void createRenderPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat geometryPassFormat);
    void createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                            VkQueue graphicsQueue, VkExtent2D swapChainExtent);

    VkRenderPass getGeometryRenderPass();
    VkRenderPass getShadowMappingRenderPass();
    VkRenderPass getGBufferRenderPass();
    VkRenderPass getSSAORenderPass();
    VkRenderPass getSSAOBlurRenderPass();
    VkRenderPass getSSAOCompositionRenderPass();
    VkRenderPass getRaytracingRenderPass();
    VkRenderPass getSurfelsVisualizationRenderPass();
    VkRenderPass getIndirectDiffuseRenderPass();

    VkFramebuffer getGeometryFramebuffer(int index);
    VkFramebuffer getShadowMappingFramebuffer(int index);
    VkFramebuffer getGBufferFramebuffer(int index);
    VkFramebuffer getSSAOFramebuffer(int index);
    VkFramebuffer getSSAOBlurFramebuffer(int index);
    VkFramebuffer getSSAOCompositionFramebuffer(int index);
    VkFramebuffer getRaytracingFramebuffer(int index);
    VkFramebuffer getSurfelsVisualizationFramebuffer(int index);
    VkFramebuffer getIndirectDiffuseFramebuffer(int index);

    VkSampler getDepthSampler();
    VkImageView getDepthImageView();
    std::vector<VkFramebuffer> getSwapChainFramebuffers();
    DepthBuffer getSwapChainDepthBufferCreator();

    VkImageView getGBufferPositionImageView();
    VkImageView getGBufferNormalImageView();
    VkImageView getGBufferAlbedoImageView();
    VkImageView getGBufferDepthImageView();
    VkImageView getGBufferSpecularImageView();
    VkImageView getSSAOColorImageView();
    VkImageView getSSAOBlurColorImageView();
    VkImageView getSurfelsColorImageView();
    VkImageView getIndirectDiffuseImageView();

    ImageCreator getNoiseTexture();
    ImageCreator getIndirectDiffuseImage();

    void cleanup(VkDevice device);
    void cleanupFramebuffers(VkDevice device);
};