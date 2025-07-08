#include "GBufferDescriptors.h"

#include "Buffers/GeometryUniformBuffer.h"
#include "Buffers/GUniformBufferManager.h"

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

void GBufferDescriptors::createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<VkBuffer> uniformBuffers,
                                           std::vector<ImageCreator> &diffuseImageCreators, std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators)
{
    // Descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes(2);
    // Pool para las matrices
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Variables uniformes
    poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    // Pool para las texturas
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // Textura
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * numTextures * numMaterials);

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
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(2);

    setLayoutBindings[0].binding = 0;
    setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[0].descriptorCount = 1;
    setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[0].pImmutableSamplers = nullptr;

    setLayoutBindings[1].binding = 1;
    setLayoutBindings[1].descriptorCount = numMaterials * numTextures; // Un array con el total de texturas a usar de la escena
    setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[1].pImmutableSamplers = nullptr;
    setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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
    // Se generan todos los descriptor sets
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        // Conjunto de datos que almacena el descriptor set
        std::vector<VkWriteDescriptorSet> descriptorWrites(2);

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformGBufferObject);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        // Asignación de las texturas
        std::vector<VkDescriptorImageInfo> imageInfos(numTextures * numMaterials);
        // Variable para ordenar las texturas correctamente en el array global
        uint32_t globalTextureId = 0;
        // Se recorre el conjunto de texturas
        for (uint32_t j = 0; j < numMaterials; j++)
        {
            // Información para la imagen
            // Se extraen los datos de la textura correspondiente y se almacenan en el vector
            // Primero se almacena el mapa difuso
            imageInfos[globalTextureId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[globalTextureId].imageView = diffuseImageCreators[j].textureImageView;
            imageInfos[globalTextureId].sampler = diffuseImageCreators[j].textureSampler;
            globalTextureId++;
            // Después, se almacena el mapa de transparencia
            imageInfos[globalTextureId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[globalTextureId].imageView = alphaImageCreators[j].textureImageView;
            imageInfos[globalTextureId].sampler = alphaImageCreators[j].textureSampler;
            globalTextureId++;
            // Mapa especular
            imageInfos[globalTextureId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[globalTextureId].imageView = specularImageCreators[j].textureImageView;
            imageInfos[globalTextureId].sampler = specularImageCreators[j].textureSampler;
            globalTextureId++;
        }

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = static_cast<uint32_t>(numTextures * numMaterials);
        descriptorWrites[1].pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void GBufferDescriptors::cleanupDescriptors(VkDevice device)
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

VkDescriptorSetLayout GBufferDescriptors::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

VkDescriptorSet GBufferDescriptors::getDescriptorSet(int index)
{
    return descriptorSets[index];
}