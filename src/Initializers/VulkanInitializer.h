#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "VulkanDeviceCreator.h"
#include "WindowManager.h"
#include "SwapChainManager.h"
#include "CommandManager.h"
#include "SynchronizationObjects.h"
#include "Render_Passes/Utils/DepthBuffer.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

class VulkanInitializer
{

private:
    uint32_t MAX_FRAMES_IN_FLIGHT;

    WindowManager windowManager;
    VulkanDeviceCreator vkDeviceCreator;
    SwapChainManager swapChainManager;
    CommandManager commandManager;
    SynchronizationObjects syncObjects;

public:
    VulkanInitializer();

    void prepareVulkan();
    void recreateSwapChain(std::vector<VkFramebuffer> swapChainFramebuffers, DepthBuffer depthBufferCreator);
    void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex,
                             VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline,
                             VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet shadowMappingDescriptorSet,
                             VkRenderPass geometryRenderPass, VkFramebuffer geometryFramebuffer, VkPipeline geometryPipeline,
                             VkPipelineLayout geometryPipelineLayout, VkDescriptorSet geometryDescriptorSet,
                             std::vector<MeshContainer> sceneMeshes, Camera camera);
    void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                             VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet gBufferDescriptorSet,
                             VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet ssaoDescriptorSet,
                             VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet ssoBlurDescriptorSet,
                             VkRenderPass ssaoCompositionRenderPass, VkFramebuffer ssaoCompositionFramebuffer, VkPipeline ssaoCompositionPipeline, VkPipelineLayout ssaoCompositionPipelineLayout, VkDescriptorSet ssaoCompositionDescriptorSet);
    void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                             VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet gBufferDescriptorSet,
                             VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet ssaoDescriptorSet,
                             VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet ssaoBlurDescriptorSet,
                             VkRenderPass ssaoCompositionRenderPass, VkFramebuffer ssaoCompositionFramebuffer, VkPipeline ssaoCompositionPipeline, VkPipelineLayout ssaoCompositionPipelineLayout, VkDescriptorSet ssaoCompositionDescriptorSet,
                             VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline, VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet shadowMappingDescriptorSet);
    void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes, VkRenderPass raytracingRenderPass,
                             VkFramebuffer raytracingFramebuffer, VkPipeline raytracingPipeline, VkPipelineLayout raytracingPipelineLayout, VkDescriptorSet raytracingDescriptorSet);
    void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                             VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet gBufferDescriptorSet,
                             VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet ssaoDescriptorSet,
                             VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet ssaoBlurDescriptorSet,
                             VkRenderPass surfelsCompositionRenderPass, VkFramebuffer surfelsCompositionFramebuffer, VkPipeline surfelsCompositionPipeline, VkPipelineLayout surfelsCompositionPipelineLayout, VkDescriptorSet surfelsCompositionDescriptorSet,
                             VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline, VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet shadowMappingDescriptorSet,
                             VkPipeline surfelsGenerationPipeline, VkPipelineLayout surfelsGenerationPipelineLayout, VkDescriptorSet surfelsGenerationDescriptorSet, VkBuffer surfelStatsBuffer,
                             VkPipeline surfelsVisualizationPipeline, VkPipelineLayout surfelsVisualizationPipelineLayout, VkDescriptorSet surfelsVisualizationDescriptorSet,
                             VkPipeline surfelsRadianceCalculationPipeline, VkPipelineLayout surfelsRadianceCalculationPipelineLayout, VkDescriptorSet surfelsRadianceCalculationDescriptorSet, VkBuffer surfelBuffer,
                             VkPipeline surfelsIndirectLightingPipeline, VkPipelineLayout surfelsIndirectLightingPipelineLayout, VkDescriptorSet surfelsIndirectLightingDescriptorSet,
                             VkRenderPass surfelsVisualizationRenderPass, VkFramebuffer surfelsVisualizationFramebuffer, VkRenderPass surfelsIndirectLightingRenderPass, VkFramebuffer surfelsIndirectLightingFramebuffer);

    void resetFramebufferResized();

    void cleanSurfelsAuxiliarBuffer();

    VkDevice getVkDevice();
    VkPhysicalDevice getVkPhysicalDevice();
    VkSurfaceKHR getVkSurface();
    VkQueue getVkGraphicsQueue();
    VkQueue getVkPresentQueue();

    VkSwapchainKHR getSwapChain();
    VkExtent2D getSwapChainExtent();
    VkFormat getSCImageFormat();
    size_t getNumImageViews();

    VkCommandBuffer *getCommandBuffer(uint32_t id);
    VkCommandPool getCommandPool();
    uint32_t getFramesInFlight();

    GLFWwindow *getWindow();
    bool isFramebufferResized();
    Camera *getCamera();

    const VkFence *getFence(uint32_t currentFrame);
    const VkSemaphore getImageSemaphore(uint32_t currentFrame);
    const VkSemaphore getRenderSemaphore(uint32_t currentFrame);

    SwapChainManager getSwapChainManager();

    void cleanup();
    void cleanupSwapChain(DepthBuffer depthBufferCreator, std::vector<VkFramebuffer> swapChainFramebuffers);
    void cleanupSwapChain();
    void cleanupSynchronizacionCommandObjects();
    void cleanupAuxiliarBuffers();
};