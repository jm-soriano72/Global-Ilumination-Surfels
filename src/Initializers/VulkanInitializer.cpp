#include "VulkanInitializer.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "VulkanDeviceCreator.h"
#include "WindowManager.h"
#include "SwapChainManager.h"
#include "CommandManager.h"
#include "SynchronizationObjects.h"
#include "RenderApplication.h"

VulkanInitializer::VulkanInitializer()
{
    MAX_FRAMES_IN_FLIGHT = 1;

    windowManager = WindowManager();
    swapChainManager = SwapChainManager();
    commandManager = CommandManager();
    syncObjects = SynchronizationObjects(MAX_FRAMES_IN_FLIGHT);
}

void VulkanInitializer::prepareVulkan()
{
    windowManager.initWindow(WIDTH, HEIGHT);
    vkDeviceCreator.createDevices(enableValidationLayers, validationLayers, windowManager.getWindow());
    swapChainManager.createSwapChain(vkDeviceCreator.getVkDevice(), vkDeviceCreator.getVkPhysicalDevice(), vkDeviceCreator.getVkSurface(), windowManager.getWindow());
    swapChainManager.createImageViews(vkDeviceCreator.getVkDevice());
    commandManager.createCommandResources(vkDeviceCreator.getVkDevice(), vkDeviceCreator.getVkPhysicalDevice(), vkDeviceCreator.getVkSurface(), MAX_FRAMES_IN_FLIGHT);
    syncObjects.createSyncObjects(vkDeviceCreator.getVkDevice());
}

void VulkanInitializer::recreateSwapChain(std::vector<VkFramebuffer> swapChainFramebuffers, DepthBuffer depthBufferCreator)
{
    swapChainManager.recreateSwapChain(vkDeviceCreator.getVkDevice(), vkDeviceCreator.getVkPhysicalDevice(), vkDeviceCreator.getVkSurface(), windowManager.getWindow(),
                                       swapChainFramebuffers, depthBufferCreator);
}

void VulkanInitializer::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex,
                                            VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline,
                                            VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet shadowMappingDescriptorSet,
                                            VkRenderPass geometryRenderPass, VkFramebuffer geometryFramebuffer, VkPipeline geometryPipeline,
                                            VkPipelineLayout geometryPipelineLayout, VkDescriptorSet geometryDescriptorSet,
                                            std::vector<MeshContainer> sceneMeshes, Camera camera)
{
    commandManager.recordCommandBuffer(extent, currentFrame, imageIndex, shadowMappingRenderPass, shadowMappingFramebuffer, shadowMappingPipeline,
                                       shadowMappingPipelineLayout, &shadowMappingDescriptorSet, geometryRenderPass, geometryFramebuffer, geometryPipeline, geometryPipelineLayout,
                                       &geometryDescriptorSet, sceneMeshes, camera);
}

void VulkanInitializer::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                                            VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet gBufferDescriptorSet,
                                            VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet ssaoDescriptorSet,
                                            VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet ssaoBlurDescriptorSet,
                                            VkRenderPass ssaoCompositionRenderPass, VkFramebuffer ssaoCompositionFramebuffer, VkPipeline ssaoCompositionPipeline, VkPipelineLayout ssaoCompositionPipelineLayout, VkDescriptorSet ssaoCompositionDescriptorSet)
{
    commandManager.recordCommandBuffer(extent, currentFrame, imageIndex, sceneMeshes,
                                       gBufferRenderPass, gBufferFramebuffer, gBufferPipeline, gBufferPipelineLayout, &gBufferDescriptorSet,
                                       ssaoRenderPass, ssaoFramebuffer, ssaoPipeline, ssaoPipelineLayout, &ssaoDescriptorSet,
                                       ssaoBlurRenderPass, ssaoBlurFramebuffer, ssaoBlurPipeline, ssaoBlurPipelineLayout, &ssaoBlurDescriptorSet,
                                       ssaoCompositionRenderPass, ssaoCompositionFramebuffer, ssaoCompositionPipeline, ssaoCompositionPipelineLayout, &ssaoCompositionDescriptorSet);
}

