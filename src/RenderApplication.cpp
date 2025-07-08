#include "RenderApplication.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Config.h"
#include "Initializers/VulkanInitializer.h"
#include "Pipelines/PipelineManager.h"
#include "Pipelines/GeometryPipeline.h"
#include "Pipelines/ShadowMappingPipeline.h"
#include "Pipelines/GBufferPipeline.h"
#include "Pipelines/RaytracingPipeline.h"
#include "Pipelines/SurfelsGenerationPipeline.h"
#include "Pipelines/SurfelsVisualizationPipeline.h"
#include "Pipelines/SurfelsRadianceCalculationPipeline.h"
#include "Pipelines/IndirectDiffuseShadingPipeline.h"
#include "Pipelines/SurfelsCompositionPipeline.h"
#include "Scene/SceneManager.h"
#include "Images/ImageCreator.h"
#include "Descriptors/DescriptorsManager.h"
#include "Buffers/UniformBuffersManager.h"
#include "Raytracing/RaytracingManager.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>

RenderApplication::RenderApplication() {}

void RenderApplication::run()
{
    initVulkan();
    mainLoop();
    cleanup();
}

void RenderApplication::initVulkan()
{
    /// ---------------------------- 1 -------------------------------------
    // Se inicializan los objetos esenciales para configurar Vulkan
    vulkanInitializer.prepareVulkan();
    // Si se va a utilizar raytracing, se habilitan las extensiones
    if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        raytracingManager.enableFeatures(vulkanInitializer.getVkDevice(), vulkanInitializer.getVkPhysicalDevice());
    }

    /// ---------------------------- 2 -------------------------------------
    // Se crean las Render Pass
    renderPassesManager.createRenderPasses(vulkanInitializer.getVkDevice(), vulkanInitializer.getVkPhysicalDevice(), vulkanInitializer.getSCImageFormat());
    // Se crean los framebuffer asociados a cada pasada
    renderPassesManager.createFramebuffers(vulkanInitializer.getNumImageViews(), vulkanInitializer.getSwapChainManager(), vulkanInitializer.getVkDevice(), vulkanInitializer.getVkPhysicalDevice(),
                                           vulkanInitializer.getCommandPool(), vulkanInitializer.getVkGraphicsQueue(), vulkanInitializer.getSwapChainExtent());

    /// ---------------------------- 3 -------------------------------------
    // Creación de la escena, carga de modelos, preparación de texturas e iluminación
    sceneManager.loadSceneAssets(vulkanInitializer.getVkDevice(), vulkanInitializer.getVkPhysicalDevice(), vulkanInitializer.getCommandPool(), vulkanInitializer.getVkGraphicsQueue());

    // Utilizando la información de la geometría cargada, se crean las estructuras de aceleración para raytracing
    if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        raytracingManager.createBottomLevelAccelerationStructures(vulkanInitializer.getVkDevice(), vulkanInitializer.getVkPhysicalDevice(), vulkanInitializer.getCommandPool(), vulkanInitializer.getVkGraphicsQueue(),
                                                                  sceneManager.sceneMeshes);
        raytracingManager.createTopLevelAccelerationStructure(vulkanInitializer.getVkDevice(), vulkanInitializer.getVkPhysicalDevice(), vulkanInitializer.getCommandPool(), vulkanInitializer.getVkGraphicsQueue());
    }

    /// ---------------------------- 4 -------------------------------------
    // Creación de los buffers de variables uniformes, propios de cada pasada
    uniformBuffersManager.createUniformBuffers(vulkanInitializer.getVkDevice(), vulkanInitializer.getVkPhysicalDevice(), vulkanInitializer.getFramesInFlight(), sceneManager.sceneLights.mainLight,
                                               vulkanInitializer.getSwapChainExtent().width, vulkanInitializer.getSwapChainExtent().height, vulkanInitializer.getCamera(), sceneManager.sceneLights,
                                               vulkanInitializer.getCommandPool(), vulkanInitializer.getVkGraphicsQueue(), sceneManager.sceneMeshes);

    /// ---------------------------- 5 -------------------------------------
    // Se crean los descriptores asociados a cada pasada de renderizado
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(), vulkanInitializer.getFramesInFlight(),
                                             sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(), sceneManager.materialsManager.getSpecularImages(),
                                             renderPassesManager.getDepthImageView(), renderPassesManager.getDepthSampler(), uniformBuffersManager.getGeometryMVPBuffers(),
                                             uniformBuffersManager.getGeometryLightsBuffers(), uniformBuffersManager.getGeometryMainLightBuffers(), uniformBuffersManager.getShadowMappingBuffers());
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), vulkanInitializer.getFramesInFlight(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(),
                                             sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(), sceneManager.materialsManager.getSpecularImages(),
                                             uniformBuffersManager.getGBuffers(), uniformBuffersManager.getSSAOProjectionBuffers(), uniformBuffersManager.getSSAOParamsBuffers(),
                                             renderPassesManager.getGBufferPositionImageView(), renderPassesManager.getGBufferNormalImageView(), renderPassesManager.getGBufferAlbedoImageView(),
                                             renderPassesManager.getSSAOColorImageView(), renderPassesManager.getSSAOBlurColorImageView(), renderPassesManager.getNoiseTexture());
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), vulkanInitializer.getFramesInFlight(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(),
                                             sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(), sceneManager.materialsManager.getSpecularImages(),
                                             uniformBuffersManager.getGBuffers(), uniformBuffersManager.getSSAOProjectionBuffers(), uniformBuffersManager.getSSAOParamsBuffers(),
                                             renderPassesManager.getGBufferPositionImageView(), renderPassesManager.getGBufferNormalImageView(), renderPassesManager.getGBufferAlbedoImageView(),
                                             renderPassesManager.getSSAOColorImageView(), renderPassesManager.getSSAOBlurColorImageView(), renderPassesManager.getNoiseTexture(),
                                             uniformBuffersManager.getShadowMappingBuffers(), renderPassesManager.getDepthImageView(), renderPassesManager.getDepthSampler(),
                                             uniformBuffersManager.getShadowCompositionLightsBuffers(), uniformBuffersManager.getShadowCompositionMainLightBuffers(), uniformBuffersManager.getShadowCompositionMVPBuffers(),
                                             renderPassesManager.getGBufferSpecularImageView());
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(), vulkanInitializer.getFramesInFlight(), uniformBuffersManager.getRaytracingMVPBuffers(),
                                             uniformBuffersManager.getRaytracingLightsBuffers(), raytracingManager.getTLAS(), sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(),
                                             sceneManager.materialsManager.getSpecularImages());
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), vulkanInitializer.getFramesInFlight(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(),
                                             sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(), sceneManager.materialsManager.getSpecularImages(),
                                             uniformBuffersManager.getGBuffers(), uniformBuffersManager.getSSAOProjectionBuffers(), uniformBuffersManager.getSSAOParamsBuffers(),
                                             renderPassesManager.getGBufferPositionImageView(), renderPassesManager.getGBufferNormalImageView(), renderPassesManager.getGBufferAlbedoImageView(),
                                             renderPassesManager.getSSAOColorImageView(), renderPassesManager.getSSAOBlurColorImageView(), renderPassesManager.getNoiseTexture(),
                                             uniformBuffersManager.getShadowMappingBuffers(), renderPassesManager.getDepthImageView(), renderPassesManager.getDepthSampler(),
                                             uniformBuffersManager.getShadowCompositionLightsBuffers(), uniformBuffersManager.getShadowCompositionMainLightBuffers(), uniformBuffersManager.getShadowCompositionMVPBuffers(),
                                             renderPassesManager.getGBufferSpecularImageView(), uniformBuffersManager.getSurfelBuffer(), uniformBuffersManager.getSurfelStatsBuffer(),
                                             uniformBuffersManager.getSurfelGridBuffer(), uniformBuffersManager.getSurfelCellBuffer(), uniformBuffersManager.getCameraSurfelBuffer(),
                                             raytracingManager.getTLAS(), uniformBuffersManager.getIndexBufferList(), uniformBuffersManager.getVertexBufferList(),
                                             uniformBuffersManager.getIndexBufferSizeList(), uniformBuffersManager.getVertexBufferSizeList(), uniformBuffersManager.getRaysNoiseImage(),
                                             renderPassesManager.getIndirectDiffuseImageView(), uniformBuffersManager.getBlueNoiseImage(), renderPassesManager.getSurfelsColorImageView());
    }

    /// ---------------------------- 6 -------------------------------------
    // Creación de los pipelines asociados a cada pasada
    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        pipelineManager.createPipelines(vulkanInitializer.getVkDevice(), vulkanInitializer.getSwapChainExtent(), descriptorsManager.getGeometryDescriptorSetLayout(), renderPassesManager.getGeometryRenderPass(),
                                        descriptorsManager.getShadowMappingDescriptorSetLayout(), renderPassesManager.getShadowMappingRenderPass());
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        pipelineManager.createPipelines(vulkanInitializer.getVkDevice(), vulkanInitializer.getSwapChainExtent(), descriptorsManager.getGBufferDescriptorSetLayout(), renderPassesManager.getGBufferRenderPass(),
                                        descriptorsManager.getSSAODescriptorSetLayout(), renderPassesManager.getSSAORenderPass(), descriptorsManager.getSSAOBlurDescriptorSetLayout(),
                                        renderPassesManager.getSSAOBlurRenderPass(), descriptorsManager.getSSAOCompositionDescriptorSetLayout(), renderPassesManager.getSSAOCompositionRenderPass());
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        pipelineManager.createPipelines(vulkanInitializer.getVkDevice(), vulkanInitializer.getSwapChainExtent(), descriptorsManager.getGBufferDescriptorSetLayout(), renderPassesManager.getGBufferRenderPass(),
                                        descriptorsManager.getSSAODescriptorSetLayout(), renderPassesManager.getSSAORenderPass(), descriptorsManager.getSSAOBlurDescriptorSetLayout(),
                                        renderPassesManager.getSSAOBlurRenderPass(), descriptorsManager.getShadowsSSAOCompositionDescriptorSetLayout(), renderPassesManager.getSSAOCompositionRenderPass(),
                                        descriptorsManager.getShadowMappingDescriptorSetLayout(), renderPassesManager.getShadowMappingRenderPass());
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        pipelineManager.createPipelines(vulkanInitializer.getVkDevice(), vulkanInitializer.getSwapChainExtent(), descriptorsManager.getRaytracingDescriptorSetLayout(), renderPassesManager.getRaytracingRenderPass());
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        pipelineManager.createPipelines(vulkanInitializer.getVkDevice(), vulkanInitializer.getSwapChainExtent(), descriptorsManager.getGBufferDescriptorSetLayout(), renderPassesManager.getGBufferRenderPass(),
                                        descriptorsManager.getSSAODescriptorSetLayout(), renderPassesManager.getSSAORenderPass(), descriptorsManager.getSSAOBlurDescriptorSetLayout(),
                                        renderPassesManager.getSSAOBlurRenderPass(), descriptorsManager.getSurfelsCompositionDescriptorSetLayout(), renderPassesManager.getSSAOCompositionRenderPass(),
                                        descriptorsManager.getShadowMappingDescriptorSetLayout(), renderPassesManager.getShadowMappingRenderPass(), descriptorsManager.getSurfelsGenerationDescriptorSetLayout(),
                                        renderPassesManager.getSurfelsVisualizationRenderPass(), descriptorsManager.getSurfelsVisualizationDescriptorSetLayout(), descriptorsManager.getSurfelsRadianceCalculationDescriptorSetLayout(),
                                        renderPassesManager.getIndirectDiffuseRenderPass(), descriptorsManager.getSurfelsIndirectLightingDescriptorSetLayout());
    }
}

