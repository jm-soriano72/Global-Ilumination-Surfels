#include "GeometryPipeline.h"

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

void GeometryPipeline::createGraphicsPipeline(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass)
{
    auto vertShaderCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/vertex.spv");
    auto fragShaderCode = ShaderStagesCreator::readFile(RESOURCES_PATH "shaders/frag.spv");

    // Se crean los módulos
    VkShaderModule vertShaderModule = ShaderStagesCreator::createShaderModule(vertShaderCode, device);
    VkShaderModule fragShaderModule = ShaderStagesCreator::createShaderModule(fragShaderCode, device);

    // Creación de las etapas del cauce gráfico
    // Etapa de vértices
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; // Se indica en qué etapa se va a usar el shader
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule; // Módulo del shader
    vertShaderStageInfo.pName = "main";            // Punto de entrada
    // vertShaderStageInfo.pSpecializationInfo se puede utilizar para establecer valores para las constantes de los shaders
    // Etapa de fragmentos
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    // Array con las etapas programables
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Especificación del formato de los vértices de entrada
    // Se toma la información de la clase VertexBuffer
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // Se obtiene la información para bind
    auto bindingDescription = Vertex::getBindingDescription();
    // Se obtiene la especificación para los atributos de los vértices
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Tratamiento de los vértices, cómo se va a dibujar la geometría
    // Topología:
    // - Cada vértice es un punto
    // - Se crea una línea entre cada dos vértices, sin reutilizarlos
    // - Se crea una línea entre cada dos vértices, reutilizando el último
    // - Se crea un triángulo entre cada 3 vértices, sin reutilizarlos
    // - Se crea un triángulo entre cada 3 vértices, reutilizando el 2 y 3
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Configuración del viewport (dinámico)
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // El scissor determina la parte de la imagen que se va a mostrar, se puede recortar
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    // Definición de elementos que van a tener un estado dinámico -> viewport y scissor
    // Al ser dinámicos, habrá que indicar su valor en el momento de la ejecución

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Se indica que el viewport y el scissor van a ser inmutables
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // El rasterizador toma la geometría creada mediante los vértices y lo convierte en fragmentos tratables por el shader
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Si se activa esto, ninguna geometría sobrepasa la etapa de fragmentos y nada se muestra por pantalla
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Se indica cómo se generan los fragmentos a partir de la geometría. Este indica que se van rellenando los polígonos con fragmentos
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;            // Configuración del culling
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Sentido horario o antihorario
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

    // Configuración del multisampling, para evitar aliasing
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    // Color blending -> configuración del color del fragmento en función de la salida del shader y el color en el framebuffer
    // Se puede hacer con una combinación de ambos o una operación bitwise
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    // Array de estructuras para todos los framebuffers, que permite establecer constantes para calcular el color en el proceso de blend
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;   // Para seguir la segunda opción, con bitwise
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    // Se indican las Push Constants que se van a utilizar
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Se usará en el fragment shader
    pushConstantRange.offset = 0;                                // Offset dentro del bloque
    pushConstantRange.size = sizeof(PushConstantsData);

    // Creación del pipeline layout, para pasar variables uniform a los shaders
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // Se indican los descriptores, es decir las especificaciones necesarias para pasar variables uniformes a los shaders
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // Se activa la detección de la profundidad
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // Comparación para descartar fragmentos
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    // Se utilizan límites para poder descartar fragmentos en un rango
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {};  // Optional

    // Se combinan todos los elementos anteriores para crear el pipeline de renderizado como tal
    // Las partes necesarias son:
    // - Módulos de las etapas programables (shaders)
    // - Conjunto de estructuras que definen características del pipeline (rasterizador, color blending, supersampling...)
    // - Pipeline layout, que indica las variables uniformes que pueden ser utilizadas por los shaders
    // - Render pass, que indica todas las asignaciones referenciadas por los shaders (framebuffer de color)

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;         // Número de etapas programables
    pipelineInfo.pStages = shaderStages; // Módulos de las etapas progamables
    // Estructuras
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pDepthStencilState = &depthStencil;
    // Pipeline layout
    pipelineInfo.layout = pipelineLayout;
    // Render passes
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // Índice del subpass que se va a utilizar
    // Vulkan permite derivar un pipeline de renderizado que derive de otro, ya que es más eficiente si tiene muchas funcionalidades en común
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    // Creación del pipeline
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &geometryPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // Los módulos sólo se necesitan para la creación del cauce gráfico, una vez hecho, se destruyen
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}