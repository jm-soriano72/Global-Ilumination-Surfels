#include "ShadowMappingUniformBuffer.h"

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "Scene/Illumination/MainDirectionalLight.h"
#include "Tools/BufferCreator.h"

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

UniformDataOffscreen uniformDataOffscreen;

void ShadowMappingUniformBuffer::createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, MainDirectionalLight light,
                                                      uint32_t width, uint32_t height)
{
    VkDeviceSize bufferSize = sizeof(UniformDataOffscreen);
    uniformOffscreenBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformOffscreenBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformOffscreenBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        // Se crea el buffer con el tamaño adecuado
        BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    uniformOffscreenBuffers[i], uniformOffscreenBuffersMemory[i]);
        // Se mapea la memoria
        vkMapMemory(device, uniformOffscreenBuffersMemory[i], 0, bufferSize, 0, &uniformOffscreenBuffersMapped[i]);
        // Se inicializan sus datos
        updateUniformBuffersOffscreen(i, light, width, height);
    }
}

void ShadowMappingUniformBuffer::updateUniformBuffersOffscreen(uint32_t currentImage, MainDirectionalLight light, uint32_t width, uint32_t height)
{
    // Se calculan las matrices desde el punto de vista de la luz

    glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(45.0f), width / (float)height, zNear, zFar);
    glm::mat4 depthViewMatrix = glm::lookAt(light.position, light.target, glm::vec3(0, 1, 0));
    glm::mat4 depthModelMatrix = glm::mat4(1.0f);

    // Se calcula la matriz resultante
    uniformDataOffscreen.depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    // Se copia la información
    memcpy(uniformOffscreenBuffersMapped[currentImage], &uniformDataOffscreen, sizeof(uniformDataOffscreen));
}

void ShadowMappingUniformBuffer::cleanup(VkDevice device)
{
    for (size_t i = 0; i < uniformOffscreenBuffers.size(); i++)
    {
        vkDestroyBuffer(device, uniformOffscreenBuffers[i], nullptr);
        vkFreeMemory(device, uniformOffscreenBuffersMemory[i], nullptr);
    }
}

std::vector<VkBuffer> ShadowMappingUniformBuffer::getUniformBuffers() {
    return uniformOffscreenBuffers;
}