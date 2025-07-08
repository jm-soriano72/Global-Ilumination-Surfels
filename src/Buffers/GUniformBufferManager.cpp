#include "GUniformBufferManager.h"

#include "GeometryUniformBuffer.h"
#include "Camera/Camera.h"

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

void GUniformBuffer::createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height, Camera* camera)
{
    VkDeviceSize bufferSize = sizeof(UniformGBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);

        updateUniformBuffers(i, width, height, camera);
    }
}

void GUniformBuffer::updateUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height, Camera* camera)
{
    UniformGBufferObject ubo{};

    ubo.model = glm::mat4(1.0f);

    // Se obtiene la matriz de vista a través de los datos de la cámara según los inputs del usuario
    ubo.view = camera->getViewMatrix();

    // Ajustar la proyección
    float zNear = 0.1f;
    float zFar = 8000.0f;

    ubo.proj = glm::perspective(glm::radians(60.0f), (float)width / (float)height, zNear, zFar);
    ubo.proj[1][1] *= -1;

    ubo.nearFarPlanes = glm::vec2(zNear, zFar);

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void GUniformBuffer::cleanup(VkDevice device)
{
    for (size_t i = 0; i < uniformBuffers.size(); i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
}

std::vector<VkBuffer> GUniformBuffer::getUniformBuffers() {
    return uniformBuffers;
}