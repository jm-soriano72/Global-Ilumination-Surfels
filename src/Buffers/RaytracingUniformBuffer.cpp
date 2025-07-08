#include "RaytracingUniformBuffer.h"

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "ShadowMappingUniformBuffer.h"
#include "GeometryUniformBuffer.h"

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

void RaytracingUniformBuffer::createRaytracingUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT,
                                                         uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights)
{
    createMVPUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera);
    createLightsBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, sceneLights);
}

void RaytracingUniformBuffer::updateRaytracingUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights)
{
    updateMVPUniformBuffer(currentImage, width, height, camera);
    updateLightsBuffer(currentImage, sceneLights);
}

void RaytracingUniformBuffer::createMVPUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height, Camera *camera)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformMVPBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformMVPBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformMVPBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    uniformMVPBuffers[i], uniformMVPBuffersMemory[i]);

        vkMapMemory(device, uniformMVPBuffersMemory[i], 0, bufferSize, 0, &uniformMVPBuffersMapped[i]);

        updateMVPUniformBuffer(i, width, height, camera);
    }
}

void RaytracingUniformBuffer::createLightsBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, LightsData sceneLights)
{
    // Se indica el tamaño del buffer en base a la clase que almacena los datos de todas las luces
    VkDeviceSize bufferSize = sizeof(LightsData);

    lightBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    lightBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    lightBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    lightBuffers[i], lightBuffersMemory[i]);

        vkMapMemory(device, lightBuffersMemory[i], 0, bufferSize, 0, &lightBuffersMapped[i]);

        updateLightsBuffer(i, sceneLights);
    }
}

void RaytracingUniformBuffer::updateMVPUniformBuffer(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera)
{
    UniformBufferObject ubo{};

    ubo.model = glm::mat4(1.0f);

    // Se obtiene la matriz de vista a través de los datos de la cámara según los inputs del usuario
    ubo.view = camera->getViewMatrix();

    // Ajustar la proyección
    float zNear = 0.1f;
    float zFar = 3000.0f;

    ubo.proj = glm::perspective(glm::radians(45.0f), width / (float)height, zNear, zFar);
    ubo.proj[1][1] *= -1;

    memcpy(uniformMVPBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void RaytracingUniformBuffer::updateLightsBuffer(uint32_t currentImage, LightsData sceneLights)
{
    // Se pasa la información al buffer
    memcpy(lightBuffersMapped[currentImage], &sceneLights, sizeof(sceneLights));
}

void RaytracingUniformBuffer::cleanup(VkDevice device)
{
    for (size_t i = 0; i < uniformMVPBuffers.size(); i++)
    {
        vkDestroyBuffer(device, uniformMVPBuffers[i], nullptr);
        vkFreeMemory(device, uniformMVPBuffersMemory[i], nullptr);
        vkDestroyBuffer(device, lightBuffers[i], nullptr);
        vkFreeMemory(device, lightBuffersMemory[i], nullptr);
    }
}

std::vector<VkBuffer> RaytracingUniformBuffer::getMVPBuffers() {
    return uniformMVPBuffers;
}

std::vector<VkBuffer> RaytracingUniformBuffer::getLightsBuffers() {
    return lightBuffers;
}