void VulkanInitializer::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                                            VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet gBufferDescriptorSet,
                                            VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet ssaoDescriptorSet,
                                            VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet ssaoBlurDescriptorSet,
                                            VkRenderPass ssaoCompositionRenderPass, VkFramebuffer ssaoCompositionFramebuffer, VkPipeline ssaoCompositionPipeline, VkPipelineLayout ssaoCompositionPipelineLayout, VkDescriptorSet ssaoCompositionDescriptorSet,
                                            VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline, VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet shadowMappingDescriptorSet)
{
    commandManager.recordCommandBuffer(extent, currentFrame, imageIndex, sceneMeshes,
                                       gBufferRenderPass, gBufferFramebuffer, gBufferPipeline, gBufferPipelineLayout, &gBufferDescriptorSet,
                                       ssaoRenderPass, ssaoFramebuffer, ssaoPipeline, ssaoPipelineLayout, &ssaoDescriptorSet,
                                       ssaoBlurRenderPass, ssaoBlurFramebuffer, ssaoBlurPipeline, ssaoBlurPipelineLayout, &ssaoBlurDescriptorSet,
                                       ssaoCompositionRenderPass, ssaoCompositionFramebuffer, ssaoCompositionPipeline, ssaoCompositionPipelineLayout, &ssaoCompositionDescriptorSet,
                                       shadowMappingRenderPass, shadowMappingFramebuffer, shadowMappingPipeline, shadowMappingPipelineLayout, &shadowMappingDescriptorSet);
}

void VulkanInitializer::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes, VkRenderPass raytracingRenderPass,
                                            VkFramebuffer raytracingFramebuffer, VkPipeline raytracingPipeline, VkPipelineLayout raytracingPipelineLayout, VkDescriptorSet raytracingDescriptorSet)
{
    commandManager.recordCommandBuffer(extent, currentFrame, imageIndex, sceneMeshes, raytracingRenderPass,
                                       raytracingFramebuffer, raytracingPipeline, raytracingPipelineLayout, &raytracingDescriptorSet);
}

void VulkanInitializer::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                                            VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet gBufferDescriptorSet,
                                            VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet ssaoDescriptorSet,
                                            VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet ssaoBlurDescriptorSet,
                                            VkRenderPass surfelsCompositionRenderPass, VkFramebuffer surfelsCompositionFramebuffer, VkPipeline surfelsCompositionPipeline, VkPipelineLayout surfelsCompositionPipelineLayout, VkDescriptorSet surfelsCompositionDescriptorSet,
                                            VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline, VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet shadowMappingDescriptorSet,
                                            VkPipeline surfelsGenerationPipeline, VkPipelineLayout surfelsGenerationPipelineLayout, VkDescriptorSet surfelsGenerationDescriptorSet, VkBuffer surfelStatsBuffer,
                                            VkPipeline surfelsVisualizationPipeline, VkPipelineLayout surfelsVisualizationPipelineLayout, VkDescriptorSet surfelsVisualizationDescriptorSet,
                                            VkPipeline surfelsRadianceCalculationPipeline, VkPipelineLayout surfelsRadianceCalculationPipelineLayout, VkDescriptorSet surfelsRadianceCalculationDescriptorSet, VkBuffer surfelBuffer,
                                            VkPipeline surfelsIndirectLightingPipeline, VkPipelineLayout surfelsIndirectLightingPipelineLayout, VkDescriptorSet surfelsIndirectLightingDescriptorSet,
                                            VkRenderPass surfelsVisualizationRenderPass, VkFramebuffer surfelsVisualizationFramebuffer, VkRenderPass surfelsIndirectLightingRenderPass, VkFramebuffer surfelsIndirectLightingFramebuffer)
{
    commandManager.recordCommandBuffer(extent, currentFrame, imageIndex, sceneMeshes,
                                       gBufferRenderPass, gBufferFramebuffer, gBufferPipeline, gBufferPipelineLayout, &gBufferDescriptorSet,
                                       ssaoRenderPass, ssaoFramebuffer, ssaoPipeline, ssaoPipelineLayout, &ssaoDescriptorSet,
                                       ssaoBlurRenderPass, ssaoBlurFramebuffer, ssaoBlurPipeline, ssaoBlurPipelineLayout, &ssaoBlurDescriptorSet,
                                       surfelsCompositionRenderPass, surfelsCompositionFramebuffer, surfelsCompositionPipeline, surfelsCompositionPipelineLayout, &surfelsCompositionDescriptorSet,
                                       shadowMappingRenderPass, shadowMappingFramebuffer, shadowMappingPipeline, shadowMappingPipelineLayout, &shadowMappingDescriptorSet,
                                       surfelsGenerationPipeline, surfelsGenerationPipelineLayout, &surfelsGenerationDescriptorSet, surfelStatsBuffer,
                                       surfelsVisualizationPipeline, surfelsVisualizationPipelineLayout, &surfelsVisualizationDescriptorSet,
                                       surfelsRadianceCalculationPipeline, surfelsRadianceCalculationPipelineLayout, &surfelsRadianceCalculationDescriptorSet, surfelBuffer,
                                       surfelsIndirectLightingPipeline, surfelsIndirectLightingPipelineLayout, &surfelsIndirectLightingDescriptorSet,
                                       vkDeviceCreator.getVkDevice(), vkDeviceCreator.getVkPhysicalDevice(), commandManager.getCommandPool(), vkDeviceCreator.getVkGraphicsQueue(),
                                       surfelsVisualizationRenderPass, surfelsVisualizationFramebuffer, surfelsIndirectLightingRenderPass, surfelsIndirectLightingFramebuffer);
}

