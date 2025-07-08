#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "ShadowMappingUniformBuffer.h"
#include "GeometryUniformBuffer.h"
#include "GUniformBufferManager.h"
#include "SSAOBufferManager.h"
#include "ShadowSSAOCompositionUniformBuffer.h"
#include "RaytracingUniformBuffer.h"
#include "SurfelsBufferManager.h"
#include "Raytracing/RaytracingManager.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <chrono>
#include <vector>
#include <array>

class UniformBuffersManager
{
private:
    ShadowMappingUniformBuffer shadowMappingUniformBuffer;
    GeometryUniformBuffer geometryUniformBuffer;

    GUniformBuffer gUniformBuffer;
    SSAOUniformBuffer ssaoUniformBuffer;

    ShadowSSAOCompositionUniformBuffer shadowSSAOUniformBuffer;

    RaytracingUniformBuffer raytracingUniformBuffer;

    SurfelsBufferManager surfelsResourcesManager;

public:
    void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, MainDirectionalLight light,
                              uint32_t width, uint32_t height, Camera* camera, LightsData sceneLights, VkCommandPool commandPool, VkQueue graphicsQueue,
                              std::vector<MeshContainer> sceneMeshes);
    void updateUniformBuffers(uint32_t currentImage, MainDirectionalLight light, uint32_t width, uint32_t height, Camera* camera, LightsData sceneLights);
    void cleanupUniformBuffers(VkDevice device);

    std::vector<VkBuffer> getGeometryMVPBuffers();
    std::vector<VkBuffer> getGeometryLightsBuffers();
    std::vector<VkBuffer> getGeometryMainLightBuffers();
    std::vector<VkBuffer> getShadowMappingBuffers();
    std::vector<VkBuffer> getGBuffers();
    std::vector<VkBuffer> getSSAOProjectionBuffers();
    std::vector<VkBuffer> getSSAOParamsBuffers();
    std::vector<VkBuffer> getShadowCompositionLightsBuffers();
    std::vector<VkBuffer> getShadowCompositionMainLightBuffers();
    std::vector<VkBuffer> getShadowCompositionMVPBuffers();
    std::vector<VkBuffer> getRaytracingMVPBuffers();
    std::vector<VkBuffer> getRaytracingLightsBuffers();

    VkBuffer getSurfelPositionBuffer();
    VkBuffer getSurfelBuffer();
    VkBuffer getSurfelStatsBuffer();
    VkBuffer getSurfelGridBuffer();
    VkBuffer getSurfelCellBuffer();
    VkBuffer getCameraSurfelBuffer();
    VkBuffer getTranslucentMaterialsBuffer();

    std::vector<VkBuffer> getIndexBufferList();
    std::vector<VkBuffer> getVertexBufferList();
    std::vector<size_t> getIndexBufferSizeList();
    std::vector<size_t> getVertexBufferSizeList();

    ImageCreator getRaysNoiseImage();
    ImageCreator getBlueNoiseImage();
};