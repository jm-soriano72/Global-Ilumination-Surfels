#pragma once

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
#include <array>
#include <string>
#include <stdexcept>

// ESTRUCTURAS //

// Estructura para almacenar los vértices de forma que se puedan subir en un buffer
struct alignas(16) Vertex_Buffer
{
    alignas(16) glm::vec3 pos;
    alignas(4) int idMaterial;

    alignas(8) glm::vec2 texCoord;
    alignas(8) glm::vec2 _pad1;

    alignas(16) glm::vec3 normal;
    alignas(4) float _pad2;
};

// Estructura para almacenar las coordenadas de posición y de color de los vértices, para pasárselas al shader
struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;
    int idMaterial;

    // Función que indica cómo se van a tratar los datos de los vértices empaquetados
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;                             // Al estar toda la información empaquetada en un array
        bindingDescription.stride = sizeof(Vertex);                 // Número de bytes entre un vértice y otro
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Se mueve al siguiente dato después de cada vértice

        return bindingDescription;
    }
    // Función que expresa cómo se van a tratar los atributos de los vértices
    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions()
    {
        // Se van a necesitar dos estructuras, ya que, se tienen atributos de posición y de color
        std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

        // Posición
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;                        // Location con la que se accede desde los shaders
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Tipo de dato (RGB porque tiene tres coordenadas)
        attributeDescriptions[0].offset = offsetof(Vertex, pos);      // Offset para acceder a los datos

        // Color
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1; // Location con la que se accede desde los shaders
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        // Textura
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        // Normal
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3; // Location con la que se accede desde los shaders
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, normal);

        // Coordenada de material
        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4; // Location con la que se accede desde los shaders
        attributeDescriptions[4].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[4].offset = offsetof(Vertex, idMaterial);

        return attributeDescriptions;
    }
};

class MeshLoader
{

public:
    static const int MULTI_OBJECT_LIMIT_INDEX = 5;
    static void loadModel(const std::string &filePath, std::vector<std::vector<Vertex>> *vertices, std::vector<std::vector<uint16_t>> *indices, uint32_t *materialIndex);

private:
    static void processMeshes(const aiScene *scene, std::vector<std::vector<Vertex>> *vertices, std::vector<std::vector<uint16_t>> *indices, uint32_t *materialIndex);
};