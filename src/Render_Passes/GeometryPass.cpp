#include "GeometryPass.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/SwapChainManager.h"
#include "Utils/DepthBuffer.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

GeometryPass::GeometryPass() {}

void GeometryPass::createRenderPass(VkDevice device, VkFormat format, VkPhysicalDevice physicalDevice)
{
    // Buffer de color, representado por una imagen del swap chain
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Como no se va a hacer multisampling, solo se toma 1 muestra
    // Se indica qué hacer con los datos antes de renderizar (pintar encima, limpiar, o ignorar)
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // Se indica qué hacer con los datos
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // Se ignora lo que hubiera en el layout antes de renderizar, ya que se va a limpiar
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // Como se va a mostrar la imagen por pantalla:
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Buffer de profundidad, para almacenar la profundidad tras el resultado del renderizado
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthBufferCreator.findDepthFormat(physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Una pasada de renderizado puede estar compuesta por múltiples subpasadas
    // Cada subpasada es secuencial, es decir, utiliza el contenido de los framebuffer de la pasada anterior
    // Esto puede ser útil para operaciones de post procesado por ejemplo
    // En este caso sólo se crea una subpasada
    // Hace referencia al único buffer de color
    VkAttachmentReference colorAttachmentRef{}; // Referencia al buffer del color
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // Se hace también una referencia al buffer de profundidad
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;       // El índice es referenciado desde el shader de fragmentos con layout(location = 0) out vec4 outColor
    subpass.pDepthStencilAttachment = &depthAttachmentRef; // Sólo se puede utilizar un Depth Stencil en cada subpasada

    // Dependencias entre subpasses
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;                                               // Primera y única subpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Se necesita esperar a la salida de color
    dependency.srcAccessMask = 0;
    // Se evita que comience directamente la transición de imágenes hasta que sea necesario
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // Dependencias al buffer de profundidad
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Se crea la pasada de renderizado
    // Buffers donde se escriben los resultados
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // Número de buffers
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1; // Se indica el único subpass
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency; // Dependencias

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void GeometryPass::createFramebuffers(uint32_t numImageViews, SwapChainManager swapChainManager, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                                      VkQueue graphicsQueue, VkExtent2D swapChainExtent)
{
    // Antes de crear los framebuffer, se crean los recursos de profundidad
    depthBufferCreator.createDepthResources(device, physicalDevice, commandPool, graphicsQueue, swapChainExtent);

    // Se necesita crear un framebuffer para cada una de las imágenes del swap chain
    framebuffers.resize(numImageViews);

    for (size_t i = 0; i < numImageViews; i++)
    {
        // Framebuffer que almacena el espacio para la información de la imagen de cada swap chain con su imagen de profundidad
        std::array<VkImageView, 2> attachments = {
            swapChainManager.getImageView(i),
            depthBufferCreator.depthImageView};
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;              // Se especifica el render pass
        framebufferInfo.attachmentCount = attachments.size(); // Sólo se va a utilizar el color
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

void GeometryPass::cleanup(VkDevice device)
{
    depthBufferCreator.cleanupDepthResources(device);
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (auto &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}

void GeometryPass::cleanupFramebuffers(VkDevice device) {}
