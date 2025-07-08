#include "SurfelsRadianceCalculationDescriptors.h"

#include "Buffers/GeometryUniformBuffer.h"
#include "Raytracing/RaytracingManager.h"
#include "Buffers/SurfelsBufferManager.h"
#include "Scene/SponzaResources.h"

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

void SurfelsRadianceCalculationDescriptors::createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, VkBuffer surfelBuffer, std::vector<VkBuffer> lightsDataBuffer,
                                                              AccelerationStructure &topLevelAccelerationStructure, std::vector<VkBuffer> indexBufferList, std::vector<VkBuffer> vertexBufferList,
                                                              std::vector<size_t> indexBufferSizeList, std::vector<size_t> vertexBufferSizeList, uint32_t numTextures, uint32_t numMaterials,
                                                              std::vector<ImageCreator> &diffuseImageCreators, std::vector<ImageCreator> &alphaImageCreators,
                                                              std::vector<ImageCreator> &specularImageCreators, ImageCreator raysNoiseImage)
{
    // Descriptor pool
    std::vector<VkDescriptorPoolSize> poolSize = {
        {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 200},
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
    setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    setLayoutBindings[1].binding = 1;
    setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[1].descriptorCount = 1;
    setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    setLayoutBindings[2].binding = 2;
    setLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    setLayoutBindings[2].descriptorCount = 1;
    setLayoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    setLayoutBindings[3].binding = 3;
    setLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    setLayoutBindings[3].descriptorCount = indexBufferList.size();
    setLayoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    setLayoutBindings[4].binding = 4;
    setLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    setLayoutBindings[4].descriptorCount = vertexBufferList.size();
    setLayoutBindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    setLayoutBindings[5].binding = 5;
    setLayoutBindings[5].descriptorCount = numMaterials * numTextures;
    setLayoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    setLayoutBindings[6].binding = 6;
    setLayoutBindings[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[6].descriptorCount = 1;
    setLayoutBindings[6].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

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

        // Binding 1 -> Información de la luz de la escena
        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = lightsDataBuffer[i];
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(LightsData);

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightBufferInfo;

        // Binding 2 -> Buffer para leer los surfels generados
        VkDescriptorBufferInfo surfelDescInfo{};
        surfelDescInfo.buffer = surfelBuffer;
        surfelDescInfo.offset = 0;
        surfelDescInfo.range = sizeof(Surfel) * SURFEL_CAPACITY;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &surfelDescInfo;

        // Binding 3 -> Lista de buffers de índices
        // Crear la lista con la información de cada buffer
        std::vector<VkDescriptorBufferInfo> indexBufferInfoList(indexBufferList.size());
        for(int i = 0; i < indexBufferList.size(); i++) {
            indexBufferInfoList[i].buffer = indexBufferList[i];
            indexBufferInfoList[i].offset = 0;
            indexBufferInfoList[i].range = indexBufferSizeList[i];
        }

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[3].descriptorCount = indexBufferInfoList.size();
        descriptorWrites[3].pBufferInfo = indexBufferInfoList.data();

        // Binding 4 -> Lista de buffers de vértices
        // Crear la lista con la información de cada buffer
        std::vector<VkDescriptorBufferInfo> vertexBufferInfoList(vertexBufferList.size());
        for(int i = 0; i < vertexBufferList.size(); i++) {
            vertexBufferInfoList[i].buffer = vertexBufferList[i];
            vertexBufferInfoList[i].offset = 0;
            vertexBufferInfoList[i].range = vertexBufferSizeList[i];
        }

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[4].descriptorCount = vertexBufferInfoList.size();
        descriptorWrites[4].pBufferInfo = vertexBufferInfoList.data();

        // Binding 5 -> Texturas de la geometría para obtener la información de color con el choque
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

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = descriptorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[5].descriptorCount = static_cast<uint32_t>(numTextures * numMaterials);
        descriptorWrites[5].pImageInfo = imageInfos.data();

        // Binding 6 -> Imagen con el ruido
        VkDescriptorImageInfo noiseImageDescriptor{};
        noiseImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        noiseImageDescriptor.imageView = raysNoiseImage.textureImageView;
        noiseImageDescriptor.sampler = raysNoiseImage.textureSampler;

        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[6].dstSet = descriptorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pImageInfo = &noiseImageDescriptor;

        // Actualización del descriptor
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void SurfelsRadianceCalculationDescriptors::cleanupDescriptors(VkDevice device)
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

VkDescriptorSetLayout SurfelsRadianceCalculationDescriptors::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

VkDescriptorSet SurfelsRadianceCalculationDescriptors::getDescriptorSet(int index)
{
    return descriptorSets[index];
}