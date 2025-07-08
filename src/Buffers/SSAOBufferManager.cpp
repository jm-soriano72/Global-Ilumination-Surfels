#include "SSAOBufferManager.h"

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
#include <random>
#include <ctime>

void SSAOUniformBuffer::createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height)
{
    createUniformProjBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height);
    createUniformSSAOBuffers(device, physicalDevice, MAX_FRAMES_IN_FLIGHT, width, height);
}

void SSAOUniformBuffer::createUniformProjBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height)
{
    VkDeviceSize bufferSize = sizeof(UniformSSAOBufferObject);

    uniformProjectionBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformProjectionBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformProjectionBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    uniformProjectionBuffers[i], uniformProjectionBuffersMemory[i]);

        vkMapMemory(device, uniformProjectionBuffersMemory[i], 0, bufferSize, 0, &uniformProjectionBuffersMapped[i]);

        updateUniformBuffers(i, width, height);
    }
}

void SSAOUniformBuffer::createUniformSSAOBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height)
{
    VkDeviceSize bufferSize = sizeof(UniformSSAOBufferKernel);

    // Se genera un kernel de muestras aleatorias para cada fragmento. Se emplea para evaluar la cercanía a otros elementos y poder calcular la oclusión ambiental
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

    UniformSSAOBufferKernel ssaoKernelUBO;
    for (uint32_t i = 0; i < SSAO_KERNEL_SIZE; ++i)
    {
        glm::vec3 sample(rndDist(rndEngine) * 2.0 - 1.0, rndDist(rndEngine) * 2.0 - 1.0, rndDist(rndEngine));
        sample = glm::normalize(sample);
        sample *= rndDist(rndEngine);
        float scale = float(i) / float(SSAO_KERNEL_SIZE);
        scale = 0.1f + (1.0f - 0.1f) * (scale * scale);
        ssaoKernelUBO.kernels[i] = glm::vec4(sample * scale, 0.0f);
    }

    uniformSSAOBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformSSAOBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformSSAOBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    uniformSSAOBuffers[i], uniformSSAOBuffersMemory[i]);

        vkMapMemory(device, uniformSSAOBuffersMemory[i], 0, bufferSize, 0, &uniformSSAOBuffersMapped[i]);

        memcpy(uniformSSAOBuffersMapped[i], &ssaoKernelUBO, sizeof(ssaoKernelUBO));
    }
}

void SSAOUniformBuffer::updateUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height)
{
    UniformSSAOBufferObject ubo{};

    // Ajustar la proyección
    float zNear = 0.1f;
    float zFar = 3000.0f;

    ubo.proj = glm::perspective(glm::radians(60.0f), (float)width / (float)height, zNear, zFar);
    ubo.proj[1][1] *= -1;

    memcpy(uniformProjectionBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

std::vector<VkBuffer> SSAOUniformBuffer::getProjectionUniformBuffers() {
    return uniformProjectionBuffers;
}

std::vector<VkBuffer> SSAOUniformBuffer::getSSAOUniformBuffers() {
    return uniformSSAOBuffers;
}

void SSAOUniformBuffer::cleanup(VkDevice device)
{
    for (size_t i = 0; i < uniformProjectionBuffers.size(); i++)
    {
        vkDestroyBuffer(device, uniformProjectionBuffers[i], nullptr);
        vkFreeMemory(device, uniformProjectionBuffersMemory[i], nullptr);
        vkDestroyBuffer(device, uniformSSAOBuffers[i], nullptr);
        vkFreeMemory(device, uniformSSAOBuffersMemory[i], nullptr);
    }
}
