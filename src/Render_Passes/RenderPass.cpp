#include "RenderPass.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

RenderPass::RenderPass() {}

VkRenderPass RenderPass::getRenderPass()
{
    return this->renderPass;
}

VkFramebuffer RenderPass::getFramebuffer(int index)
{
    return framebuffers[index];
}

std::vector<VkFramebuffer> RenderPass::getFramebuffers()
{
    return this->framebuffers;
}

DepthBuffer RenderPass::getDepthBufferCreator()
{
    return this->depthBufferCreator;
}