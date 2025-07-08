#include "SurfelsCompositionDescriptors.h"

#include "Buffers/GeometryUniformBuffer.h"
#include "Buffers/SSAOBufferManager.h"

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

void SurfelsCompositionDescriptors::createDescriptors(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT, std::vector<VkBuffer> uniformSSAOParamsBuffers, VkSampler colorSampler,
                                                          VkImageView positionImageView, VkImageView normalImageView, VkImageView albedoImageView, VkImageView colorSSAOBlurImageView,
                                                          VkImageView depthImageView, VkSampler depthSampler, std::vector<VkBuffer> lightBuffers, std::vector<VkBuffer> mainLightDataBuffers,
                                                          std::vector<VkBuffer> uniformMVPBuffers, VkImageView specularImageView, VkImageView indirectDiffuseImageView,
                                                          VkImageView surfelsVisualizationImageView)
{
    // Descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 10 * MAX_FRAMES_IN_FLIGHT;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 12 * MAX_FRAMES_IN_FLIGHT;

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
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(11);

    setLayoutBindings[0].binding = 0;
    setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[0].descriptorCount = 1;
    setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[0].pImmutableSamplers = nullptr;

    setLayoutBindings[1].binding = 1;
    setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[1].descriptorCount = 1;
    setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[1].pImmutableSamplers = nullptr;

    setLayoutBindings[2].binding = 2;
    setLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[2].descriptorCount = 1;
    setLayoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[2].pImmutableSamplers = nullptr;

    setLayoutBindings[3].binding = 3;
    setLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[3].descriptorCount = 1;
    setLayoutBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[3].pImmutableSamplers = nullptr;

    setLayoutBindings[4].binding = 4;
    setLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[4].descriptorCount = 1;
    setLayoutBindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[4].pImmutableSamplers = nullptr;

    setLayoutBindings[5].binding = 5;
    setLayoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[5].descriptorCount = 1;
    setLayoutBindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[5].pImmutableSamplers = nullptr;

    setLayoutBindings[6].binding = 6;
    setLayoutBindings[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[6].descriptorCount = 1;
    setLayoutBindings[6].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[6].pImmutableSamplers = nullptr;

    setLayoutBindings[7].binding = 7;
    setLayoutBindings[7].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBindings[7].descriptorCount = 1;
    setLayoutBindings[7].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[7].pImmutableSamplers = nullptr;

    setLayoutBindings[8].binding = 8;
    setLayoutBindings[8].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[8].descriptorCount = 1;
    setLayoutBindings[8].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[8].pImmutableSamplers = nullptr;

    setLayoutBindings[9].binding = 9;
    setLayoutBindings[9].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[9].descriptorCount = 1;
    setLayoutBindings[9].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[9].pImmutableSamplers = nullptr;

    setLayoutBindings[10].binding = 10;
    setLayoutBindings[10].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBindings[10].descriptorCount = 1;
    setLayoutBindings[10].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBindings[10].pImmutableSamplers = nullptr;

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
        std::vector<VkWriteDescriptorSet> descriptorWrites(11);

        // Binding 0 -> Imagen con la posición
        VkDescriptorImageInfo positionImageDescriptor{};
        positionImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        positionImageDescriptor.imageView = positionImageView;
        positionImageDescriptor.sampler = colorSampler;

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &positionImageDescriptor;

        // Binding 1 -> Imagen con las normales
        VkDescriptorImageInfo normalImageDescriptor{};
        normalImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalImageDescriptor.imageView = normalImageView;
        normalImageDescriptor.sampler = colorSampler;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &normalImageDescriptor;

        // Binding 2 -> Imagen con el albedo de la segunda pasada
        VkDescriptorImageInfo albedoImageDescriptor{};
        albedoImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoImageDescriptor.imageView = albedoImageView;
        albedoImageDescriptor.sampler = colorSampler;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &albedoImageDescriptor;

        // Binding 3 -> Imagen con el color de la pasada de SSAO Blur
        VkDescriptorImageInfo colorSSAOBlurImageDescriptor{};
        colorSSAOBlurImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorSSAOBlurImageDescriptor.imageView = colorSSAOBlurImageView;
        colorSSAOBlurImageDescriptor.sampler = colorSampler;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &colorSSAOBlurImageDescriptor;

        // Binding 4 -> Información de las luces de la escena
        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = lightBuffers[i];
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(LightsData);

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pBufferInfo = &lightBufferInfo;

        // Binding 5 -> Matriz para pasar al espacio de la luz principal
        VkDescriptorBufferInfo mainLightBufferInfo{};
        mainLightBufferInfo.buffer = mainLightDataBuffers[i];
        mainLightBufferInfo.offset = 0;
        mainLightBufferInfo.range = sizeof(UniformDataOffscreen);

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = descriptorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pBufferInfo = &mainLightBufferInfo;

        // Binding 6 -> Shadow Map
        VkDescriptorImageInfo shadowMapDescriptor{};
        shadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        shadowMapDescriptor.imageView = depthImageView;
        shadowMapDescriptor.sampler = depthSampler;

        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[6].dstSet = descriptorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pImageInfo = &shadowMapDescriptor;

        // Binding 7 -> Matrices para revertir la transformación en espacio de cámara para shadow mapping
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformMVPBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[7].dstSet = descriptorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[7].descriptorCount = 1;
        descriptorWrites[7].pBufferInfo = &bufferInfo;

        // Binding 8 -> Imagen con el brillo especular de la segunda pasada
        VkDescriptorImageInfo specularImageDescriptor{};
        specularImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        specularImageDescriptor.imageView = specularImageView;
        specularImageDescriptor.sampler = colorSampler;

        descriptorWrites[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[8].dstSet = descriptorSets[i];
        descriptorWrites[8].dstBinding = 8;
        descriptorWrites[8].dstArrayElement = 0;
        descriptorWrites[8].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[8].descriptorCount = 1;
        descriptorWrites[8].pImageInfo = &specularImageDescriptor;

        // Binding 9 -> Imagen con el mapa de iluminación difusa indirecta
        VkDescriptorImageInfo indirectDiffuseImageDescriptor{};
        indirectDiffuseImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        indirectDiffuseImageDescriptor.imageView = indirectDiffuseImageView;
        indirectDiffuseImageDescriptor.sampler = colorSampler;

        descriptorWrites[9].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[9].dstSet = descriptorSets[i];
        descriptorWrites[9].dstBinding = 9;
        descriptorWrites[9].dstArrayElement = 0;
        descriptorWrites[9].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[9].descriptorCount = 1;
        descriptorWrites[9].pImageInfo = &indirectDiffuseImageDescriptor;

        // Binding 10 -> Imagen con la visualización de los surfels
        VkDescriptorImageInfo surfelsVisualizationImageDescriptor{};
        surfelsVisualizationImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        surfelsVisualizationImageDescriptor.imageView = surfelsVisualizationImageView;
        surfelsVisualizationImageDescriptor.sampler = colorSampler;

        descriptorWrites[10].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[10].dstSet = descriptorSets[i];
        descriptorWrites[10].dstBinding = 10;
        descriptorWrites[10].dstArrayElement = 0;
        descriptorWrites[10].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[10].descriptorCount = 1;
        descriptorWrites[10].pImageInfo = &surfelsVisualizationImageDescriptor;

        // Actualización del descriptor
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void SurfelsCompositionDescriptors::cleanupDescriptors(VkDevice device)
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

VkDescriptorSetLayout SurfelsCompositionDescriptors::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

VkDescriptorSet SurfelsCompositionDescriptors::getDescriptorSet(int index)
{
    return descriptorSets[index];
}