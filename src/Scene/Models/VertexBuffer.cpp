#include "VertexBuffer.h"

#include "Buffers/Tools/BufferCreator.h"
#include "Buffers/Tools/CommandBufferManager.h"
#include "MeshLoader.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <array>

void VertexBuffer::createBufferData(VkDevice device, VkPhysicalDevice physicalDevice,
    VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& filePath, uint32_t* materialIndex) {
    // Se cargan los modelos de la escena
    MeshLoader::loadModel(filePath, &vertices, &indices, materialIndex);
    // Se crea un buffer para cada objeto
    for(int i = 0; i < vertices.size(); i++) {
        // Creación del buffer de vértices
        createVertexBuffer(device, physicalDevice, commandPool, graphicsQueue, i);
        // Creación del buffer de índices
        createIndexBuffer(device, physicalDevice, commandPool, graphicsQueue, i);
    }
}

void VertexBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
    VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue) {
    // Se inicializa el command buffer
    VkCommandBuffer commandBuffer = CommandBufferManager::beginSingleTimeCommands(commandPool, device);
    // Se pasa la acción de copiar el buffer
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    // Se finaliza el command buffer
    CommandBufferManager::endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
}

void VertexBuffer::createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    VkCommandPool commandPool, VkQueue graphicsQueue, int index) {
    // Creación del buffer
    VkDeviceSize bufferSize = sizeof(vertices[index][0]) * vertices[index].size();
    // Es un buffer temporal accesible desde la CPU
    // Se utiliza para cargar la información de los vértices, más rápido que al hacerlo directamente en la GPU
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    // Una vez asignada la memoria, se copian los datos de los vértices al staging buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices[index].data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    // Se crea el buffer de vértices en GPU
    // Se declara el buffer para rellenarlo, al igual que la memoria
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    // Se copia la información del buffer en CPU al buffer de vértices en GPU
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize, device, commandPool, graphicsQueue);
    // Se añaden el buffer y la memoria a la lista
    vertexBufferList.push_back(vertexBuffer);
    vertexBufferMemoryList.push_back(vertexBufferMemory);
    // Se liberan los recursos
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VertexBuffer::createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    VkCommandPool commandPool, VkQueue graphicsQueue, int index) {
    VkDeviceSize bufferSize = sizeof(indices[index][0]) * indices[index].size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices[index].data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
        
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize, device, commandPool, graphicsQueue);

    // Se añaden los recursos a la lista
    indexBufferList.push_back(indexBuffer);
    indexBufferMemoryList.push_back(indexBufferMemory);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VertexBuffer::cleanup(VkDevice device) {
    for(int i = 0; i < vertexBufferList.size(); i++) {
        vkDestroyBuffer(device, indexBufferList[i], nullptr); // Destrucción del buffer de índices
        vkFreeMemory(device, indexBufferMemoryList[i], nullptr); // Liberación de memoria
        vkDestroyBuffer(device, vertexBufferList[i], nullptr); // Destrucción del buffer de vértices
        vkFreeMemory(device, vertexBufferMemoryList[i], nullptr); // Liberación de memoria
    }
}