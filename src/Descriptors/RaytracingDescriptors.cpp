#include "RaytracingDescriptors.h"

#include "Buffers/GeometryUniformBuffer.h"
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

void RaytracingDescriptors::createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers,
                                              AccelerationStructure &topLevelAccelerationStructure, std::vector<ImageCreator> &diffuseImageCreators, std::vector<ImageCreator> &alphaImageCreators,
                                              std::vector<ImageCreator> &specularImageCreators)
{
    // Descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes(4);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    poolSizes[2].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * numTextures * numMaterials);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    // Descriptor set layout
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(4);

    setLayoutBindings[0].binding = 0;
    setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[0].descriptorCount = 1;
    setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    setLayoutBindings[0].pImmutableSamplers = nullptr;

    setLayoutBindings[1].binding = 1;
    setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[1].descriptorCount = 1;
    setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    setLayoutBindings[1].pImmutableSamplers = nullptr;

    setLayoutBindings[2].binding = 2;
    setLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    setLayoutBindings[2].descriptorCount = 1;
    setLayoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[2].pImmutableSamplers = nullptr;

    setLayoutBindings[3].binding = 3;
    setLayoutBindings[3].descriptorCount = numMaterials * numTextures; // Un array con el total de texturas a usar de la escena
    setLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[3].pImmutableSamplers = nullptr;
    setLayoutBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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
    // Se reserva memoria para los descriptores
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set!");
    }

    // Configuración de los descriptores, uno para cada frame
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        // Conjunto de datos que almacena el descriptor set
        std::vector<VkWriteDescriptorSet> descriptorWrites(4);

        // Binding 0 -> Asignación del UBO (Matrices MVP)
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformMVPBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        // Binding 1 -> Asignación del UBO de luces
        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = lightBuffers[i];
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(LightsData);

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightBufferInfo;

        // Binding 2 -> Estructura de aceleración
        VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
        descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
        descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAccelerationStructure.handle;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pNext = &descriptorAccelerationStructureInfo;

        // Binding 3 -> Texturas
        std::vector<VkDescriptorImageInfo> imageInfos(numTextures * numMaterials);
        uint32_t globalTextureId = 0;
        for (uint32_t j = 0; j < numMaterials; j++)
        {
            imageInfos[globalTextureId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[globalTextureId].imageView = diffuseImageCreators[j].textureImageView;
            imageInfos[globalTextureId].sampler = diffuseImageCreators[j].textureSampler;
            globalTextureId++;

            imageInfos[globalTextureId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[globalTextureId].imageView = alphaImageCreators[j].textureImageView;
            imageInfos[globalTextureId].sampler = alphaImageCreators[j].textureSampler;
            globalTextureId++;

            imageInfos[globalTextureId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[globalTextureId].imageView = specularImageCreators[j].textureImageView;
            imageInfos[globalTextureId].sampler = specularImageCreators[j].textureSampler;
            globalTextureId++;
        }

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = static_cast<uint32_t>(numTextures * numMaterials);
        descriptorWrites[3].pImageInfo = imageInfos.data();

        // Actualización del descriptor
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void RaytracingDescriptors::cleanupDescriptors(VkDevice device)
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

VkDescriptorSetLayout RaytracingDescriptors::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

VkDescriptorSet RaytracingDescriptors::getDescriptorSet(int index)
{
    return descriptorSets[index];
}