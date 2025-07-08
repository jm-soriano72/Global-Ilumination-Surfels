#include "IndirectDiffuseShadingDescriptors.h"

#include "Buffers/GeometryUniformBuffer.h"
#include "Buffers/SSAOBufferManager.h"
#include "Buffers/SurfelsBufferManager.h"
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

#include <vector>

void IndirectDiffuseShadingDescriptors::createDescriptors(VkDevice device, AccelerationStructure &topLevelAccelerationStructure, uint32_t MAX_FRAMES_IN_FLIGHT,
                                                          VkBuffer surfelBuffer, VkBuffer surfelGridBuffer, VkBuffer surfelCellBuffer, VkBuffer cameraUniformBuffer,
                                                          VkImageView positionImageView, VkImageView normalImageView)
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
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(7);

    setLayoutBindings[0].binding = 0;
    setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    setLayoutBindings[0].descriptorCount = 1;
    setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    setLayoutBindings[1].binding = 1;
    setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    setLayoutBindings[1].descriptorCount = 1;
    setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    setLayoutBindings[2].binding = 2;
    setLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    setLayoutBindings[2].descriptorCount = 1;
    setLayoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    setLayoutBindings[3].binding = 3;
    setLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    setLayoutBindings[3].descriptorCount = 1;
    setLayoutBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    setLayoutBindings[4].binding = 4;
    setLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[4].descriptorCount = 1;
    setLayoutBindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    setLayoutBindings[5].binding = 5;
    setLayoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[5].descriptorCount = 1;
    setLayoutBindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    setLayoutBindings[6].binding = 6;
    setLayoutBindings[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[6].descriptorCount = 1;
    setLayoutBindings[6].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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

    // Creación del sampler
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 1.0f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vkCreateSampler(device, &samplerCreateInfo, nullptr, &imageDescriptorSampler);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites(7);

        // Binding 0 -> Estructura de aceleración con la geometría de la escena
        VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
        descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
        descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAccelerationStructure.handle;

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pNext = &descriptorAccelerationStructureInfo;

        // Binding 1 -> Buffer con la lista de surfels
        VkDescriptorBufferInfo surfelDescInfo{};
        surfelDescInfo.buffer = surfelBuffer;
        surfelDescInfo.offset = 0;
        surfelDescInfo.range = sizeof(Surfel) * SURFEL_CAPACITY;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &surfelDescInfo;

        // Binding 2 -> Buffer con la el número de surfels por celda
        VkDescriptorBufferInfo surfelGridDescInfo{};
        surfelGridDescInfo.buffer = surfelGridBuffer;
        surfelGridDescInfo.offset = 0;
        surfelGridDescInfo.range = sizeof(unsigned int) * SURFEL_TABLE_SIZE;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &surfelGridDescInfo;

        // Binding 3 -> Buffer con los identificadores globales de los surfels de cada celda
        VkDescriptorBufferInfo surfelCellDescInfo{};
        surfelCellDescInfo.buffer = surfelCellBuffer;
        surfelCellDescInfo.offset = 0;
        surfelCellDescInfo.range = sizeof(unsigned int) * SURFEL_TABLE_SIZE * SURFEL_CELL_LIMIT;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &surfelCellDescInfo;

        // Binding 4 -> Imagen con la posición
        VkDescriptorImageInfo positionImageDescriptor{};
        positionImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        positionImageDescriptor.imageView = positionImageView;
        positionImageDescriptor.sampler = imageDescriptorSampler;

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &positionImageDescriptor;

        // Binding 5 -> Imagen con las normales
        VkDescriptorImageInfo normalImageDescriptor{};
        normalImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalImageDescriptor.imageView = normalImageView;
        normalImageDescriptor.sampler = imageDescriptorSampler;

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = descriptorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pImageInfo = &normalImageDescriptor;

        // Binding 6 -> Buffer para leer los datos de la cámara
        VkDescriptorBufferInfo cameraBufferInfo{};
        cameraBufferInfo.buffer = cameraUniformBuffer;
        cameraBufferInfo.offset = 0;
        cameraBufferInfo.range = sizeof(CameraUniformBuffer);

        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[6].dstSet = descriptorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pBufferInfo = &cameraBufferInfo;

        // Actualización del descriptor
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void IndirectDiffuseShadingDescriptors::cleanupDescriptors(VkDevice device)
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroySampler(device, imageDescriptorSampler, nullptr);
}

VkDescriptorSetLayout IndirectDiffuseShadingDescriptors::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

VkDescriptorSet IndirectDiffuseShadingDescriptors::getDescriptorSet(int index)
{
    return descriptorSets[index];
}