#include "ComputePipeline.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

void ComputePipeline::cleanup(VkDevice device)
{
    vkDestroyPipeline(device, geometryPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

VkPipelineLayout ComputePipeline::getPipelineLayout()
{
    return this->pipelineLayout;
}

VkPipeline ComputePipeline::getGraphicsPipeline()
{
    return this->geometryPipeline;
}