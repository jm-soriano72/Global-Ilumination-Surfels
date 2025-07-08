#include "SSAOPass.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <chrono>
#include <vector>
#include <array>
#include <random>
#include <ctime>

SSAOPass::SSAOPass() {}

void SSAOPass::createRenderPass(VkDevice device, VkFormat format, VkPhysicalDevice physicalDevice)
{
    // Esta pasada de renderizado cuenta con un único attachment, de color
    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = VK_FORMAT_R8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = &colorReference;
    subpass.colorAttachmentCount = 1;

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pAttachments = &attachmentDescription;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void SSAOPass::createImageAttachments(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D swapChainExtent)
{
    // Attachment 0: Color
    colorImage.createImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage.textureImage, colorImage.textureImageMemory, "SSAO-Out");
    colorImage.textureImageView = colorImage.createImageView(device, colorImage.textureImage, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, false);
}

void SSAOPass::createNoiseImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

    std::vector<glm::vec4> *noiseValues = new std::vector<glm::vec4>();
    for (uint32_t i = 0; i < SSAO_NOISE_DIM * SSAO_NOISE_DIM; i++)
    {
        noiseValues->push_back(glm::vec4(rndDist(rndEngine) * 2.0f - 1.0f, rndDist(rndEngine) * 2.0f - 1.0f, 0.0f, 0.0f));
    }
    noiseImage.createNoiseTextureImage(device, physicalDevice, commandPool, graphicsQueue, noiseValues, SSAO_NOISE_DIM, SSAO_NOISE_DIM);
}

void SSAOPass::createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                                  VkQueue graphicsQueue, VkExtent2D swapChainExtent)
{
    createImageAttachments(device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);
    createNoiseImage(device, physicalDevice, commandPool, graphicsQueue);
    std::array<VkImageView, 1> attachments;
    attachments[0] = colorImage.textureImageView;

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

void SSAOPass::cleanup(VkDevice device)
{
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (auto &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    colorImage.cleanup(device);
    noiseImage.cleanup(device);
}

void SSAOPass::cleanupFramebuffers(VkDevice device)
{
    for (auto &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    colorImage.cleanup(device);
    noiseImage.cleanup(device);
}

VkImageView SSAOPass::getColorImageView()
{
    return this->colorImage.textureImageView;
}

ImageCreator SSAOPass::getNoiseImage()
{
    return this->noiseImage;
}
