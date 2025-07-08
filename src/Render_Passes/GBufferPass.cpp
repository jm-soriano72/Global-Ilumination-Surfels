#include "GBufferPass.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"
#include "Config.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

GBufferPass::GBufferPass() {}

void GBufferPass::createRenderPass(VkDevice device, VkFormat format, VkPhysicalDevice physicalDevice)
{
    // Attachment para la posición
    VkAttachmentDescription positionAttachment{};
    positionAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    positionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    positionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    positionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    positionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    positionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    positionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Attachment para las normales
    VkAttachmentDescription normalAttachment{};
    normalAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    normalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Attachment para el color
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Attachment para la componente especular
    VkAttachmentDescription specularAttachment{};
    specularAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    specularAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    specularAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    specularAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    specularAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    specularAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    specularAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Attachment para la profundidad
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = DepthBuffer::findDepthFormat(physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    if (renderConfig == RenderMode::SSAO)
    {
        std::array<VkAttachmentDescription, 4> attachmentDescs = {positionAttachment, normalAttachment, colorAttachment, depthAttachment};

        // Attachment references (3 de color y 1 de profundidad)
        std::vector<VkAttachmentReference> colorReferences;
        colorReferences.push_back({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 3;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Creacíón de la subpass y las dependencias para las transiciones entre attachments
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pColorAttachments = colorReferences.data();
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpass.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for attachment layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Creación de la render pass
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
        renderPassInfo.pAttachments = attachmentDescs.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        std::array<VkAttachmentDescription, 5> attachmentDescs = {positionAttachment, normalAttachment, colorAttachment, specularAttachment, depthAttachment};

        // Attachment references (4 de color y 1 de profundidad)
        std::vector<VkAttachmentReference> colorReferences;
        colorReferences.push_back({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 4;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Creacíón de la subpass y las dependencias para las transiciones entre attachments
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pColorAttachments = colorReferences.data();
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpass.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for attachment layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Creación de la render pass
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
        renderPassInfo.pAttachments = attachmentDescs.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        std::array<VkAttachmentDescription, 5> attachmentDescs = {positionAttachment, normalAttachment, colorAttachment, specularAttachment, depthAttachment};

        // Attachment references (4 de color y 1 de profundidad)
        std::vector<VkAttachmentReference> colorReferences;
        colorReferences.push_back({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        colorReferences.push_back({3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 4;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Creacíón de la subpass y las dependencias para las transiciones entre attachments
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pColorAttachments = colorReferences.data();
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpass.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for attachment layout transitions
        std::array<VkSubpassDependency, 4> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[2].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[2].dstSubpass = 0;
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[3].srcSubpass = 0;
        dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[3].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[3].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[3].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[3].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Creación de la render pass
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
        renderPassInfo.pAttachments = attachmentDescs.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 4;
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }
}

void GBufferPass::createImageAttachments(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D swapChainExtent)
{
    // Attachment 0: Posición
    positionImage.createImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, positionImage.textureImage, positionImage.textureImageMemory, "GBuffer-Position");
    positionImage.textureImageView = positionImage.createImageView(device, positionImage.textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, false);
    // Attachment 1: Normales
    normalImage.createImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, normalImage.textureImage, normalImage.textureImageMemory, "GBuffer-Normal");
    normalImage.textureImageView = normalImage.createImageView(device, normalImage.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, false);
    // Attachment 2: Albedo
    albedoImage.createImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, albedoImage.textureImage, albedoImage.textureImageMemory, "GBuffer-Color");
    albedoImage.textureImageView = albedoImage.createImageView(device, albedoImage.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, false);
    if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        // Attachment 3: Especular
        specularImage.createImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, specularImage.textureImage, specularImage.textureImageMemory, "GBuffer-Specular");
        specularImage.textureImageView = specularImage.createImageView(device, specularImage.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, false);
    }
    // Attachment 3/4: Profundidad
    depthBufferCreator.createDepthResources(device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
}

void GBufferPass::createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                                     VkQueue graphicsQueue, VkExtent2D swapChainExtent)
{
    createImageAttachments(device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
    if (renderConfig == RenderMode::SSAO)
    {
        // Se asocian las Image View a cada uno de los attachments de la render pass
        std::array<VkImageView, 4> attachments;
        attachments[0] = positionImage.textureImageView;
        attachments[1] = normalImage.textureImageView;
        attachments[2] = albedoImage.textureImageView;
        attachments[3] = depthBufferCreator.depthImageView;

        framebuffers.resize(numImageViews);

        for (size_t i = 0; i < numImageViews; i++)
        {
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainManager.getSwapChainExtent().width;
            framebufferInfo.height = swapChainManager.getSwapChainExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        // Se asocian las Image View a cada uno de los attachments de la render pass
        std::array<VkImageView, 5> attachments;
        attachments[0] = positionImage.textureImageView;
        attachments[1] = normalImage.textureImageView;
        attachments[2] = albedoImage.textureImageView;
        attachments[3] = specularImage.textureImageView;
        attachments[4] = depthBufferCreator.depthImageView;

        framebuffers.resize(numImageViews);

        for (size_t i = 0; i < numImageViews; i++)
        {
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainManager.getSwapChainExtent().width;
            framebufferInfo.height = swapChainManager.getSwapChainExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }
}

void GBufferPass::cleanup(VkDevice device)
{
    depthBufferCreator.cleanupDepthResources(device);
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (auto &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    positionImage.cleanup(device);
    normalImage.cleanup(device);
    albedoImage.cleanup(device);
    if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION || renderConfig == RenderMode::SURFELS_VISUALIZATION)
    {
        specularImage.cleanup(device);
    }
}

void GBufferPass::cleanupFramebuffers(VkDevice device)
{
    depthBufferCreator.cleanupDepthResources(device);
    for (auto &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    positionImage.cleanup(device);
    normalImage.cleanup(device);
    albedoImage.cleanup(device);
    if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION || renderConfig == RenderMode::SURFELS_VISUALIZATION)
    {
        specularImage.cleanup(device);
    }
}

VkImageView GBufferPass::getPositionImageView()
{
    return this->positionImage.textureImageView;
}

VkImageView GBufferPass::getNormalImageView()
{
    return this->normalImage.textureImageView;
}

VkImageView GBufferPass::getAlbedoImageView()
{
    return this->albedoImage.textureImageView;
}

VkImageView GBufferPass::getSpecularImageView()
{
    return this->specularImage.textureImageView;
}