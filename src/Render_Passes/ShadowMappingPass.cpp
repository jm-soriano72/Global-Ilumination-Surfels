#include "ShadowMappingPass.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

ShadowMappingPass::ShadowMappingPass() {}

void ShadowMappingPass::createRenderPass(VkDevice device, VkFormat format, VkPhysicalDevice physicalDevice)
{
    VkFormat depthFormat = DepthBuffer::findDepthFormat(physicalDevice);

    // Se crea un sampler para el depth attachment
    VkFilter shadowMapFilter = formatIsFilterable(physicalDevice, depthFormat, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = shadowMapFilter;
    samplerInfo.minFilter = shadowMapFilter;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vkCreateSampler(device, &samplerInfo, nullptr, &depthSampler);

    // Información del attachment de profundidad
    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = depthFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Se va a utilizar la información en la siguiente pasada, es necesario guardarla
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 0;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0; // No se va a almacenar color, sólo se necesita la profundidad
    subpass.pDepthStencilAttachment = &depthReference;

    // Dependencias para configurar la pasada
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassCreateInfo.pDependencies = dependencies.data();

    vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
}

void ShadowMappingPass::createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                                           VkQueue graphicsQueue, VkExtent2D swapChainExtent)
{
    // Antes de crear los framebuffer, se crean los recursos de profundidad
    depthBufferCreator.createShadowDepthResources(device, physicalDevice, swapChainExtent.width, swapChainExtent.height);

    // Se necesita crear un framebuffer para cada una de las imágenes del swap chain
    framebuffers.resize(numImageViews);

    for (size_t i = 0; i < numImageViews; i++)
    {
        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &depthBufferCreator.depthImageView;
        framebufferCreateInfo.width = swapChainExtent.width;
        framebufferCreateInfo.height = swapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]);
    }

}

// Se comprueba si el formato de imagen es filtrable o no
bool ShadowMappingPass::formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

    if (tiling == VK_IMAGE_TILING_LINEAR)
    {
        return (formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
    }
    else if (tiling == VK_IMAGE_TILING_OPTIMAL)
    {
        return (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
    }

    return false;
}

void ShadowMappingPass::cleanup(VkDevice device)
{
    depthBufferCreator.cleanupDepthResources(device);
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (auto &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    vkDestroySampler(device, depthSampler, nullptr);
}

void ShadowMappingPass::cleanupFramebuffers(VkDevice device) 
{
    depthBufferCreator.cleanupDepthResources(device);
    for (auto &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}

VkSampler ShadowMappingPass::getDepthSampler()
{
    return this->depthSampler;
}

VkImageView ShadowMappingPass::getDepthImageView()
{
    return this->depthBufferCreator.depthImageView;
}
