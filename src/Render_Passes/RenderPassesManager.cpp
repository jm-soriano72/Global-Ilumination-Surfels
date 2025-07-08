#include "RenderPassesManager.h"

#include "GeometryPass.h"
#include "ShadowMappingPass.h"
#include "Config.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

void RenderPassesManager::createRenderPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat endPassFormat)
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        shadowMappingPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        geometryPassManager.createRenderPass(device, endPassFormat, physicalDevice);
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        gBufferPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoBlurPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoCompositionPassManager.createRenderPass(device, endPassFormat, physicalDevice);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        shadowMappingPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        gBufferPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoBlurPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoCompositionPassManager.createRenderPass(device, endPassFormat, physicalDevice);
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        shadowMappingPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        gBufferPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        surfelsVisualizationPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        indirectDiffusePassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoBlurPassManager.createRenderPass(device, endPassFormat, physicalDevice);
        ssaoCompositionPassManager.createRenderPass(device, endPassFormat, physicalDevice);
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        raytracingPassManager.createRenderPass(device, endPassFormat, physicalDevice);
    }
}

void RenderPassesManager::createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                                             VkQueue graphicsQueue, VkExtent2D swapChainExtent)
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        shadowMappingPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        geometryPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        gBufferPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoBlurPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoCompositionPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        shadowMappingPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        gBufferPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoBlurPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoCompositionPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        shadowMappingPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        gBufferPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        surfelsVisualizationPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        indirectDiffusePassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoBlurPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
        ssaoCompositionPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        raytracingPassManager.createFramebuffers(numImageViews, swapChainManager, device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
    }
}

VkRenderPass RenderPassesManager::getGeometryRenderPass()
{
    return geometryPassManager.getRenderPass();
}

VkRenderPass RenderPassesManager::getShadowMappingRenderPass()
{
    return shadowMappingPassManager.getRenderPass();
}

VkRenderPass RenderPassesManager::getGBufferRenderPass()
{
    return gBufferPassManager.getRenderPass();
}

VkRenderPass RenderPassesManager::getSSAORenderPass()
{
    return ssaoPassManager.getRenderPass();
}

VkRenderPass RenderPassesManager::getSSAOBlurRenderPass()
{
    return ssaoBlurPassManager.getRenderPass();
}

VkRenderPass RenderPassesManager::getSSAOCompositionRenderPass()
{
    return ssaoCompositionPassManager.getRenderPass();
}

VkRenderPass RenderPassesManager::getRaytracingRenderPass()
{
    return raytracingPassManager.getRenderPass();
}

VkRenderPass RenderPassesManager::getSurfelsVisualizationRenderPass()
{
    return surfelsVisualizationPassManager.getRenderPass();
}

VkRenderPass RenderPassesManager::getIndirectDiffuseRenderPass()
{
    return indirectDiffusePassManager.getRenderPass();
}

VkFramebuffer RenderPassesManager::getGeometryFramebuffer(int index)
{
    return geometryPassManager.getFramebuffer(index);
}

VkFramebuffer RenderPassesManager::getShadowMappingFramebuffer(int index)
{
    return shadowMappingPassManager.getFramebuffer(index);
}

VkFramebuffer RenderPassesManager::getGBufferFramebuffer(int index)
{
    return gBufferPassManager.getFramebuffer(index);
}

VkFramebuffer RenderPassesManager::getSSAOFramebuffer(int index)
{
    return ssaoPassManager.getFramebuffer(index);
}

VkFramebuffer RenderPassesManager::getSSAOBlurFramebuffer(int index)
{
    return ssaoBlurPassManager.getFramebuffer(index);
}

VkFramebuffer RenderPassesManager::getSSAOCompositionFramebuffer(int index)
{
    return ssaoCompositionPassManager.getFramebuffer(index);
}

VkFramebuffer RenderPassesManager::getRaytracingFramebuffer(int index)
{
    return raytracingPassManager.getFramebuffer(index);
}

VkFramebuffer RenderPassesManager::getSurfelsVisualizationFramebuffer(int index)
{
    return surfelsVisualizationPassManager.getFramebuffer(index);
}

VkFramebuffer RenderPassesManager::getIndirectDiffuseFramebuffer(int index)
{
    return indirectDiffusePassManager.getFramebuffer(index);
}

