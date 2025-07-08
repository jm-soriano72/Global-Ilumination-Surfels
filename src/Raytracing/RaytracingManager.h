#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Scene/Models/MeshContainer.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

struct AccelerationStructure
{
	VkAccelerationStructureKHR handle;
	uint64_t deviceAddress = 0;
	VkDeviceMemory memory;
	VkBuffer buffer;
};

struct ScratchBuffer
{
	uint64_t deviceAddress = 0;
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct SceneOrganizationStructure
{
	std::vector<int> blasIndexOffsets;
	std::vector<glm::vec3> scenePrimitivesIndexes;

	std::vector<int> blasVertexOffsets;
	std::vector<Vertex_Buffer> sceneVertex;
};

class RaytracingManager
{
private:
	// Estructuras de aceleración
	std::vector<AccelerationStructure> bottomLevelAccelerationStructures;
	AccelerationStructure topLevelAccelerationStructure;

	// Organización de la escena para obtener información con las colisiones
	SceneOrganizationStructure sceneStructure;

	// Propiedades y extensiones necesarias
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

	// Punteros a funciones necesarias para raytracing
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

	uint64_t getBufferDeviceAddress(VkDevice device, VkBuffer buffer);
	void createAccelerationStructure(VkDevice device, VkPhysicalDevice physicalDevice, AccelerationStructure &accelerationStructure, VkAccelerationStructureTypeKHR type,
									 VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
	ScratchBuffer createScratchBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize deviceSize);
	void deleteScratchBuffer(VkDevice device, ScratchBuffer &scratchBuffer);

public:
	void enableFeatures(VkDevice device, VkPhysicalDevice physicalDevice);
	void createBottomLevelAccelerationStructures(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
												VkQueue graphicsQueue, std::vector<MeshContainer> sceneMeshes);
	void createTopLevelAccelerationStructure(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	AccelerationStructure getTLAS();
	SceneOrganizationStructure getSceneStructure();

	void cleanup(VkDevice device);
};