void VulkanInitializer::resetFramebufferResized()
{
    windowManager.resetFramebufferResized();
}

void VulkanInitializer::cleanSurfelsAuxiliarBuffer()
{
    commandManager.cleanupAuxiliarBuffers(vkDeviceCreator.getVkDevice());
}

VkDevice VulkanInitializer::getVkDevice()
{
    return vkDeviceCreator.getVkDevice();
}

VkPhysicalDevice VulkanInitializer::getVkPhysicalDevice()
{
    return vkDeviceCreator.getVkPhysicalDevice();
}

VkSurfaceKHR VulkanInitializer::getVkSurface()
{
    return vkDeviceCreator.getVkSurface();
}

VkQueue VulkanInitializer::getVkGraphicsQueue()
{
    return vkDeviceCreator.getVkGraphicsQueue();
}

VkQueue VulkanInitializer::getVkPresentQueue()
{
    return vkDeviceCreator.getVkPresentQueue();
}

VkSwapchainKHR VulkanInitializer::getSwapChain()
{
    return swapChainManager.getSwapChain();
}

VkExtent2D VulkanInitializer::getSwapChainExtent()
{
    return swapChainManager.getSwapChainExtent();
}

VkFormat VulkanInitializer::getSCImageFormat()
{
    return swapChainManager.getSCImageFormat();
}

size_t VulkanInitializer::getNumImageViews()
{
    return swapChainManager.getNumImageViews();
}

VkCommandBuffer *VulkanInitializer::getCommandBuffer(uint32_t id)
{
    return commandManager.getCommandBuffer(id);
}

VkCommandPool VulkanInitializer::getCommandPool()
{
    return commandManager.getCommandPool();
}

GLFWwindow *VulkanInitializer::getWindow()
{
    return windowManager.getWindow();
}

bool VulkanInitializer::isFramebufferResized()
{
    return windowManager.isFramebufferResized();
}

Camera *VulkanInitializer::getCamera()
{
    return windowManager.getCamera();
}

uint32_t VulkanInitializer::getFramesInFlight()
{
    return MAX_FRAMES_IN_FLIGHT;
}

const VkFence *VulkanInitializer::getFence(uint32_t currentFrame)
{
    return syncObjects.getFence(currentFrame);
}

const VkSemaphore VulkanInitializer::getImageSemaphore(uint32_t currentFrame)
{
    return syncObjects.getImageSemaphore(currentFrame);
}

const VkSemaphore VulkanInitializer::getRenderSemaphore(uint32_t currentFrame)
{
    return syncObjects.getRenderSemaphore(currentFrame);
}

SwapChainManager VulkanInitializer::getSwapChainManager()
{
    return this->swapChainManager;
}

void VulkanInitializer::cleanup()
{
    vkDeviceCreator.cleanupDevices(enableValidationLayers);
    windowManager.cleanupWindow();
}

void VulkanInitializer::cleanupSwapChain(DepthBuffer depthBufferCreator, std::vector<VkFramebuffer> swapChainFramebuffers)
{
    swapChainManager.cleanupSwapChain(vkDeviceCreator.getVkDevice(), depthBufferCreator, swapChainFramebuffers);
}

void VulkanInitializer::cleanupSwapChain()
{
    swapChainManager.cleanupSwapChain(vkDeviceCreator.getVkDevice());
}

void VulkanInitializer::cleanupSynchronizacionCommandObjects()
{
    syncObjects.cleanup(vkDeviceCreator.getVkDevice());
    commandManager.cleanup(vkDeviceCreator.getVkDevice());
}

void VulkanInitializer::cleanupAuxiliarBuffers()
{
    commandManager.cleanupAuxiliarBuffers(vkDeviceCreator.getVkDevice());
}