VkSampler RenderPassesManager::getDepthSampler()
{
    return shadowMappingPassManager.getDepthSampler();
}

VkImageView RenderPassesManager::getDepthImageView()
{
    return shadowMappingPassManager.getDepthImageView();
}

std::vector<VkFramebuffer> RenderPassesManager::getSwapChainFramebuffers()
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF) {
        return geometryPassManager.getFramebuffers();
    }
    else if (renderConfig == RenderMode::SSAO || renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION) {
        return ssaoCompositionPassManager.getFramebuffers();
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS) {
        return raytracingPassManager.getFramebuffers();
    }
}

DepthBuffer RenderPassesManager::getSwapChainDepthBufferCreator()
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF) {
        return geometryPassManager.getDepthBufferCreator();
    }
    else if (renderConfig == RenderMode::SSAO || renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION) {
        return ssaoCompositionPassManager.getDepthBufferCreator();
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS) {
        return raytracingPassManager.getDepthBufferCreator();
    }
}

VkImageView RenderPassesManager::getGBufferPositionImageView()
{
    return gBufferPassManager.getPositionImageView();
}

VkImageView RenderPassesManager::getGBufferNormalImageView()
{
    return gBufferPassManager.getNormalImageView();
}

VkImageView RenderPassesManager::getGBufferAlbedoImageView()
{
    return gBufferPassManager.getAlbedoImageView();
}

VkImageView RenderPassesManager::getGBufferSpecularImageView()
{
    return gBufferPassManager.getSpecularImageView();
}

VkImageView RenderPassesManager::getGBufferDepthImageView()
{
    return gBufferPassManager.getDepthBufferCreator().depthImageView;
}

VkImageView RenderPassesManager::getSSAOColorImageView()
{
    return ssaoPassManager.getColorImageView();
}

VkImageView RenderPassesManager::getSSAOBlurColorImageView()
{
    return ssaoBlurPassManager.getColorImageView();
}

VkImageView RenderPassesManager::getSurfelsColorImageView()
{
    return surfelsVisualizationPassManager.getColorImageView();
}

VkImageView RenderPassesManager::getIndirectDiffuseImageView()
{
    return indirectDiffusePassManager.getColorImageView();
}

ImageCreator RenderPassesManager::getNoiseTexture()
{
    return ssaoPassManager.getNoiseImage();
}

ImageCreator RenderPassesManager::getIndirectDiffuseImage()
{
    return indirectDiffusePassManager.getColorImage();
}

void RenderPassesManager::cleanup(VkDevice device)
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        shadowMappingPassManager.cleanup(device);
        geometryPassManager.cleanup(device);
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        gBufferPassManager.cleanup(device);
        ssaoPassManager.cleanup(device);
        ssaoBlurPassManager.cleanup(device);
        ssaoCompositionPassManager.cleanup(device);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        shadowMappingPassManager.cleanup(device);
        gBufferPassManager.cleanup(device);
        ssaoPassManager.cleanup(device);
        ssaoBlurPassManager.cleanup(device);
        ssaoCompositionPassManager.cleanup(device);
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        shadowMappingPassManager.cleanup(device);
        gBufferPassManager.cleanup(device);
        surfelsVisualizationPassManager.cleanup(device);
        indirectDiffusePassManager.cleanup(device);
        ssaoPassManager.cleanup(device);
        ssaoBlurPassManager.cleanup(device);
        ssaoCompositionPassManager.cleanup(device);
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        raytracingPassManager.cleanup(device);
    }
}

void RenderPassesManager::cleanupFramebuffers(VkDevice device) {
    if (renderConfig != RenderMode::SSAO && renderConfig != RenderMode::RAYTRACING_BASE_SHADOWS) {
        shadowMappingPassManager.cleanupFramebuffers(device);
    } 
    if (renderConfig == RenderMode::SSAO || renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        gBufferPassManager.cleanupFramebuffers(device);
        ssaoPassManager.cleanupFramebuffers(device);
        ssaoBlurPassManager.cleanupFramebuffers(device);
    }
    if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        gBufferPassManager.cleanupFramebuffers(device);
        surfelsVisualizationPassManager.cleanupFramebuffers(device);
        indirectDiffusePassManager.cleanupFramebuffers(device);
        ssaoPassManager.cleanupFramebuffers(device);
        ssaoBlurPassManager.cleanupFramebuffers(device);
    }
}