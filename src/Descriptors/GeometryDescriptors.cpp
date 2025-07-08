#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "GeometryDescriptors.h"
#include "ShadowMappingDescriptors.h"
#include "Buffers/GeometryUniformBuffer.h"

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

void GeometryDescriptors::createDescriptors(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<ImageCreator> &diffuseImageCreators,
                                            std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, VkImageView depthImageView, VkSampler depthSampler,
                                            std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffers)
{
    createDescriptorSetLayout(device, numMaterials, numTextures);
    createDescriptorPool(device, numTextures, numMaterials, MAX_FRAMES_IN_FLIGHT);
    createDescriptorSets(device, numTextures, numMaterials, MAX_FRAMES_IN_FLIGHT, diffuseImageCreators, alphaImageCreators, specularImageCreators, depthImageView, depthSampler,
                         uniformMVPBuffers, lightBuffers, mainLightDataBuffers);
}

void GeometryDescriptors::createDescriptorSetLayout(VkDevice device, const uint32_t numMaterials, const uint32_t numTextures)
{
    // Se crea un vector que almacene todos los bindings, las matrices y las texturas
    // Las texturas se almacenan en un array dentro del mismo binding, para seleccionar la correcta en función del ID del objeto
    std::vector<VkDescriptorSetLayoutBinding> bindings(5);
    // El primer binding del descriptor se reserva para las matrices de transformación
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;                                                    // Como solo se tiene un objeto, solo se necesita un descriptor
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // Se especifica que la información sólo se va a utilizar en el shader de vértices
    bindings[0].pImmutableSamplers = nullptr;
    // El siguiente binding es para el array de texturas
    bindings[1].binding = 1;
    bindings[1].descriptorCount = numMaterials * numTextures; // Un array con el total de texturas a usar de la escena
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].pImmutableSamplers = nullptr;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    // El tercer binding es para el conjunto de luces
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].pImmutableSamplers = nullptr;
    // El cuarto binding se utiliza para cargar la matriz de la fuente de luz
    bindings[3].binding = 3;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[3].pImmutableSamplers = nullptr;
    // El quinto binding se utiliza para cargar la imagen de profundidad
    bindings[4].binding = 4;
    bindings[4].descriptorCount = 1;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].pImmutableSamplers = nullptr;
    bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

// Función para crear el pool de de descriptores, para poder crear un conjunto de ellos
void GeometryDescriptors::createDescriptorPool(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT)
{
    // Creación del pool
    std::vector<VkDescriptorPoolSize> poolSizes(5);
    // Pool para las matrices
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Variables uniformes
    poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    // Pool para las texturas
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // Textura
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * numTextures * numMaterials);
    // Pool para las luces
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    // Pool para la matriz de luz
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[3].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    // Pool para la imagen de profundidad
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    // Creación del pool
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

// Función para asignar los conjuntos de descriptores
void GeometryDescriptors::createDescriptorSets(VkDevice device, uint32_t numTextures, uint32_t numMaterials, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<ImageCreator> &diffuseImageCreators,
                                               std::vector<ImageCreator> &alphaImageCreators, std::vector<ImageCreator> &specularImageCreators, VkImageView depthImageView, VkSampler depthSampler,
                                               std::vector<VkBuffer> uniformMVPBuffers, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffers)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;                                  // Se indica el pool
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // Número de conjuntos
    allocInfo.pSetLayouts = layouts.data();
    // Se crean todos los conjuntos
    // Esta función asigna un buffer a cada conjunto
    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    // Configuración de los descriptores, uno para cada frame
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        // Conjunto de datos que almacena el descriptor set
        std::vector<VkWriteDescriptorSet> descriptorWrites(5);

        // Asignación del UBO
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

        // Asignación del UBO de luces
        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = lightBuffers[i];
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(LightsData);

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &lightBufferInfo;

        // UBO con la información de la luz
        VkDescriptorBufferInfo mainLightBufferInfo{};
        mainLightBufferInfo.buffer = mainLightDataBuffers[i];
        mainLightBufferInfo.offset = 0;
        mainLightBufferInfo.range = sizeof(UniformDataOffscreen);

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &mainLightBufferInfo;

        // Mapa de profundidad para calcular el sombreado
        // Descriptor con la información de la imagen de profundidad
        VkDescriptorImageInfo shadowMapDescriptor{};
        shadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        shadowMapDescriptor.imageView = depthImageView;
        shadowMapDescriptor.sampler = depthSampler;

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &shadowMapDescriptor;

        // Actualización del descriptor
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void GeometryDescriptors::cleanupDescriptors(VkDevice device)
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

VkDescriptorSetLayout GeometryDescriptors::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

VkDescriptorSet GeometryDescriptors::getDescriptorSet(int index)
{
    return descriptorSets[index];
}