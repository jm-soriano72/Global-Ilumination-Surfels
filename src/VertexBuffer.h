#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <vector>
#include <array>

#include <RenderApplication.h>
#include <BufferCreator.h>
#include <vma/vk_mem_alloc.h>

class VertexBuffer {

public:

	// ESTRUCTURAS //

	// Estructura para almacenar las coordenadas de posición y de color de los vértices, para pasársela al shader
	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		// Función que indica cómo se van a tratar los datos de los vértices empaquetados
		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0; // Al estar toda la información empaquetada en un array
			bindingDescription.stride = sizeof(Vertex); // Número de bytes entre un vértice y otro
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Se mueve al siguiente dato después de cada vértice

			return bindingDescription;
		}
		// Función que expresa cómo se van a tratar los atributos de los vértices
		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			// Se van a necesitar dos estructuras, ya que, se tienen atributos de posición y de color
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

				// Posición
				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0; // Location con la que se accede desde los shaders
				attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // Tipo de dato (RG porque tiene dos coordenadas)
				attributeDescriptions[0].offset = offsetof(Vertex, pos); // Offset para acceder a los datos
				
				// Color
				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1; // Location con la que se accede desde los shaders
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, color);


			return attributeDescriptions;
		}
	};

	// ATRIBUTOS //

	// Array de vértices para pasar al shader
	const std::vector<Vertex> vertices = {
		{{0.0f, 0.0f}, {0.7f, 0.7f, 1.0f}},   // Centro (Blanco)
		{{0.0f, -0.9f}, {0.0f, 0.0f, 1.0f}},  // Pico superior (Azul oscuro)
		{{0.2f, -0.2f}, {1.0f, 1.0f, 0.25f}},  // Esquina interior derecha superior (Amarillo claro)
		{{0.9f, 0.0f}, {0.0f, 0.0f, 1.0f}},   // Pico derecho (Azul oscuro)
		{{0.2f, 0.2f}, {1.0f, 1.0f, 0.25f}},   // Esquina interior derecha inferior (Amarillo claro)
		{{0.0f, 0.9f}, {0.0f, 0.0f, 1.0}},   // Pico inferior (Azul oscuro)
		{{-0.2f, 0.2f}, {1.0f, 1.0f, 0.25f}},  // Esquina interior izquierda inferior (Amarillo claro)
		{{-0.9f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // Pico izquierdo (Azul oscuro)
		{{-0.2f, -0.2f}, {1.0f, 1.0f, 0.25f}}  // Esquina interior izquierda superior (Amarillo claro)
	};

	// Array que representa los índices de los vértices con los que se pintará la geometría
	const std::vector<uint16_t> indices = {
		0, 1, 2,  // Triángulo 1: Centro, Pico superior, Valle derecho superior
		0, 2, 3,  // Triángulo 2: Centro, Valle derecho superior, Pico derecho
		0, 3, 4,  // Triángulo 3: Centro, Pico derecho, Valle derecho inferior
		0, 4, 5,  // Triángulo 4: Centro, Valle derecho inferior, Pico inferior
		0, 5, 6,  // Triángulo 5: Centro, Pico inferior, Valle izquierdo inferior
		0, 6, 7,  // Triángulo 6: Centro, Valle izquierdo inferior, Pico izquierdo
		0, 7, 8,  // Triángulo 7: Centro, Pico izquierdo, Valle izquierdo superior
		0, 8, 1   // Triángulo 8: Centro, Valle izquierdo superior, Pico superior
	};

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	// FUNCIONES //

	// Función para realizar la creación de los buffer de vértices y de índices
	void createBufferData(VkDevice device, VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool, VkQueue graphicsQueue) {

		// Creación del buffer de vértices
		createVertexBuffer(device, physicalDevice, commandPool, graphicsQueue);
		// Creación del buffer de índices
		createIndexBuffer(device, physicalDevice, commandPool, graphicsQueue);
	}
	
	// Función para crear un buffer utilizando Vulkan Memory Allocator
	void createBufferWithVMA(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
		VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation) {
		// Información para crear el buffer
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size; // Tamaño del buffer
		bufferInfo.usage = usage; // Uso del buffer
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Acceso exclusivo

		// Información para la asignación de memoria
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = memoryUsage; // Tipo de memoria, por ejemplo, VMA_MEMORY_USAGE_GPU_ONLY

		// Creación del buffer y asignación de memoria
		if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer with VMA!");
		}
	}
	

	// Función para copiar de un buffer a otro
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, 
		VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;
		// Se inicializa el buffer de comandos para poder realizar la copia después
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		// Copia
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		// Se deja de almacenar el el command buffer
		vkEndCommandBuffer(commandBuffer);
		// Se ejecuta el comando
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue); // Se espera a que termine
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer); // Se libera
	}


	// Funciones para abstraer la lógica de creación del Vertex Buffer
	void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, 
		VkCommandPool commandPool, VkQueue graphicsQueue) {
		// Creación del buffer
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		// Es un buffer temporal accesible desde la CPU
		// Se utiliza para cargar la información de los vértices, más rápido que al hacerlo directamente en la GPU
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Una vez asignada la memoria, se copian los datos de los vértices al staging buffer
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t) bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);
		// Se crea el buffer de vértices en GPU
		BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		// Se copia la información del buffer en CPU al buffer de vértices en GPU
		copyBuffer(stagingBuffer, vertexBuffer, bufferSize, device, commandPool, graphicsQueue);
		
		// Se liberan los recursos
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// Función para crear el buffer de indexación de los vértices
	// Se sigue el mismo procedimiento de lectura en CPU, copia a GPU...
	void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool, VkQueue graphicsQueue) {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize, device, commandPool, graphicsQueue);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// Destrucción de recursos
	void cleanup(VkDevice device) {
		vkDestroyBuffer(device, indexBuffer, nullptr); // Destrucción del buffer de índices
		vkFreeMemory(device, indexBufferMemory, nullptr); // Liberación de memoria
		vkDestroyBuffer(device, vertexBuffer, nullptr); // Destrucción del buffer de vértices
		vkFreeMemory(device, vertexBufferMemory, nullptr); // Liberación de memoria
	}

};