#pragma once

#include "Buffers/Tools/BufferCreator.h"
#include "Buffers/Tools/CommandBufferManager.h"
#include "MeshLoader.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <array>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

class VertexBuffer {

public:

	// ATRIBUTOS //
	// Vector de buffers para almacenar los vértices
	std::vector<VkBuffer> vertexBufferList;
	std::vector<VkDeviceMemory> vertexBufferMemoryList;
	// Vector de buffers para almacenar los índices
	std::vector<VkBuffer> indexBufferList;
	std::vector<VkDeviceMemory> indexBufferMemoryList;

	// Vector que almacena la lista de vértices de los objetos de cada fichero
	std::vector<std::vector<Vertex>> vertices;
	// Array que representa el conjunto de todos los índices de los vértices con los que se pintará la geometría
	std::vector<std::vector<uint16_t>> indices;

	// FUNCIONES //
	// Función para realizar la creación de los buffer de vértices y de índices
	void createBufferData(VkDevice device, VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& filePath, uint32_t* materialIndex);
	// Función para copiar de un buffer a otro
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
		VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
	// Funciones para abstraer la lógica de creación del Vertex Buffer
	void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool, VkQueue graphicsQueue, int index);
	// Función para crear el buffer de indexación de los vértices
	// Se sigue el mismo procedimiento de lectura en CPU, copia a GPU...
	void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool, VkQueue graphicsQueue, int index);
	// Destrucción de recursos
	void cleanup(VkDevice device);
	
};