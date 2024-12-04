#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <vector>
#include <array>

#include <RenderApplication.h>
#include <vma/vk_mem_alloc.h>

class UniformBuffer {

public:

	// ESTRUCTURAS //
	// Se define la estructura del uniform buffer object que se va a transmitir al shader de vértices
	// Incluye las matrices de modelado (con las transformaciones geométricas), de vista (con las coordenadas de la cámara) y de proyección (para proyectar una imagen 3D en 2D)
	// Alignas se utiliza para que haya una correspondencia entre los espaciados de memoria con los shaders
	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	// ATRIBUTOS //
	BufferCreator bufferCreator;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	// Buffer para almacenar las variables uniformes
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	// Se almacenan varios en vectores, porque se necesita tener tantos como frames a la vez se procesen, para evitar sobreescrituras y problemas al calcular un frame y pasar al siguiente
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	uint32_t MAX_FRAMES_IN_FLIGHT;
	std::vector<VkDescriptorSet> descriptorSets; // Lista de los conjuntos de descriptores

	// Esta función crea el descriptor para definir toda la información que se le va a pasar al shader 
	void createDescriptorSetLayout(VkDevice device) {

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1; // Como solo se tiene un objeto, solo se necesita un descriptor
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Se especifica que la información sólo se va a utilizar en el shader de vértices
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		// Creación del objeto con toda la información indicada
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	// Función para crear los buffers en los que se almacenarán los atributos de las variables uniformes
	void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT) {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		this->MAX_FRAMES_IN_FLIGHT = MAX_FRAMES_IN_FLIGHT;
		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			bufferCreator.createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}
	}

	// Función para actualizar el buffer con las matrices model, view y projection
	void updateUniformBuffer(uint32_t currentImage, uint32_t width, uint32_t height) {

		// Se calcula el tiempo en segundos desde que se empezó a renderizar
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		// Se crea el UBO, definiendo las distintas matrices, en función del tiempo transcurrido
		UniformBufferObject ubo{};
		// Se define una rotación
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		// Matriz de vista
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		// Matriz de proyección
		ubo.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		// Se copia la información al buffer
		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	// Función para crear el pool de de descriptores, para poder crear un conjunto de ellos
	void createDescriptorPool(VkDevice device) {
		// Tamaño del pool
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // Se crean tantos como número de frames haya a la par
		// Creación del pool
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		// Número máximo de conjuntos
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		// Creación del pool
		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	// Función para asignar los conjuntos de descriptores
	void createDescriptorSets(VkDevice device) {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool; // Se indica el pool
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // Número de conjuntos
		allocInfo.pSetLayouts = layouts.data();
		// Se crean todos los conjuntos
		// Esta función asigna un buffer a cada conjunto
		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		// Configuración de los descriptores
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);
			// Información para poder actualizar los buffers, en cada frame
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			// Tipo de descriptor (Uniform Buffer)
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1; // Número de elementos que se quieren actualizar

			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional
			// Actualización del descriptor
			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	// Se liberan todos los recursos de la clase
	void cleanup(VkDevice device) {
		// Se eliminan los buffers
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
		// Se destruye el pool, con el que se elimina cada conjunto de descriptores
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}





};