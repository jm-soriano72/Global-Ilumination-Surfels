#include "ImageCreator.h"

#include "RenderApplication.h"
#include "Buffers/Tools/BufferCreator.h"
#include "Buffers/Tools/CommandBufferManager.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define STB_IMAGE_IMPLEMENTATION

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>
#include <stb_image.h>

#include <vector>
#include <array>

void ImageCreator::createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const char *texturePath)
{
    // Se carga la imagen y se almacenan sus pixels, junto con su tamaño
    int texWidth, texHeight, texChannels;
    // Se utiliza la variable definida en CMake para referir donde se encuentran los recursos
    stbi_uc *pixels = stbi_load(texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }
    // Creación del buffer
    BufferCreator::createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    // Se copian los pixels cargados con la librería al buffer
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);
    // Se libera el espacio
    stbi_image_free(pixels);
    // Creación del objeto imagen
    createImage(device, physicalDevice, texWidth, texHeight,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, "SceneTexture");

    // Copia del buffer a la imagen creada
    // Primero se indica la etiqueta de la imagen, como destino de copia de datos
    transitionImageLayout(commandPool, device, graphicsQueue, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // Después se hace la copia como tal
    copyBufferToImage(commandPool, device, graphicsQueue, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    // Se crean los mip maps
    generateMipmaps(commandPool, device, graphicsQueue, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight);
    
    // Se libera la memoria de los buffers auxiliares
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void ImageCreator::createNoiseTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<glm::vec4> *noiseValues, uint32_t texWidth, uint32_t texHeight)
{
    VkCommandBuffer commandBuffer = CommandBufferManager::beginSingleTimeCommands(commandPool, device);
    VkDeviceSize imageSize = noiseValues->size() * sizeof(glm::vec4);

    // Creación del buffer
    BufferCreator::createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, noiseValues->data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);
    // Creación de la imagen
    createImage(device, physicalDevice, texWidth, texHeight,
                VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, "NoiseTexture");
    // Copia del buffer a la imagen creada
    transitionImageLayout(commandPool, device, graphicsQueue, textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(commandPool, device, graphicsQueue, stagingBuffer, textureImage, texWidth, texHeight);
    transitionImageLayout(commandPool, device, graphicsQueue, textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
    // Creación de la image view
    textureImageView = createImageView(device, textureImage, VK_FORMAT_R32G32B32A32_SFLOAT,
                                       VK_IMAGE_ASPECT_COLOR_BIT, false);
    // Creación del sampler
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;

    if (vkCreateSampler(device, &samplerCreateInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }

    CommandBufferManager::endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
}

void ImageCreator::createRaysNoiseTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<glm::vec2> *noiseValues, uint32_t texWidth, uint32_t texHeight)
{
    VkCommandBuffer commandBuffer = CommandBufferManager::beginSingleTimeCommands(commandPool, device);
    VkDeviceSize imageSize = noiseValues->size() * sizeof(glm::vec2);

    // Creación del buffer
    BufferCreator::createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, noiseValues->data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);
    // Creación de la imagen
    createImage(device, physicalDevice, texWidth, texHeight,
                VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, "NoiseRaysTexture");
    // Copia del buffer a la imagen creada
    transitionImageLayout(commandPool, device, graphicsQueue, textureImage, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(commandPool, device, graphicsQueue, stagingBuffer, textureImage, texWidth, texHeight);
    transitionImageLayout(commandPool, device, graphicsQueue, textureImage, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
    // Creación de la image view
    textureImageView = createImageView(device, textureImage, VK_FORMAT_R32G32_SFLOAT,
                                       VK_IMAGE_ASPECT_COLOR_BIT, false);
    // Creación del sampler
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;

    if (vkCreateSampler(device, &samplerCreateInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }

    CommandBufferManager::endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
}

void ImageCreator::createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                               uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                               VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory, char *imageName)
{
    // Creación de los objetos para las imágenes
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D; // Se pueden crear imágenes 1D, 2D y 3D
    // Dimensiones de la imagen
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    if (strcmp(imageName, "SceneTexture") == 0)
    {
        imageInfo.mipLevels = MIP_MAP_LEVELS;
    }
    else
    {
        imageInfo.mipLevels = 1;
    }
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    // std::cout << std::endl << "Image name: " << imageName << " Handler: " << (void*)image << std::endl;

    // Reserva de memoria para el objeto imagen
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = BufferCreator::findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

void ImageCreator::transitionImageLayout(VkCommandPool commandPool, VkDevice device, VkQueue graphicsQueue,
                                         VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{

    VkCommandBuffer commandBuffer = CommandBufferManager::beginSingleTimeCommands(commandPool, device);

    // Se utiliza esta barrera para cambiar entre layouts de la imagen
    // La imagen se utilizaba como transferencia (Destino de copia de datos) y lectura (Acceso al shader)
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // Se utiliza el aspecto corecto
    // Si se comprueba que tiene el componente de profundidad, se modifica su aspecto
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        // Nueva transición añadida para solucionar el error
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    CommandBufferManager::endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
}

void ImageCreator::copyBufferToImage(VkCommandPool commandPool, VkDevice device, VkQueue graphicsQueue,
                                     VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{

    VkCommandBuffer commandBuffer = CommandBufferManager::beginSingleTimeCommands(commandPool, device);
    // Correspondencia de la imagen al buffer en el que se copia
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1};
    // Copia del buffer a la imagen
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    CommandBufferManager::endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
}

VkImageView ImageCreator::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, bool mipMapEnabled)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    if (mipMapEnabled)
    {
        viewInfo.subresourceRange.levelCount = MIP_MAP_LEVELS;
    }
    else
    {
        viewInfo.subresourceRange.levelCount = 1;
    }
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void ImageCreator::createTextureImageView(VkDevice device)
{
    textureImageView = createImageView(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_ASPECT_COLOR_BIT, true);
}

void ImageCreator::createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    // Se aplica un filtro bilinear para aumentar la calidad de la textura
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    // Se especifican las coordenadas de textura, para ver qué ocurre cuando se sale de la textura
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    // Se activa el filtro anisotrópico
    samplerInfo.anisotropyEnable = VK_TRUE;
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    // Se utilizan los límites para lograr la máxima calidad
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    // Se especifica el color del borde de la textura
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    // Se indica si se utilizan coordenadas normalizadas o no
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    // Parámetros para el mipmapping
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(MIP_MAP_LEVELS);

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void ImageCreator::generateMipmaps(VkCommandPool commandPool, VkDevice device, VkQueue graphicsQueue,
                                   VkFormat imageFormat, uint32_t texWidth, uint32_t texHeight)
{
    VkCommandBuffer commandBuffer = CommandBufferManager::beginSingleTimeCommands(commandPool, device);

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    // Transición inicial del mip 0 a TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = textureImage;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    for (uint32_t i = 1; i < MIP_MAP_LEVELS; i++)
    {
        // Transición mip i-1: TRANSFER_DST_OPTIMAL → TRANSFER_SRC_OPTIMAL
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);

        // Transición del mip i a TRANSFER_DST_OPTIMAL (ANTES del blit)
        barrier.subresourceRange.baseMipLevel = i;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);

        // Blit de mip i-1 → mip i
        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;

        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {std::max(mipWidth / 2, 1), std::max(mipHeight / 2, 1), 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
                       textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        // Transición mip i-1: TRANSFER_SRC_OPTIMAL → SHADER_READ_ONLY_OPTIMAL
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    // Transición final del último mip (MIP_MAP_LEVELS - 1) a SHADER_READ_ONLY_OPTIMAL
    barrier.subresourceRange.baseMipLevel = MIP_MAP_LEVELS - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    CommandBufferManager::endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
}

bool ImageCreator::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void ImageCreator::cleanup(VkDevice device)
{
    if (textureSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, textureSampler, nullptr);
    }
    vkDestroyImageView(device, textureImageView, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
}
