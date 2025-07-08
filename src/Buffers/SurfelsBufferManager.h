#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Raytracing/RaytracingManager.h"
#include "Scene/SceneManager.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <chrono>
#include <vector>
#include <array>

struct SimpleVertex
{
    glm::vec3 normal;
    int idMaterial;
    glm::vec2 uv;
    glm::vec2 pad;
};

struct Surfel
{
    glm::vec3 position;
    float radius;
    glm::vec3 normal;
    int generatedRays;
    glm::vec3 color;
    float padding1;
    glm::vec3 direct_radiance;
    float padding2;
    glm::vec3 indirect_radiance;
    float padding3;
};

struct SimpleSurfel
{
    glm::vec3 position;
    float radius;
    glm::vec3 color;
    float padding0;
    glm::vec3 normal;
    float padding1;
};

struct CameraUniformBuffer
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec2 nearFarPlanes;
    glm::vec2 padding0;
    glm::vec4 frame;
    glm::vec3 cameraPosition;
    float padding1;
};

struct PushConstants
{
    float width;
    float height;
};

static const glm::uvec3 SURFEL_GRID_DIMENSIONS = glm::uvec3(256, 128, 128);                                                    // Dimensiones del mallado en el que se va a dividir la escena, para situar los surfels
static const unsigned int SURFEL_TABLE_SIZE = SURFEL_GRID_DIMENSIONS.x * SURFEL_GRID_DIMENSIONS.y * SURFEL_GRID_DIMENSIONS.z; // Tamaño del grid
static const unsigned int SURFEL_CAPACITY = 100000;
static const unsigned int SURFEL_CELL_LIMIT = 100;
static const unsigned int NUM_RAYS = 50;

class SurfelsBufferManager
{
private:
    // Buffers para almacenar los surfels creados, sus localizaciones en la escena...
    VkBuffer surfelPositionBuffer;
    VmaAllocation surfelPositionBufferAllocation;
    VkBuffer surfelBuffer;
    VmaAllocation surfelBufferAllocation;
    VkBuffer surfelStatsBuffer;
    VmaAllocation surfelStatsBufferAllocation;
    VkBuffer surfelGridBuffer;
    VmaAllocation surfelGridBufferAllocation;
    VkBuffer surfelCellBuffer;
    VmaAllocation surfelCellBufferAllocation;

    // Listas de buffers de vértices y de índices de la escena
    std::vector<VkBuffer> vertexBufferList;
    std::vector<VmaAllocation> vertexBufferAllocationList;
    std::vector<size_t> vertexBufferSizeList;
    std::vector<VkBuffer> indexBufferList;
    std::vector<VmaAllocation> indexBufferAllocationList;
    std::vector<size_t> indexBufferSizeList;

    // Buffer de variables uniformes de la cámara
    VkBuffer uniformCameraBuffer;
    VkDeviceMemory uniformCameraBufferMemory;
    void *uniformCameraBufferMapped;

    // Buffer con la información de los materiales translúcidos
    VkBuffer translucentMaterialsBuffer;
    VkDeviceMemory translucentMaterialsBufferMemory;
    void *translucentMaterialsBufferMapped;

    // Imagen con la textura del ruido para la dirección de los rayos
    ImageCreator noiseImage;
    // Imagen con el blue noisse para la generación de los surfels
    ImageCreator blueNoiseImage;

    const char *blueNoisePath = RESOURCES_PATH "textures/Blue_Noise.png";

    void createRaytracingNoiseTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

public:
    void createSurfelsResources(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, Camera *camera, VkCommandPool commandPool,
                                VkQueue graphicsQueue, std::vector<MeshContainer> sceneMeshes);
    void updateUniformBuffers(uint32_t width, uint32_t height, Camera *camera);

    static void mapSurfelsVisualizationData(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
                                            VkBuffer surfelsGeneratedData, VkBuffer &outVertexBuffer, VmaAllocation &outVertexAlloc, bool radianceVisualization);

    VkBuffer getSurfelPositionBuffer();
    VkBuffer getSurfelBuffer();
    VkBuffer getSurfelStatsBuffer();
    VkBuffer getSurfelGridBuffer();
    VkBuffer getSurfelCellBuffer();
    VkBuffer getCameraSurfelBuffer();
    VkBuffer getTranslucentMaterialsBuffer();

    std::vector<VkBuffer> getIndexBufferList();
    std::vector<VkBuffer> getVertexBufferList();
    std::vector<size_t> getIndexBufferSizeList();
    std::vector<size_t> getVertexBufferSizeList();

    ImageCreator getRaysNoiseImage();
    ImageCreator getBlueNoiseImage();

    void cleanup(VkDevice device);
};