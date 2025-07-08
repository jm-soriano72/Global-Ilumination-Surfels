#pragma once

#include "VertexBuffer.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

#include <vector>

class MeshContainer {
public:

    // Información de todos los objetos dentro del fichero del modelo
    VertexBuffer vertexMeshesData;

    // Función para cargar los datos de los vértices del modelo
    void loadVertexData(VkDevice device, VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& filePath, uint32_t* materialIndex);
};