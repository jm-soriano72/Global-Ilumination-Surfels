#include "SurfelsRadianceCalculationPipeline.h"

#include "Scene/Models/MeshLoader.h"
#include "Render_Passes/Utils/DepthBuffer.h"
#include "Tools/ShaderStagesCreator.h"
#include "Buffers/UniformBuffersManager.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <cstdint>
#include <limits>
#include <algorithm>

void SurfelsRadianceCalculationPipeline::createGraphicsPipeline(VkDevice device, VkDescriptorSetLayout descriptorSetLayout)
{
    auto computeShaderCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/surfel_radiance_calculation.spv");
    VkShaderModule computeShaderModule = ShaderStagesCreator::createShaderModule(computeShaderCode, device);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkPushConstantRange pushConstRange{};
    pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
    pushConstRange.offset = 0;
    pushConstRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstRange;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkComputePipelineCreateInfo computePipelineCI = {};
    computePipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCI.stage = computeShaderStageInfo;
    computePipelineCI.layout = pipelineLayout;

    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCI, nullptr, &geometryPipeline);

    vkDestroyShaderModule(device, computeShaderModule, nullptr);
}