#pragma once

#include "Scene/Models/MeshContainer.h"
#include "Camera/Camera.h"
#include "Buffers/SurfelsBufferManager.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

class CommandManager
{
private:
	int indirectDiffuseFrameCount = 0;

	VkCommandPool commandPool; // Maneja la memoria utilizada para guardar los buffers
	std::vector<VkCommandBuffer> commandBuffers;

	VkBuffer surfelsVBO = VK_NULL_HANDLE;
    VmaAllocation surfelsVBOAlloc;

	// Parámetros para Depth Bias
	float depthBiasConstant;
	float depthBiasSlope;

	void createCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void createCommandBuffers(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT);

public:
	CommandManager();

	void createCommandResources(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t MAX_FRAMES_IN_FLIGHT);
	void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex,
							 VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline,
							 VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet *shadowMappingDescriptorSet,
							 VkRenderPass geometryRenderPass, VkFramebuffer geometryFramebuffer, VkPipeline geometryPipeline,
							 VkPipelineLayout geometryPipelineLayout, VkDescriptorSet *geometryDescriptorSet,
							 std::vector<MeshContainer> sceneMeshes, Camera camera);
	void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
							 VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet *gBufferDescriptorSet,
							 VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet *ssaoDescriptorSet,
							 VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet *ssoBlurDescriptorSet,
							 VkRenderPass ssaoCompositionRenderPass, VkFramebuffer ssaoCompositionFramebuffer, VkPipeline ssaoCompositionPipeline, VkPipelineLayout ssaoCompositionPipelineLayout, VkDescriptorSet *ssaoCompositionDescriptorSet);
	void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
							 VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet *gBufferDescriptorSet,
							 VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet *ssaoDescriptorSet,
							 VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet *ssaoBlurDescriptorSet,
							 VkRenderPass ssaoCompositionRenderPass, VkFramebuffer ssaoCompositionFramebuffer, VkPipeline ssaoCompositionPipeline, VkPipelineLayout ssaoCompositionPipelineLayout, VkDescriptorSet *ssaoCompositionDescriptorSet,
							 VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline, VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet *shadowMappingDescriptorSet);
	void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes, VkRenderPass raytracingRenderPass,
							 VkFramebuffer raytracingFramebuffer, VkPipeline raytracingPipeline, VkPipelineLayout raytracingPipelineLayout, VkDescriptorSet *raytracingDescriptorSet);
	void recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
							 VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet *gBufferDescriptorSet,
							 VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet *ssaoDescriptorSet,
							 VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet *ssaoBlurDescriptorSet,
							 VkRenderPass surfelsCompositionRenderPass, VkFramebuffer surfelsCompositionFramebuffer, VkPipeline surfelsCompositionPipeline, VkPipelineLayout surfelsCompositionPipelineLayout, VkDescriptorSet *surfelsCompositionDescriptorSet,
							 VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline, VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet *shadowMappingDescriptorSet,
							 VkPipeline surfelsGenerationPipeline, VkPipelineLayout surfelsGenerationPipelineLayout, VkDescriptorSet *surfelsGenerationDescriptorSet, VkBuffer surfelStatsBuffer,
							 VkPipeline surfelsVisualizationPipeline, VkPipelineLayout surfelsVisualizationPipelineLayout, VkDescriptorSet *surfelsVisualizationDescriptorSet,
							 VkPipeline surfelsRadianceCalculationPipeline, VkPipelineLayout surfelsRadianceCalculationPipelineLayout, VkDescriptorSet *surfelsRadianceCalculationDescriptorSet, VkBuffer surfelBuffer,
							 VkPipeline surfelsIndirectLightingPipeline, VkPipelineLayout surfelsIndirectLightingPipelineLayout, VkDescriptorSet *surfelsIndirectLightingDescriptorSet,
							 VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkRenderPass surfelsVisualizationRenderPass, VkFramebuffer surfelsVisualizationFramebuffer,
							 VkRenderPass surfelsIndirectLightingRenderPass, VkFramebuffer surfelsIndirectLightingFramebuffer);
	void cleanup(VkDevice device);

	void cleanupAuxiliarBuffers(VkDevice device);

	VkCommandPool getCommandPool() const;
	VkCommandBuffer *getCommandBuffer(uint32_t id);
};