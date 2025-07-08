#include "GeometryUniformBuffer.h"

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "ShadowMappingUniformBuffer.h"

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

void GeometryUniformBuffer::createGeometryUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT,
                                                         uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights)
{
    createMVPUniformBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height, camera);
    createLightsBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, sceneLights);
    createMainLightDataBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT);
}

void GeometryUniformBuffer::updateGeometryUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights)
{
    updateMVPUniformBuffer(currentImage, width, height, camera);
    updateLightsBuffer(currentImage, sceneLights);
    updateMainLightDataBuffer(currentImage);
}

void GeometryUniformBuffer::createMVPUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height, Camera *camera)
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

void GeometryUniformBuffer::createLightsBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, LightsData sceneLights)
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

void GeometryUniformBuffer::createMainLightDataBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT)
{
    VkDeviceSize bufferSize = sizeof(UniformDataOffscreen);

    mainLightDataBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    mainLightDataBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    mainLightDataBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    mainLightDataBuffers[i], mainLightDataBuffersMemory[i]);

        vkMapMemory(device, mainLightDataBuffersMemory[i], 0, bufferSize, 0, &mainLightDataBuffersMapped[i]);

        updateMainLightDataBuffer(i);
    }
}

void GeometryUniformBuffer::updateMVPUniformBuffer(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera)
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

void GeometryUniformBuffer::updateLightsBuffer(uint32_t currentImage, LightsData sceneLights)
{
    // Se pasa la información al buffer
    memcpy(lightBuffersMapped[currentImage], &sceneLights, sizeof(sceneLights));
}

void GeometryUniformBuffer::updateMainLightDataBuffer(uint32_t currentImage)
{
    // Se pasa la información al buffer
    memcpy(mainLightDataBuffersMapped[currentImage], &uniformDataOffscreen, sizeof(uniformDataOffscreen));
}

void GeometryUniformBuffer::cleanup(VkDevice device)
{
    for (size_t i = 0; i < uniformMVPBuffers.size(); i++)
    {
        vkDestroyBuffer(device, uniformMVPBuffers[i], nullptr);
        vkFreeMemory(device, uniformMVPBuffersMemory[i], nullptr);
        vkDestroyBuffer(device, lightBuffers[i], nullptr);
        vkFreeMemory(device, lightBuffersMemory[i], nullptr);
        vkDestroyBuffer(device, mainLightDataBuffers[i], nullptr);
        vkFreeMemory(device, mainLightDataBuffersMemory[i], nullptr);
    }
}

std::vector<VkBuffer> GeometryUniformBuffer::getMVPBuffers() {
    return uniformMVPBuffers;
}

std::vector<VkBuffer> GeometryUniformBuffer::getLightsBuffers() {
    return lightBuffers;
}

std::vector<VkBuffer> GeometryUniformBuffer::getMainLightBuffers() {
    return mainLightDataBuffers;
}