void RenderApplication::mainLoop()
{
    // Se hace que la función se ejecute hasta que se cierre la ventana
    while (!glfwWindowShouldClose(vulkanInitializer.getWindow()))
    {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(vulkanInitializer.getVkDevice());
}

void RenderApplication::drawFrame()
{
    // Los pasos para renderizar cada frame son los siguientes:

    // 1. Esperar que termine el frame anterior
    // (Esto es problemático la primera vez que se ejecute, porque no habrá una señal previa)
    vkWaitForFences(vulkanInitializer.getVkDevice(), 1, vulkanInitializer.getFence(currentFrame), VK_TRUE, UINT64_MAX); // Esta función toma un array de fences, al indicar Vk_TRUE hay que esperar a todos (no influye porque sólo tenemos uno)
    vkResetFences(vulkanInitializer.getVkDevice(), 1, vulkanInitializer.getFence(currentFrame));
    // 2. Tomar una imagen del swap chain
    uint32_t imageIndex; // Índice de la imagen tomada, para escgoer el framebuffer asociado
    // Se referencia al dispositivo lógico y el swap chain, junto con la herramienta de sincronización
    VkResult result = vkAcquireNextImageKHR(vulkanInitializer.getVkDevice(), vulkanInitializer.getSwapChain(), UINT64_MAX, vulkanInitializer.getImageSemaphore(currentFrame), VK_NULL_HANDLE, &imageIndex);
    // Si se detecta alguna anomalía, se recrea la SwapChain
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vulkanInitializer.recreateSwapChain(renderPassesManager.getSwapChainFramebuffers(), renderPassesManager.getSwapChainDepthBufferCreator());
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    // Se resetea el fence sólo si se está presentando trabajo
    vkResetFences(vulkanInitializer.getVkDevice(), 1, vulkanInitializer.getFence(currentFrame));

    // 3. Almacenar la acción en un command buffer que dibuja la escena en dicha imagen
    // Primero se resetea el command buffer
    vkResetCommandBuffer(*vulkanInitializer.getCommandBuffer(currentFrame), 0);

    if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
    {
        vulkanInitializer.recordCommandBuffer(vulkanInitializer.getSwapChainExtent(), currentFrame, imageIndex, renderPassesManager.getShadowMappingRenderPass(),
                                              renderPassesManager.getShadowMappingFramebuffer(imageIndex), pipelineManager.getShadowMappingPipeline(),
                                              pipelineManager.getShadowMappingPipelineLayout(), descriptorsManager.getShadowMappingDescriptor(currentFrame),
                                              renderPassesManager.getGeometryRenderPass(), renderPassesManager.getGeometryFramebuffer(imageIndex), pipelineManager.getGeometryPipeline(),
                                              pipelineManager.getGeometryPipelineLayout(), descriptorsManager.getGeometryDescriptor(currentFrame), sceneManager.sceneMeshes,
                                              *vulkanInitializer.getCamera());
    }
    else if (renderConfig == RenderMode::SSAO)
    {
        vulkanInitializer.recordCommandBuffer(vulkanInitializer.getSwapChainExtent(), currentFrame, imageIndex, sceneManager.sceneMeshes,
                                              renderPassesManager.getGBufferRenderPass(), renderPassesManager.getGBufferFramebuffer(imageIndex), pipelineManager.getGBufferPipeline(),
                                              pipelineManager.getGBufferPipelineLayout(), descriptorsManager.getGBufferDescriptor(currentFrame),
                                              renderPassesManager.getSSAORenderPass(), renderPassesManager.getSSAOFramebuffer(imageIndex), pipelineManager.getSSAOPipeline(),
                                              pipelineManager.getSSAOPipelineLayout(), descriptorsManager.getSSAODescriptor(currentFrame),
                                              renderPassesManager.getSSAOBlurRenderPass(), renderPassesManager.getSSAOBlurFramebuffer(imageIndex), pipelineManager.getSSAOBlurPipeline(),
                                              pipelineManager.getSSAOBlurPipelineLayout(), descriptorsManager.getSSAOBlurDescriptor(currentFrame),
                                              renderPassesManager.getSSAOCompositionRenderPass(), renderPassesManager.getSSAOCompositionFramebuffer(imageIndex), pipelineManager.getSSAOCompositionPipeline(),
                                              pipelineManager.getSSAOCompositionPipelineLayout(), descriptorsManager.getSSAOCompositionDescriptor(currentFrame));
    }
    else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
    {
        vulkanInitializer.recordCommandBuffer(vulkanInitializer.getSwapChainExtent(), currentFrame, imageIndex, sceneManager.sceneMeshes,
                                              renderPassesManager.getGBufferRenderPass(), renderPassesManager.getGBufferFramebuffer(imageIndex), pipelineManager.getGBufferPipeline(),
                                              pipelineManager.getGBufferPipelineLayout(), descriptorsManager.getGBufferDescriptor(currentFrame),
                                              renderPassesManager.getSSAORenderPass(), renderPassesManager.getSSAOFramebuffer(imageIndex), pipelineManager.getSSAOPipeline(),
                                              pipelineManager.getSSAOPipelineLayout(), descriptorsManager.getSSAODescriptor(currentFrame),
                                              renderPassesManager.getSSAOBlurRenderPass(), renderPassesManager.getSSAOBlurFramebuffer(imageIndex), pipelineManager.getSSAOBlurPipeline(),
                                              pipelineManager.getSSAOBlurPipelineLayout(), descriptorsManager.getSSAOBlurDescriptor(currentFrame),
                                              renderPassesManager.getSSAOCompositionRenderPass(), renderPassesManager.getSSAOCompositionFramebuffer(imageIndex), pipelineManager.getSSAOCompositionPipeline(),
                                              pipelineManager.getSSAOCompositionPipelineLayout(), descriptorsManager.getShadowsSSAOCompositionDescriptor(currentFrame),
                                              renderPassesManager.getShadowMappingRenderPass(), renderPassesManager.getShadowMappingFramebuffer(imageIndex), pipelineManager.getShadowMappingPipeline(),
                                              pipelineManager.getShadowMappingPipelineLayout(), descriptorsManager.getShadowMappingDescriptor(currentFrame));
    }
    else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
    {
        vulkanInitializer.recordCommandBuffer(vulkanInitializer.getSwapChainExtent(), currentFrame, imageIndex, sceneManager.sceneMeshes,
                                              renderPassesManager.getRaytracingRenderPass(), renderPassesManager.getRaytracingFramebuffer(imageIndex), pipelineManager.getRaytracingPipeline(),
                                              pipelineManager.getRaytracingPipelineLayout(), descriptorsManager.getRaytracingDescriptor(currentFrame));
    }
    else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        vulkanInitializer.cleanSurfelsAuxiliarBuffer();
        vulkanInitializer.recordCommandBuffer(vulkanInitializer.getSwapChainExtent(), currentFrame, imageIndex, sceneManager.sceneMeshes,
                                              renderPassesManager.getGBufferRenderPass(), renderPassesManager.getGBufferFramebuffer(imageIndex), pipelineManager.getGBufferPipeline(),
                                              pipelineManager.getGBufferPipelineLayout(), descriptorsManager.getGBufferDescriptor(currentFrame),
                                              renderPassesManager.getSSAORenderPass(), renderPassesManager.getSSAOFramebuffer(imageIndex), pipelineManager.getSSAOPipeline(),
                                              pipelineManager.getSSAOPipelineLayout(), descriptorsManager.getSSAODescriptor(currentFrame),
                                              renderPassesManager.getSSAOBlurRenderPass(), renderPassesManager.getSSAOBlurFramebuffer(imageIndex), pipelineManager.getSSAOBlurPipeline(),
                                              pipelineManager.getSSAOBlurPipelineLayout(), descriptorsManager.getSSAOBlurDescriptor(currentFrame),
                                              renderPassesManager.getSSAOCompositionRenderPass(), renderPassesManager.getSSAOCompositionFramebuffer(imageIndex), pipelineManager.getSurfelsCompositionPipeline(),
                                              pipelineManager.getSurfelsCompositionPipelineLayout(), descriptorsManager.getSurfelsCompositionDescriptor(currentFrame),
                                              renderPassesManager.getShadowMappingRenderPass(), renderPassesManager.getShadowMappingFramebuffer(imageIndex), pipelineManager.getShadowMappingPipeline(),
                                              pipelineManager.getShadowMappingPipelineLayout(), descriptorsManager.getShadowMappingDescriptor(currentFrame),
                                              pipelineManager.getSurfelsGenerationPipeline(), pipelineManager.getSurfelsGenerationPipelineLayout(), descriptorsManager.getSurfelsGenerationDescriptor(currentFrame),
                                              uniformBuffersManager.getSurfelStatsBuffer(), pipelineManager.getSurfelsVisualizationPipeline(), pipelineManager.getSurfelsVisualizationPipelineLayout(),
                                              descriptorsManager.getSurfelsVisualizationDescriptor(currentFrame), pipelineManager.getSurfelsRadianceCalculationPipeline(),
                                              pipelineManager.getSurfelsRadianceCalculationPipelineLayout(), descriptorsManager.getSurfelsRadianceCalculationDescriptor(currentFrame), uniformBuffersManager.getSurfelBuffer(),
                                              pipelineManager.getSurfelsIndirectLightingPipeline(), pipelineManager.getSurfelsIndirectLightingPipelineLayout(), descriptorsManager.getSurfelsIndirectLightingDescriptor(currentFrame),
                                              renderPassesManager.getSurfelsVisualizationRenderPass(), renderPassesManager.getSurfelsVisualizationFramebuffer(imageIndex), renderPassesManager.getIndirectDiffuseRenderPass(),
                                              renderPassesManager.getIndirectDiffuseFramebuffer(imageIndex));
    }

    // 4. Se actualiza el buffer de variables uniformes
    uniformBuffersManager.updateUniformBuffers(currentFrame, sceneManager.sceneLights.mainLight, vulkanInitializer.getSwapChainExtent().width, vulkanInitializer.getSwapChainExtent().height,
                                               vulkanInitializer.getCamera(), sceneManager.sceneLights);

    // 5. Presentar dicho command buffer a las queues de ejecución
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vulkanInitializer.getImageSemaphore(currentFrame)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;           // Número de semáforos hay que esperar antes de ejecutar
    submitInfo.pWaitSemaphores = waitSemaphores; // Lista de semáforos que hay que esperar
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = vulkanInitializer.getCommandBuffer(currentFrame); // Referencia al command buffer
    VkSemaphore signalSemaphores[] = {vulkanInitializer.getRenderSemaphore(currentFrame)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores; // Lista de semáforos que activar cuando se finaliza la ejecución
    // Se pasa a la queue
    // El fence hace que la CPU espere a que se termine esta ejecución antes de comenzar a calcular el siguiente frame
    if (vkQueueSubmit(vulkanInitializer.getVkGraphicsQueue(), 1, &submitInfo, *vulkanInitializer.getFence(currentFrame)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // 6. Mostrar la imagen del swap chain
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // Se debe esperar a que termine la ejecución del command buffer
    // Imágenes de la swap chain a presentar
    VkSwapchainKHR swapChains[] = {vulkanInitializer.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    // Función de presentación
    result = vkQueuePresentKHR(vulkanInitializer.getVkPresentQueue(), &presentInfo);
    // Se comprueba si la SwapChain no es óptima
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkanInitializer.isFramebufferResized())
    {
        vkDeviceWaitIdle(vulkanInitializer.getVkDevice());

        vulkanInitializer.resetFramebufferResized();
        vulkanInitializer.recreateSwapChain(renderPassesManager.getSwapChainFramebuffers(), renderPassesManager.getSwapChainDepthBufferCreator());
        renderPassesManager.cleanupFramebuffers(vulkanInitializer.getVkDevice());
        renderPassesManager.createFramebuffers(vulkanInitializer.getNumImageViews(), vulkanInitializer.getSwapChainManager(), vulkanInitializer.getVkDevice(), vulkanInitializer.getVkPhysicalDevice(),
                                               vulkanInitializer.getCommandPool(), vulkanInitializer.getVkGraphicsQueue(), vulkanInitializer.getSwapChainExtent());
        descriptorsManager.cleanupDescriptors(vulkanInitializer.getVkDevice());
        if (renderConfig == RenderMode::SHADOW_MAPPING || renderConfig == RenderMode::SHADOW_MAPPING_PCF)
        {
            descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(), vulkanInitializer.getFramesInFlight(),
                                                 sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(), sceneManager.materialsManager.getSpecularImages(),
                                                 renderPassesManager.getDepthImageView(), renderPassesManager.getDepthSampler(), uniformBuffersManager.getGeometryMVPBuffers(),
                                                 uniformBuffersManager.getGeometryLightsBuffers(), uniformBuffersManager.getGeometryMainLightBuffers(), uniformBuffersManager.getShadowMappingBuffers());
        }
        else if (renderConfig == RenderMode::SSAO)
        {
            descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), vulkanInitializer.getFramesInFlight(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(),
                                                 sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(), sceneManager.materialsManager.getSpecularImages(),
                                                 uniformBuffersManager.getGBuffers(), uniformBuffersManager.getSSAOProjectionBuffers(), uniformBuffersManager.getSSAOParamsBuffers(),
                                                 renderPassesManager.getGBufferPositionImageView(), renderPassesManager.getGBufferNormalImageView(), renderPassesManager.getGBufferAlbedoImageView(),
                                                 renderPassesManager.getSSAOColorImageView(), renderPassesManager.getSSAOBlurColorImageView(), renderPassesManager.getNoiseTexture());
        }
        else if (renderConfig == RenderMode::SSAO_SHADOW_MAPPING_PCF)
        {
            descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), vulkanInitializer.getFramesInFlight(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(),
                                                 sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(), sceneManager.materialsManager.getSpecularImages(),
                                                 uniformBuffersManager.getGBuffers(), uniformBuffersManager.getSSAOProjectionBuffers(), uniformBuffersManager.getSSAOParamsBuffers(),
                                                 renderPassesManager.getGBufferPositionImageView(), renderPassesManager.getGBufferNormalImageView(), renderPassesManager.getGBufferAlbedoImageView(),
                                                 renderPassesManager.getSSAOColorImageView(), renderPassesManager.getSSAOBlurColorImageView(), renderPassesManager.getNoiseTexture(),
                                                 uniformBuffersManager.getShadowMappingBuffers(), renderPassesManager.getDepthImageView(), renderPassesManager.getDepthSampler(),
                                                 uniformBuffersManager.getShadowCompositionLightsBuffers(), uniformBuffersManager.getShadowCompositionMainLightBuffers(), uniformBuffersManager.getShadowCompositionMVPBuffers(),
                                                 renderPassesManager.getGBufferSpecularImageView());
        }
        else if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS)
        {
            descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(), vulkanInitializer.getFramesInFlight(), uniformBuffersManager.getRaytracingMVPBuffers(),
                                                 uniformBuffersManager.getRaytracingLightsBuffers(), raytracingManager.getTLAS(), sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(),
                                                 sceneManager.materialsManager.getSpecularImages());
        }
        else if (renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
        {
            descriptorsManager.createDescriptors(vulkanInitializer.getVkDevice(), vulkanInitializer.getFramesInFlight(), NUM_TEXTURES_PER_MATERIAL, sceneManager.materialsManager.getNumImages(),
                                                 sceneManager.materialsManager.getDiffuseImages(), sceneManager.materialsManager.getAlphaImages(), sceneManager.materialsManager.getSpecularImages(),
                                                 uniformBuffersManager.getGBuffers(), uniformBuffersManager.getSSAOProjectionBuffers(), uniformBuffersManager.getSSAOParamsBuffers(),
                                                 renderPassesManager.getGBufferPositionImageView(), renderPassesManager.getGBufferNormalImageView(), renderPassesManager.getGBufferAlbedoImageView(),
                                                 renderPassesManager.getSSAOColorImageView(), renderPassesManager.getSSAOBlurColorImageView(), renderPassesManager.getNoiseTexture(),
                                                 uniformBuffersManager.getShadowMappingBuffers(), renderPassesManager.getDepthImageView(), renderPassesManager.getDepthSampler(),
                                                 uniformBuffersManager.getShadowCompositionLightsBuffers(), uniformBuffersManager.getShadowCompositionMainLightBuffers(), uniformBuffersManager.getShadowCompositionMVPBuffers(),
                                                 renderPassesManager.getGBufferSpecularImageView(), uniformBuffersManager.getSurfelBuffer(), uniformBuffersManager.getSurfelStatsBuffer(),
                                                 uniformBuffersManager.getSurfelGridBuffer(), uniformBuffersManager.getSurfelCellBuffer(), uniformBuffersManager.getCameraSurfelBuffer(),
                                                 raytracingManager.getTLAS(), uniformBuffersManager.getIndexBufferList(), uniformBuffersManager.getVertexBufferList(),
                                                 uniformBuffersManager.getIndexBufferSizeList(), uniformBuffersManager.getVertexBufferSizeList(), uniformBuffersManager.getRaysNoiseImage(),
                                                 renderPassesManager.getIndirectDiffuseImageView(), uniformBuffersManager.getBlueNoiseImage(), renderPassesManager.getSurfelsColorImageView());
        }
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
    currentFrame = (currentFrame + 1) % vulkanInitializer.getFramesInFlight();
}

