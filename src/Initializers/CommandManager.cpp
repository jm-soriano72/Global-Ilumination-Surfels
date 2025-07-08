#include "CommandManager.h"

#include "Tools/VulkanUtils.h"
#include "Scene/Models/MeshContainer.h"
#include "Camera/Camera.h"
#include "Camera/CameraController.h"
#include "Pipelines/GeometryPipeline.h"
#include "Pipelines/SSAOPipeline.h"
#include "Buffers/SurfelsBufferManager.h"
#include "Config.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stdexcept>
#include <iostream>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

CommandManager::CommandManager()
{
    depthBiasConstant = 1.25f;
    depthBiasSlope = 1.75f;
}

void CommandManager::createCommandResources(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t MAX_FRAMES_IN_FLIGHT)
{
    createCommandPool(device, physicalDevice, surface);
    createCommandBuffers(device, MAX_FRAMES_IN_FLIGHT);
}

void CommandManager::createCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices queueFamilyIndices = VulkanUtils::findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // Solo hay dos posibles flags, para reestablecer los commands en conjunto muy frecuentemente, o poder hacerlo individualmente
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void CommandManager::createCommandBuffers(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT)
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Nivel primario, no pueden ser llamados desde otros command buffers, se llaman directamente
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CommandManager::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex,
                                         VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline,
                                         VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet *shadowMappingDescriptorSet,
                                         VkRenderPass geometryRenderPass, VkFramebuffer geometryFramebuffer, VkPipeline geometryPipeline,
                                         VkPipelineLayout geometryPipelineLayout, VkDescriptorSet *geometryDescriptorSet,
                                         std::vector<MeshContainer> sceneMeshes, Camera camera)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkClearValue clearValues[2];
    VkViewport viewport{};
    VkRect2D scissor{};

    // PRIMERA PASADA DE RENDERIZADO - Se genera el shadow map renderizando la escena desde el punto de vista de la luz //

    clearValues[0].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = shadowMappingRenderPass;
    renderPassBeginInfo.framebuffer = shadowMappingFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    // Depth bias para evitar problemas en el shadow mapping
    vkCmdSetDepthBias(
        commandBuffers[currentFrame],
        depthBiasConstant,
        0.0f,
        depthBiasSlope);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMappingPipeline);

    for (const auto &mesh : sceneMeshes)
    {
        // Dentro de cada contenedor de objetos con el mismo material, se procesa cada mesh por separado, con sus buffers individuales
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {
            // Se obtiene el Vertex Buffer del objeto actual de la escena
            VkBuffer vertexBuffers[] = {mesh.vertexMeshesData.vertexBufferList[i]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

            // Se pasa también el buffer de índices
            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.vertexMeshesData.indexBufferList[i], 0, VK_INDEX_TYPE_UINT16);

            // Se pasa el conjunto de descriptores correcto, en función del número de frame
            // No son exclusivos para cada pipeline, se pueden reutilizar
            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMappingPipelineLayout,
                                    0, 1, shadowMappingDescriptorSet, 0, nullptr);

            // Dibujar
            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // SEGUNDA PASADA DE RENDERIZADO - Se renderiza la escena aplicando el shadow mapping //

    // Parámetros de color y profundidad al limpiar la escena
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    // Se comienza el render pass para escribir el comando
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = geometryRenderPass;
    renderPassInfo.framebuffer = geometryFramebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipeline);

    for (const auto &mesh : sceneMeshes)
    {
        // Dentro de cada contenedor de objetos con el mismo material, se procesa cada mesh por separado, con sus buffers individuales
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {
            // Se obtiene el Vertex Buffer del objeto actual de la escena
            VkBuffer vertexBuffers[] = {mesh.vertexMeshesData.vertexBufferList[i]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

            // Se pasa también el buffer de índices
            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.vertexMeshesData.indexBufferList[i], 0, VK_INDEX_TYPE_UINT16);

            // Se pasan las Push Constants a los shaders
            // Se definen los datos
            PushConstantsData pushConstants;
            pushConstants.cameraPosition = camera.getPosition();
            pushConstants.enablePCF = (renderConfig == RenderMode::SHADOW_MAPPING_PCF) ? 1 : 0;
            // Asignación al shader
            vkCmdPushConstants(commandBuffers[currentFrame], geometryPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantsData), &pushConstants);

            // Se pasa el conjunto de descriptores correcto, en función del número de frame
            // No son exclusivos para cada pipeline, se pueden reutilizar
            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipelineLayout, 0, 1, geometryDescriptorSet, 0, nullptr);

            // Dibujar
            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandManager::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                                         VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet *gBufferDescriptorSet,
                                         VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet *ssaoDescriptorSet,
                                         VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet *ssaoBlurDescriptorSet,
                                         VkRenderPass ssaoCompositionRenderPass, VkFramebuffer ssaoCompositionFramebuffer, VkPipeline ssaoCompositionPipeline, VkPipelineLayout ssaoCompositionPipelineLayout, VkDescriptorSet *ssaoCompositionDescriptorSet)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkViewport viewport{};
    VkRect2D scissor{};

    std::vector<VkClearValue> clearValues(4);
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[2].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[3].depthStencil = {1.0f, 0};

    // PRIMERA PASADA - GBUFFER

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = gBufferRenderPass;
    renderPassBeginInfo.framebuffer = gBufferFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferPipeline);

    // En esta pasada se carga la geometría de la escena
    for (const auto &mesh : sceneMeshes)
    {
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {
            VkBuffer vertexBuffers[] = {mesh.vertexMeshesData.vertexBufferList[i]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.vertexMeshesData.indexBufferList[i], 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferPipelineLayout,
                                    0, 1, gBufferDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // SEGUNDA PASADA - GENERACIÓN DE SSAO

    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassBeginInfo.renderPass = ssaoRenderPass;
    renderPassBeginInfo.framebuffer = ssaoFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipelineLayout, 0, 1, ssaoDescriptorSet, 0, nullptr);

    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // TERCERA PASADA - SSAO BLUR

    renderPassBeginInfo.renderPass = ssaoBlurRenderPass;
    renderPassBeginInfo.framebuffer = ssaoBlurFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoBlurPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoBlurPipelineLayout, 0, 1, ssaoBlurDescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // CUARTA PASADA - COMPOSICIÓN

    renderPassBeginInfo.renderPass = ssaoCompositionRenderPass;
    renderPassBeginInfo.framebuffer = ssaoCompositionFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoCompositionPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoCompositionPipelineLayout, 0, 1, ssaoCompositionDescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandManager::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                                         VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet *gBufferDescriptorSet,
                                         VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet *ssaoDescriptorSet,
                                         VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet *ssaoBlurDescriptorSet,
                                         VkRenderPass ssaoCompositionRenderPass, VkFramebuffer ssaoCompositionFramebuffer, VkPipeline ssaoCompositionPipeline, VkPipelineLayout ssaoCompositionPipelineLayout, VkDescriptorSet *ssaoCompositionDescriptorSet,
                                         VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline,
                                         VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet *shadowMappingDescriptorSet)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkViewport viewport{};
    VkRect2D scissor{};

    // PRIMER PASADA - GENERACIÓN DEL SHADOW MAP

    VkClearValue clearValuesShadowMapping[1];
    clearValuesShadowMapping[0].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = shadowMappingRenderPass;
    renderPassBeginInfo.framebuffer = shadowMappingFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValuesShadowMapping;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    // Depth bias para evitar problemas en el shadow mapping
    vkCmdSetDepthBias(
        commandBuffers[currentFrame],
        depthBiasConstant,
        0.0f,
        depthBiasSlope);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMappingPipeline);

    for (const auto &mesh : sceneMeshes)
    {
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {
            VkBuffer vertexBuffers[] = {mesh.vertexMeshesData.vertexBufferList[i]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.vertexMeshesData.indexBufferList[i], 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMappingPipelineLayout,
                                    0, 1, shadowMappingDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // SEGUNDA PASADA - GBUFFER

    // Dependiendo de la configuración, se necesitan más clear values o menos, debido al número de attachments
    if (renderConfig == RenderMode::SSAO)
    {
        std::vector<VkClearValue> clearValues(4);
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[2].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[3].depthStencil = {1.0f, 0};

        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = gBufferRenderPass;
        renderPassBeginInfo.framebuffer = gBufferFramebuffer;
        renderPassBeginInfo.renderArea.extent = extent;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        std::vector<VkClearValue> clearValues(5);
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[2].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[3].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[4].depthStencil = {1.0f, 0};

        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = gBufferRenderPass;
        renderPassBeginInfo.framebuffer = gBufferFramebuffer;
        renderPassBeginInfo.renderArea.extent = extent;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferPipeline);

    for (const auto &mesh : sceneMeshes)
    {
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {
            VkBuffer vertexBuffers[] = {mesh.vertexMeshesData.vertexBufferList[i]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.vertexMeshesData.indexBufferList[i], 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferPipelineLayout,
                                    0, 1, gBufferDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // TERCERA PASADA - GENERACIÓN DE SSAO
    std::vector<VkClearValue> clearValues(2);
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassBeginInfo.renderPass = ssaoRenderPass;
    renderPassBeginInfo.framebuffer = ssaoFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipelineLayout, 0, 1, ssaoDescriptorSet, 0, nullptr);

    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // CUARTA PASADA - SSAO BLUR

    renderPassBeginInfo.renderPass = ssaoBlurRenderPass;
    renderPassBeginInfo.framebuffer = ssaoBlurFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoBlurPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoBlurPipelineLayout, 0, 1, ssaoBlurDescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // QUINTA PASADA - COMPOSICIÓN

    renderPassBeginInfo.renderPass = ssaoCompositionRenderPass;
    renderPassBeginInfo.framebuffer = ssaoCompositionFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoCompositionPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoCompositionPipelineLayout, 0, 1, ssaoCompositionDescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandManager::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes, VkRenderPass raytracingRenderPass,
                                         VkFramebuffer raytracingFramebuffer, VkPipeline raytracingPipeline, VkPipelineLayout raytracingPipelineLayout, VkDescriptorSet *raytracingDescriptorSet)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    std::vector<VkClearValue> clearValues(2);
    VkViewport viewport;
    VkRect2D scissor;

    clearValues[0].color = {{0.0f, 0.0f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = raytracingRenderPass;
    renderPassBeginInfo.framebuffer = raytracingFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, raytracingPipeline);

    // Se carga la geometría de la escena
    for (const auto &mesh : sceneMeshes)
    {
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {
            VkBuffer vertexBuffers[] = {mesh.vertexMeshesData.vertexBufferList[i]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.vertexMeshesData.indexBufferList[i], 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, raytracingPipelineLayout,
                                    0, 1, raytracingDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandManager::recordCommandBuffer(VkExtent2D extent, uint32_t currentFrame, uint32_t imageIndex, std::vector<MeshContainer> sceneMeshes,
                                         VkRenderPass gBufferRenderPass, VkFramebuffer gBufferFramebuffer, VkPipeline gBufferPipeline, VkPipelineLayout gBufferPipelineLayout, VkDescriptorSet *gBufferDescriptorSet,
                                         VkRenderPass ssaoRenderPass, VkFramebuffer ssaoFramebuffer, VkPipeline ssaoPipeline, VkPipelineLayout ssaoPipelineLayout, VkDescriptorSet *ssaoDescriptorSet,
                                         VkRenderPass ssaoBlurRenderPass, VkFramebuffer ssaoBlurFramebuffer, VkPipeline ssaoBlurPipeline, VkPipelineLayout ssaoBlurPipelineLayout, VkDescriptorSet *ssaoBlurDescriptorSet,
                                         VkRenderPass surfelsCompositionRenderPass, VkFramebuffer surfelsCompositionFramebuffer, VkPipeline surfelsCompositionPipeline, VkPipelineLayout surfelsCompositionPipelineLayout, VkDescriptorSet *surfelsCompositionDescriptorSet,
                                         VkRenderPass shadowMappingRenderPass, VkFramebuffer shadowMappingFramebuffer, VkPipeline shadowMappingPipeline, VkPipelineLayout shadowMappingPipelineLayout, VkDescriptorSet *shadowMappingDescriptorSet,
                                         VkPipeline surfelsGenerationPipeline, VkPipelineLayout surfelsGenerationPipelineLayout, VkDescriptorSet *surfelsGenerationDescriptorSet, VkBuffer surfelStatsBuffer,
                                         VkPipeline surfelsVisualizationPipeline, VkPipelineLayout surfelsVisualizationPipelineLayout, VkDescriptorSet *surfelsVisualizationDescriptorSet,
                                         VkPipeline surfelsRadianceCalculationPipeline, VkPipelineLayout surfelsRadianceCalculationPipelineLayout, VkDescriptorSet *surfelsRadianceCalculationDescriptorSet, VkBuffer surfelBuffer,
                                         VkPipeline surfelsIndirectLightingPipeline, VkPipelineLayout surfelsIndirectLightingPipelineLayout, VkDescriptorSet *surfelsIndirectLightingDescriptorSet,
                                         VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkRenderPass surfelsVisualizationRenderPass, VkFramebuffer surfelsVisualizationFramebuffer,
                                         VkRenderPass surfelsIndirectLightingRenderPass, VkFramebuffer surfelsIndirectLightingFramebuffer)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkViewport viewport{};
    VkRect2D scissor{};

    // PRIMER PASADA - GENERACIÓN DEL SHADOW MAP

    VkClearValue clearValuesShadowMapping[1];
    clearValuesShadowMapping[0].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = shadowMappingRenderPass;
    renderPassBeginInfo.framebuffer = shadowMappingFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValuesShadowMapping;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    // Depth bias para evitar problemas en el shadow mapping
    vkCmdSetDepthBias(
        commandBuffers[currentFrame],
        depthBiasConstant,
        0.0f,
        depthBiasSlope);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMappingPipeline);

    for (const auto &mesh : sceneMeshes)
    {
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {
            VkBuffer vertexBuffers[] = {mesh.vertexMeshesData.vertexBufferList[i]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.vertexMeshesData.indexBufferList[i], 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMappingPipelineLayout,
                                    0, 1, shadowMappingDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // SEGUNDA PASADA - GBUFFER

    // Dependiendo de la configuración, se necesitan más clear values o menos, debido al número de attachments

    std::vector<VkClearValue> clearValues(5);
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[2].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[3].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[4].depthStencil = {1.0f, 0};

    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = gBufferRenderPass;
    renderPassBeginInfo.framebuffer = gBufferFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferPipeline);

    for (const auto &mesh : sceneMeshes)
    {
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {
            VkBuffer vertexBuffers[] = {mesh.vertexMeshesData.vertexBufferList[i]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.vertexMeshesData.indexBufferList[i], 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferPipelineLayout,
                                    0, 1, gBufferDescriptorSet, 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // -------------------------------------------------------------------------------------------------------------------------------------- //
    // PASADA DE CÓMPUTO 1 - GENERACIÓN DE SURFELS

    PushConstants windowSize{(float)extent.width, (float)extent.height};

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, surfelsGenerationPipeline);
    vkCmdPushConstants(
        commandBuffers[currentFrame],
        surfelsGenerationPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT |
            VK_SHADER_STAGE_GEOMETRY_BIT |
            VK_SHADER_STAGE_FRAGMENT_BIT |
            VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(PushConstants),
        &windowSize);
    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, surfelsGenerationPipelineLayout, 0, 1, surfelsGenerationDescriptorSet, 0, nullptr);

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    VkBufferMemoryBarrier bufferbarrierdesc = {};
    bufferbarrierdesc.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferbarrierdesc.pNext = nullptr;
    bufferbarrierdesc.buffer = surfelStatsBuffer;
    bufferbarrierdesc.size = sizeof(unsigned int) * 8;
    bufferbarrierdesc.offset = 0;
    bufferbarrierdesc.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

    bufferbarrierdesc.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    bufferbarrierdesc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferbarrierdesc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(
        commandBuffers[currentFrame],
        srcStage,
        dstStage,
        0,
        0, nullptr,
        1, &bufferbarrierdesc,
        0, nullptr);

    vkCmdDispatch(commandBuffers[currentFrame], ((uint32_t)extent.width + 15) / 16, ((uint32_t)extent.height + 15) / 16, 1);

    VkMemoryBarrier memorybarrierdesc = {};
    memorybarrierdesc.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memorybarrierdesc.pNext = nullptr;
    memorybarrierdesc.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    memorybarrierdesc.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffers[currentFrame],
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        1, &memorybarrierdesc,
        0, nullptr,
        0, nullptr);

    // TERCERA PASADA - VISUALIZACIÓN DE LA COBERTURA OBTENIDA POR LA GENERACIÓN DE SURFELS
    // Una vez terminada la pasada de cómputo para generar los surfels, se mapea el buffer a otro con la información indispensable para poder
    // visualizar los surfels proyectados

    if (renderConfig == RenderMode::SURFELS_VISUALIZATION)
    {
        SurfelsBufferManager::mapSurfelsVisualizationData(device, physicalDevice, commandPool, graphicsQueue, surfelBuffer, surfelsVBO, surfelsVBOAlloc, false);

        VkClearValue clearValuesSurfels[2];
        clearValuesSurfels[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValuesSurfels[1].depthStencil = {1.0f, 0};

        renderPassBeginInfo.renderPass = surfelsVisualizationRenderPass;
        renderPassBeginInfo.framebuffer = surfelsVisualizationFramebuffer;
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = extent;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValuesSurfels;

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = float(extent.width);
        viewport.height = float(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

        scissor.offset = {0, 0};
        scissor.extent = extent;
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

        vkCmdBindPipeline(
            commandBuffers[currentFrame],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            surfelsVisualizationPipeline);

        vkCmdPushConstants(
            commandBuffers[currentFrame],
            surfelsVisualizationPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT |
                VK_SHADER_STAGE_GEOMETRY_BIT |
                VK_SHADER_STAGE_FRAGMENT_BIT |
                VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            sizeof(PushConstants),
            &windowSize);

        vkCmdBindDescriptorSets(
            commandBuffers[currentFrame],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            surfelsVisualizationPipelineLayout,
            0, 1,
            surfelsVisualizationDescriptorSet,
            0, nullptr);

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(
            commandBuffers[currentFrame],
            0, 1,
            &surfelsVBO,
            offsets);

        vkCmdDraw(
            commandBuffers[currentFrame],
            SURFEL_CAPACITY,
            1,
            0,
            0);

        vkCmdEndRenderPass(commandBuffers[currentFrame]);
    }

    // PASADA DE CÓMPUTO 2 - CÁLCULO DE LA RADIANCIA ALMACENADA POR CADA SURFEL

    if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        uint32_t groupCount = (SURFEL_CAPACITY + 64 - 1) / 64;

        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, surfelsRadianceCalculationPipeline);
        vkCmdPushConstants(
            commandBuffers[currentFrame],
            surfelsRadianceCalculationPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT |
                VK_SHADER_STAGE_GEOMETRY_BIT |
                VK_SHADER_STAGE_FRAGMENT_BIT |
                VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            sizeof(PushConstants),
            &windowSize);
        vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, surfelsRadianceCalculationPipelineLayout, 0, 1, surfelsRadianceCalculationDescriptorSet, 0, nullptr);
        vkCmdDispatch(commandBuffers[currentFrame], groupCount, 1, 1);

        // Barrera para asegurar que se termina de escribir la radiancia en todos los surfels del buffer
        VkBufferMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = surfelBuffer;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(
            commandBuffers[currentFrame],
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0, nullptr,
            1, &barrier,
            0, nullptr);

        if (renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
        {
            // CUARTA PASADA - VISUALIZACIÓN DE LA RADIANCIA DE LOS SURFELS
            SurfelsBufferManager::mapSurfelsVisualizationData(device, physicalDevice, commandPool, graphicsQueue, surfelBuffer, surfelsVBO, surfelsVBOAlloc, true);

            VkClearValue clearValuesSurfels[2];
            clearValuesSurfels[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValuesSurfels[1].depthStencil = {1.0f, 0};

            renderPassBeginInfo.renderPass = surfelsVisualizationRenderPass;
            renderPassBeginInfo.framebuffer = surfelsVisualizationFramebuffer;
            renderPassBeginInfo.renderArea.offset = {0, 0};
            renderPassBeginInfo.renderArea.extent = extent;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValuesSurfels;

            vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = float(extent.width);
            viewport.height = float(extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

            scissor.offset = {0, 0};
            scissor.extent = extent;
            vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

            vkCmdBindPipeline(
                commandBuffers[currentFrame],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                surfelsVisualizationPipeline);

            vkCmdPushConstants(
                commandBuffers[currentFrame],
                surfelsVisualizationPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT |
                    VK_SHADER_STAGE_GEOMETRY_BIT |
                    VK_SHADER_STAGE_FRAGMENT_BIT |
                    VK_SHADER_STAGE_COMPUTE_BIT,
                0,
                sizeof(PushConstants),
                &windowSize);

            vkCmdBindDescriptorSets(
                commandBuffers[currentFrame],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                surfelsVisualizationPipelineLayout,
                0, 1,
                surfelsVisualizationDescriptorSet,
                0, nullptr);

            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(
                commandBuffers[currentFrame],
                0, 1,
                &surfelsVBO,
                offsets);

            vkCmdDraw(
                commandBuffers[currentFrame],
                SURFEL_CAPACITY,
                1,
                0,
                0);

            vkCmdEndRenderPass(commandBuffers[currentFrame]);
        }
        else
        {
            // CUARTA PASADA - CÁLCULO DE LA ILUMINACIÓN DIFUSA INDIRECTA

            clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};

            renderPassBeginInfo.renderPass = surfelsIndirectLightingRenderPass;
            renderPassBeginInfo.framebuffer = surfelsIndirectLightingFramebuffer;
            renderPassBeginInfo.renderArea.extent = extent;
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(extent.width);
            viewport.height = static_cast<float>(extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

            scissor.offset = {0, 0};
            scissor.extent = extent;
            vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

            vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, surfelsIndirectLightingPipeline);

            vkCmdPushConstants(
                commandBuffers[currentFrame],
                surfelsIndirectLightingPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT |
                    VK_SHADER_STAGE_GEOMETRY_BIT |
                    VK_SHADER_STAGE_FRAGMENT_BIT |
                    VK_SHADER_STAGE_COMPUTE_BIT,
                0,
                sizeof(PushConstants),
                &windowSize);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, surfelsIndirectLightingPipelineLayout, 0, 1, surfelsIndirectLightingDescriptorSet, 0, nullptr);

            vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[currentFrame]);
        }
    }

    // -----------------------------------------------------------------------------------------

    // QUINTA PASADA - GENERACIÓN DE SSAO
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassBeginInfo.renderPass = ssaoRenderPass;
    renderPassBeginInfo.framebuffer = ssaoFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipelineLayout, 0, 1, ssaoDescriptorSet, 0, nullptr);

    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // SEXTA PASADA - SSAO BLUR

    renderPassBeginInfo.renderPass = ssaoBlurRenderPass;
    renderPassBeginInfo.framebuffer = ssaoBlurFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoBlurPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoBlurPipelineLayout, 0, 1, ssaoBlurDescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    // SÉPTIMA PASADA - COMPOSICIÓN

    renderPassBeginInfo.renderPass = surfelsCompositionRenderPass;
    renderPassBeginInfo.framebuffer = surfelsCompositionFramebuffer;
    renderPassBeginInfo.renderArea.extent = extent;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, surfelsCompositionPipeline);

    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, surfelsCompositionPipelineLayout, 0, 1, surfelsCompositionDescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandManager::cleanup(VkDevice device)
{
    vkDestroyCommandPool(device, commandPool, nullptr);
    cleanupAuxiliarBuffers(device);
}

void CommandManager::cleanupAuxiliarBuffers(VkDevice)
{
    if (surfelsVBO != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(BufferCreator::allocator, surfelsVBO, surfelsVBOAlloc);
    }
}

VkCommandPool CommandManager::getCommandPool() const { return this->commandPool; }
VkCommandBuffer *CommandManager::getCommandBuffer(uint32_t id) { return &this->commandBuffers[id]; }
