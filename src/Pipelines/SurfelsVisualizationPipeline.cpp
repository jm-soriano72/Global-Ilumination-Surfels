#include "SurfelsVisualizationPipeline.h"

#include "Scene/Models/MeshLoader.h"
#include "Render_Passes/Utils/DepthBuffer.h"
#include "Tools/ShaderStagesCreator.h"
#include "Buffers/SurfelsBufferManager.h"
#include "Config.h"

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

void SurfelsVisualizationPipeline::createGraphicsPipeline(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass)
{
    VkPushConstantRange pushConstRange{};
    pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
    pushConstRange.offset = 0;
    pushConstRange.size = sizeof(PushConstants);

    // Se crea el Pipeline Layout
    VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstRange;
    vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

    // Se habilita que se puedan dibujar los Points para visualizar los surfels
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Se especifica el Input de Vértices (en este caso de los datos para cada punto)
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(SimpleSurfel);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 4> attribs{};
    attribs[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleSurfel, position)};
    attribs[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleSurfel, normal)};
    attribs[2] = {2, 0, VK_FORMAT_R32_SFLOAT, offsetof(SimpleSurfel, radius)};
    attribs[3] = {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleSurfel, color)};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attribs.size();
    vertexInputInfo.pVertexAttributeDescriptions = attribs.data();

    VkPipelineRasterizationStateCreateInfo rasterizer{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.lineWidth = 1.0f;

    VkViewport viewport{0, 0, (float)swapChainExtent.width, (float)swapChainExtent.height, 0.0f, 1.0f};
    VkRect2D scissor{{0, 0}, swapChainExtent};
    VkPipelineViewportStateCreateInfo vpState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vpState.viewportCount = 1;
    vpState.pViewports = &viewport;
    vpState.scissorCount = 1;
    vpState.pScissors = &scissor;

    VkPipelineMultisampleStateCreateInfo msState{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    msState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState colorAttach{};
    colorAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorAttach.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo blendState{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blendState.attachmentCount = 1;
    blendState.pAttachments = &colorAttach;

    std::array<VkDynamicState, 2> dynStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynState{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynState.dynamicStateCount = (uint32_t)dynStates.size();
    dynState.pDynamicStates = dynStates.data();

    // Creación de las etapas de la pasada (VÉRTICES, GEOMETRÍA Y FRAGMENTOS)
    std::vector<char> vertCode;
    if (renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION) vertCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/surfel_visualization_vert.spv");
    else if (renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION) vertCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/surfel_radiance_visualization_vert.spv");
    auto geomCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/surfel_visualization_geom.spv");
    auto fragCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/surfel_visualization_frag.spv");

    VkShaderModule vertSM = ShaderStagesCreator::createShaderModule(vertCode, device);
    VkShaderModule geomSM = ShaderStagesCreator::createShaderModule(geomCode, device);
    VkShaderModule fragSM = ShaderStagesCreator::createShaderModule(fragCode, device);

    VkPipelineShaderStageCreateInfo vertStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertSM;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo geomStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    geomStage.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    geomStage.module = geomSM;
    geomStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragSM;
    fragStage.pName = "main";

    std::array<VkPipelineShaderStageCreateInfo, 3> shaderStages = {vertStage, geomStage, fragStage};

    // Creación del pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount = (uint32_t)shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &vpState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &msState;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &blendState;
    pipelineInfo.pDynamicState = &dynState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &geometryPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create surfel graphics pipeline!");
    }

    // Se eliminan los módulos auxiliares
    vkDestroyShaderModule(device, vertSM, nullptr);
    vkDestroyShaderModule(device, geomSM, nullptr);
    vkDestroyShaderModule(device, fragSM, nullptr);
}