void RenderApplication::cleanup()
{
    // Se limpian todos los recursos de la swap chain
    vulkanInitializer.cleanupSwapChain();
    // Se limpian las pasadas de renderizado y sus framebuffer asociados
    renderPassesManager.cleanup(vulkanInitializer.getVkDevice());
    // Se liberan los buffers uniformes
    uniformBuffersManager.cleanupUniformBuffers(vulkanInitializer.getVkDevice());
    // Se liberan los recursos de los descriptores
    descriptorsManager.cleanupDescriptors(vulkanInitializer.getVkDevice());
    // Se elimina la información de los buffer de vértices de cada modelo de la escena
    sceneManager.cleanup(vulkanInitializer.getVkDevice());
    // Se eliminan las herramientas de sincronización y el command pool
    vulkanInitializer.cleanupSynchronizacionCommandObjects();
    // Se elimina el pipeline de renderizado
    pipelineManager.cleanup(vulkanInitializer.getVkDevice());
    // Se liberan los recursos asociados a raytracing
    if (renderConfig == RenderMode::RAYTRACING_BASE_SHADOWS || renderConfig == RenderMode::SURFELS_GLOBAL_ILLUMINATION || renderConfig == RenderMode::SURFELS_VISUALIZATION || renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
    {
        raytracingManager.cleanup(vulkanInitializer.getVkDevice());
    }
    // Se limpian los objetos de Vulkan y la ventana
    vulkanInitializer.cleanup();
}