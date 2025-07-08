#include "SSAOPipeline.h"

#include "Scene/Models/MeshLoader.h"
#include "Render_Passes/Utils/DepthBuffer.h"
#include "Tools/ShaderStagesCreator.h"

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

void SSAOPipeline::createGraphicsPipeline(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass)
{
    // Creación del pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // Características para el pipeline

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 0xf;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Creación de las etapas programables
    auto vertShaderCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/ssao_fullscreen_vertex.spv");
    VkShaderModule vertShaderModule = ShaderStagesCreator::createShaderModule(vertShaderCode, device);
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    // Constantes que se van a utilizar en el shader
    struct SSAOSpecializationData
    {
        uint32_t kernelSize = SSAO_KERNEL_SIZE;
        float radius = SSAO_RADIUS;
    } specializationData;

    VkSpecializationMapEntry specializationMapEntryKernelSize{};
    specializationMapEntryKernelSize.constantID = 0;
    specializationMapEntryKernelSize.offset = offsetof(SSAOSpecializationData, kernelSize);
    specializationMapEntryKernelSize.size = sizeof(SSAOSpecializationData::kernelSize);

    VkSpecializationMapEntry specializationMapEntryRadius{};
    specializationMapEntryRadius.constantID = 1;
    specializationMapEntryRadius.offset = offsetof(SSAOSpecializationData, radius);
    specializationMapEntryRadius.size = sizeof(SSAOSpecializationData::radius);

    std::array<VkSpecializationMapEntry, 2> specializationMapEntries = {
        specializationMapEntryKernelSize,
        specializationMapEntryRadius};
    VkSpecializationInfo specializationInfo{};
    specializationInfo.mapEntryCount = 2;
    specializationInfo.pMapEntries = specializationMapEntries.data();
    specializationInfo.dataSize = sizeof(specializationData);
    specializationInfo.pData = &specializationData;

    auto fragmentShaderCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/ssao_generation_frag.spv");
    VkShaderModule fragmentShaderModule = ShaderStagesCreator::createShaderModule(fragmentShaderCode, device);
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";
    fragmentShaderStageInfo.pSpecializationInfo = &specializationInfo;

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragmentShaderStageInfo};

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    // Estructuras
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pDynamicState = &dynamicState;
    // Pipeline layout
    pipelineInfo.layout = pipelineLayout;
    // Render passes
    pipelineInfo.renderPass = renderPass;
    // Entrada de vértices vacía para esta etapa
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    // Creación del pipeline
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &geometryPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // Los módulos sólo se necesitan para la creación del cauce gráfico, una vez hecho, se destruyen
    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}