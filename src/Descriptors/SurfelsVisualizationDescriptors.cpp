#include "SurfelsVisualizationDescriptors.h"

#include "Buffers/GeometryUniformBuffer.h"
#include "Buffers/SSAOBufferManager.h"
#include "Buffers/SurfelsBufferManager.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>

void SurfelsVisualizationDescriptors::createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, VkBuffer surfelBuffer, VkBuffer surfelGridBuffer, VkBuffer surfelCellBuffer,
                                                        VkBuffer cameraUniformBuffer, VkImageView positionImageView)
{
    // Descriptor pool
    std::vector<VkDescriptorPoolSize> poolSize = {
        {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1}};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    // Descriptor set layout
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(1);

    setLayoutBindings[0].binding = 0;
    setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[0].descriptorCount = 1;
    setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    layoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    // Descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = layouts.data();
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites(1);

        // Binding 0 -> Buffer para leer los datos de la cámara
        VkDescriptorBufferInfo cameraBufferInfo{};
        cameraBufferInfo.buffer = cameraUniformBuffer;
        cameraBufferInfo.offset = 0;
        cameraBufferInfo.range = sizeof(CameraUniformBuffer);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

        // Actualización del descriptor
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void SurfelsVisualizationDescriptors::cleanupDescriptors(VkDevice device)
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

VkDescriptorSetLayout SurfelsVisualizationDescriptors::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

VkDescriptorSet SurfelsVisualizationDescriptors::getDescriptorSet(int index)
{
    return descriptorSets[index];
}