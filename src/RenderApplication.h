#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Initializers/VulkanInitializer.h"
#include "Render_Passes/RenderPassesManager.h"
#include "Pipelines/PipelineManager.h"
#include "Scene/SceneManager.h"
#include "Scene/SponzaResources.h"
#include "Images/ImageCreator.h"
#include "Buffers/UniformBuffersManager.h"
#include "Descriptors/DescriptorsManager.h"
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

class RenderApplication
{
public:
	RenderApplication();

	void run();

private:
	
	// Referencias a clases externas
	// Inicializador de los recursos básicos y controlador de Vulkan
	VulkanInitializer vulkanInitializer;
	// Creador de las render pass y los framebuffer asociados
	RenderPassesManager renderPassesManager;
	// Controlador de los recursos de la escena (modelos, texturas, iluminación...)
	SceneManager sceneManager;
	// Gestor de los buffers de variables uniformes de cada una de las pasadas
	UniformBuffersManager uniformBuffersManager;
	// Controlador de los descriptores de cada pasada
	DescriptorsManager descriptorsManager;
	// Creador de los pipelines de renderizado
	PipelineManager pipelineManager;
	// Gestor del raytracing
	RaytracingManager raytracingManager;

	uint32_t currentFrame = 0;


	void initVulkan();
	void mainLoop();
	void drawFrame();
	
	void cleanup();
};
