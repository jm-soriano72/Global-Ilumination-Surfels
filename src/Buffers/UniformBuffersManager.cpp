#include "UniformBuffersManager.h"

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/SceneManager.h"
#include "Scene/Illumination/LightsData.h"
#include "Scene/Illumination/MainDirectionalLight.h"
#include "ShadowMappingUniformBuffer.h"
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

#include <chrono>
#include <vector>
#include <array>

void UniformBuffersManager::createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, MainDirectionalLight light,
                                                 uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights, VkCommandPool commandPool, VkQueue graphicsQueue,
                                                 std::vector<MeshContainer> sceneMeshes)
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        shadowMappingUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, light, width, height);
        geometryUniformBuffer.createGeometryUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera, sceneLights);
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        gUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera);
        ssaoUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        shadowMappingUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, light, width, height);
        gUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera);
        ssaoUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height);
        shadowSSAOUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera, sceneLights);
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        raytracingUniformBuffer.createRaytracingUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera, sceneLights);
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        shadowMappingUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, light, width, height);
        gUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera);
        ssaoUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height);
        shadowSSAOUniformBuffer.createUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera, sceneLights);
        surfelsResourcesManager.createSurfelsResources(device, physicalDevice, width, height, camera, commandPool, graphicsQueue, sceneMeshes);
    }
}

void UniformBuffersManager::updateUniformBuffers(uint32_t currentImage, MainDirectionalLight light, uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights)
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        shadowMappingUniformBuffer.updateUniformBuffersOffscreen(currentImage, light, width, height);
        geometryUniformBuffer.updateGeometryUniformBuffers(currentImage, width, height, camera, sceneLights);
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        gUniformBuffer.updateUniformBuffers(currentImage, width, height, camera);
        ssaoUniformBuffer.updateUniformBuffers(currentImage, width, height);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        shadowMappingUniformBuffer.updateUniformBuffersOffscreen(currentImage, light, width, height);
        gUniformBuffer.updateUniformBuffers(currentImage, width, height, camera);
        ssaoUniformBuffer.updateUniformBuffers(currentImage, width, height);
        shadowSSAOUniformBuffer.updateUniformBuffers(currentImage, width, height, camera, sceneLights);
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        raytracingUniformBuffer.updateRaytracingUniformBuffers(currentImage, width, height, camera, sceneLights);
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        shadowMappingUniformBuffer.updateUniformBuffersOffscreen(currentImage, light, width, height);
        gUniformBuffer.updateUniformBuffers(currentImage, width, height, camera);
        ssaoUniformBuffer.updateUniformBuffers(currentImage, width, height);
        shadowSSAOUniformBuffer.updateUniformBuffers(currentImage, width, height, camera, sceneLights);
        surfelsResourcesManager.updateUniformBuffers(width, height, camera);
    }
}

void UniformBuffersManager::cleanupUniformBuffers(VkDevice device)
{
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        shadowMappingUniformBuffer.cleanup(device);
        geometryUniformBuffer.cleanup(device);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        shadowMappingUniformBuffer.cleanup(device);
        gUniformBuffer.cleanup(device);
        ssaoUniformBuffer.cleanup(device);
        shadowSSAOUniformBuffer.cleanup(device);
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        gUniformBuffer.cleanup(device);
        ssaoUniformBuffer.cleanup(device);
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        raytracingUniformBuffer.cleanup(device);
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        shadowMappingUniformBuffer.cleanup(device);
        gUniformBuffer.cleanup(device);
        ssaoUniformBuffer.cleanup(device);
        shadowSSAOUniformBuffer.cleanup(device);
        surfelsResourcesManager.cleanup(device);
    }
}

std::vector<VkBuffer> UniformBuffersManager::getGeometryMVPBuffers()
{
    return geometryUniformBuffer.getMVPBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getGeometryLightsBuffers()
{
    return geometryUniformBuffer.getLightsBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getGeometryMainLightBuffers()
{
    return geometryUniformBuffer.getMainLightBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getShadowMappingBuffers()
{
    return shadowMappingUniformBuffer.getUniformBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getGBuffers()
{
    return gUniformBuffer.getUniformBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getSSAOProjectionBuffers()
{
    return ssaoUniformBuffer.getProjectionUniformBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getSSAOParamsBuffers()
{
    return ssaoUniformBuffer.getSSAOUniformBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getShadowCompositionLightsBuffers()
{
    return shadowSSAOUniformBuffer.getLightsBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getShadowCompositionMainLightBuffers()
{
    return shadowSSAOUniformBuffer.getMainLightBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getShadowCompositionMVPBuffers()
{
    return shadowSSAOUniformBuffer.getMVPBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getRaytracingMVPBuffers()
{
    return raytracingUniformBuffer.getMVPBuffers();
}

std::vector<VkBuffer> UniformBuffersManager::getRaytracingLightsBuffers()
{
    return raytracingUniformBuffer.getLightsBuffers();
}

VkBuffer UniformBuffersManager::getSurfelPositionBuffer()
{
    return surfelsResourcesManager.getSurfelPositionBuffer();
}

VkBuffer UniformBuffersManager::getSurfelBuffer()
{
    return surfelsResourcesManager.getSurfelBuffer();
}

VkBuffer UniformBuffersManager::getSurfelStatsBuffer()
{
    return surfelsResourcesManager.getSurfelStatsBuffer();
}

VkBuffer UniformBuffersManager::getSurfelGridBuffer()
{
    return surfelsResourcesManager.getSurfelGridBuffer();
}

VkBuffer UniformBuffersManager::getSurfelCellBuffer()
{
    return surfelsResourcesManager.getSurfelCellBuffer();
}

VkBuffer UniformBuffersManager::getCameraSurfelBuffer()
{
    return surfelsResourcesManager.getCameraSurfelBuffer();
}

VkBuffer UniformBuffersManager::getTranslucentMaterialsBuffer()
{
    return surfelsResourcesManager.getTranslucentMaterialsBuffer();
}

std::vector<VkBuffer> UniformBuffersManager::getIndexBufferList()
{
    return surfelsResourcesManager.getIndexBufferList();
}

std::vector<VkBuffer> UniformBuffersManager::getVertexBufferList()
{
    return surfelsResourcesManager.getVertexBufferList();
}

std::vector<size_t> UniformBuffersManager::getIndexBufferSizeList()
{
    return surfelsResourcesManager.getIndexBufferSizeList();
}

std::vector<size_t> UniformBuffersManager::getVertexBufferSizeList()
{
    return surfelsResourcesManager.getVertexBufferSizeList();
}

ImageCreator UniformBuffersManager::getRaysNoiseImage()
{
    return surfelsResourcesManager.getRaysNoiseImage();
}

ImageCreator UniformBuffersManager::getBlueNoiseImage()
{
    return surfelsResourcesManager.getBlueNoiseImage();
}