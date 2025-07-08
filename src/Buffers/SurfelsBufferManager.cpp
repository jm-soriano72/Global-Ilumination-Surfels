#include "SurfelsBufferManager.h"

#include "Images/ImageCreator.h"
#include "Tools/BufferCreator.h"
#include "Camera/Camera.h"
#include "Config.h"
#include "Scene/SponzaResources.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <random>

void SurfelsBufferManager::createSurfelsResources(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, Camera *camera, VkCommandPool commandPool,
                                                  VkQueue graphicsQueue, std::vector<MeshContainer> sceneMeshes)
{
    // Creación de los buffer de escritura de los surfels
    BufferCreator::createBufferVMA(
        width / 16 * height / 16 * sizeof(glm::vec2),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        surfelPositionBuffer,
        surfelPositionBufferAllocation);
    BufferCreator::createBufferVMA(
        sizeof(Surfel) * SURFEL_CAPACITY,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        surfelBuffer,
        surfelBufferAllocation);
    BufferCreator::createBufferVMA(
        sizeof(unsigned int) * 8,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        surfelStatsBuffer,
        surfelStatsBufferAllocation);
    BufferCreator::createBufferVMA(
        sizeof(unsigned int) * SURFEL_TABLE_SIZE,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        surfelGridBuffer,
        surfelGridBufferAllocation);
    BufferCreator::createBufferVMA(
        sizeof(unsigned int) * SURFEL_TABLE_SIZE * SURFEL_CELL_LIMIT,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        surfelCellBuffer,
        surfelCellBufferAllocation);

    // Se crean y se rellenan los buffers con la información de la geometría, para poder extraerla desde los shaders
    for (auto &mesh : sceneMeshes)
    {
        for (int i = 0; i < mesh.vertexMeshesData.vertices.size(); i++)
        {

            size_t vertexBufferSize = mesh.vertexMeshesData.vertices[i].size() * sizeof(SimpleVertex);
            size_t indexBufferSize = mesh.vertexMeshesData.indices[i].size() * sizeof(uint16_t);

            VkBuffer vertexBuffer;
            VkBuffer indexBuffer;
            VmaAllocation vertexBufferAllocation;
            VmaAllocation indexBufferAllocation;

            BufferCreator::createBufferVMA(
                indexBufferSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                indexBuffer,
                indexBufferAllocation);

            void *indexMappedData;
            vmaMapMemory(BufferCreator::allocator, indexBufferAllocation, &indexMappedData);
            memcpy(indexMappedData, mesh.vertexMeshesData.indices[i].data(), indexBufferSize);
            vmaUnmapMemory(BufferCreator::allocator, indexBufferAllocation);

            BufferCreator::createBufferVMA(
                vertexBufferSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                vertexBuffer,
                vertexBufferAllocation);

            // Se simplifica la información de los vértices
            std::vector<SimpleVertex> vertexList;
            for(int j = 0; j < mesh.vertexMeshesData.vertices[i].size(); j++) {
                SimpleVertex vertex;
                vertex.normal = mesh.vertexMeshesData.vertices[i][j].normal;
                vertex.idMaterial = mesh.vertexMeshesData.vertices[i][j].idMaterial;
                vertex.uv = mesh.vertexMeshesData.vertices[i][j].texCoord;
                vertexList.push_back(vertex);
            }

            void *vertexMappedData;
            vmaMapMemory(BufferCreator::allocator, vertexBufferAllocation, &vertexMappedData);
            memcpy(vertexMappedData, vertexList.data(), vertexBufferSize);
            vmaUnmapMemory(BufferCreator::allocator, vertexBufferAllocation);

            vertexBufferList.push_back(vertexBuffer);
            vertexBufferAllocationList.push_back(vertexBufferAllocation);
            vertexBufferSizeList.push_back(vertexBufferSize);
            indexBufferList.push_back(indexBuffer);
            indexBufferAllocationList.push_back(indexBufferAllocation);
            indexBufferSizeList.push_back(indexBufferSize);
        }
    }

    // Creación del buffer de variables uniformes
    VkDeviceSize bufferSize = sizeof(CameraUniformBuffer);
    BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                uniformCameraBuffer, uniformCameraBufferMemory);
    vkMapMemory(device, uniformCameraBufferMemory, 0, bufferSize, 0, &uniformCameraBufferMapped);
    updateUniformBuffers(width, height, camera);

    bufferSize = sizeof(bool) * translucentMaterials.size();
    BufferCreator::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                translucentMaterialsBuffer, translucentMaterialsBufferMemory);
    vkMapMemory(device, translucentMaterialsBufferMemory, 0, bufferSize, 0, &translucentMaterialsBufferMapped);
    memcpy(translucentMaterialsBufferMapped, &translucentMaterials, sizeof(bool) * translucentMaterials.size());

    // Creación de la imagen con ruido para el trazado de rayos desde cada surfel
    createRaytracingNoiseTexture(device, physicalDevice, commandPool, graphicsQueue);

    // Creación de la imagen que contiene el blue noise
    blueNoiseImage.createTextureImage(device, physicalDevice, commandPool, graphicsQueue, blueNoisePath);
    blueNoiseImage.createTextureImageView(device);
    blueNoiseImage.createTextureSampler(device, physicalDevice);
}

void SurfelsBufferManager::updateUniformBuffers(uint32_t width, uint32_t height, Camera *camera)
{
    CameraUniformBuffer ubo{};

    // Se obtiene la matriz de vista a través de los datos de la cámara según los inputs del usuario
    ubo.view = camera->getViewMatrix();

    // Ajustar la proyección
    float zNear = 0.1f;
    float zFar = 3000.0f;

    ubo.projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, zNear, zFar);
    ubo.projection[1][1] *= -1;

    ubo.nearFarPlanes = glm::vec2(zNear, zFar);

    ubo.cameraPosition = camera->getPosition();

    memcpy(uniformCameraBufferMapped, &ubo, sizeof(ubo));
}

void SurfelsBufferManager::mapSurfelsVisualizationData(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
                                                       VkBuffer surfelsGeneratedData, VkBuffer &outVertexBuffer, VmaAllocation &outVertexAlloc, bool radianceVisualization)
{
    VkDeviceSize surfelsGeneratedBufferSize = sizeof(Surfel) * SURFEL_CAPACITY;

    // Se crea un stagging buffer para leer la información del buffer de surfels generados
    VkBuffer stagingReadBuf;
    VmaAllocation stagingReadAlloc;
    BufferCreator::createBufferVMA(
        surfelsGeneratedBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        stagingReadBuf,
        stagingReadAlloc);

    // Se copia del buffer original al creado
    VkCommandBuffer cmd = CommandBufferManager::beginSingleTimeCommands(commandPool, device);
    VkBufferCopy copyRegion{0, 0, surfelsGeneratedBufferSize};
    vkCmdCopyBuffer(cmd, surfelsGeneratedData, stagingReadBuf, 1, &copyRegion);
    CommandBufferManager::endSingleTimeCommands(cmd, graphicsQueue, device, commandPool);

    // Se extraen los datos del buffer
    Surfel *mappedSurfels = nullptr;
    vmaMapMemory(BufferCreator::allocator, stagingReadAlloc, (void **)&mappedSurfels);

    // Se rellena un vector con los datos de los surfels generados previamente, y se guarda la información indispensable para visualizarlos
    std::vector<SimpleSurfel> verts;
    verts.reserve(SURFEL_CAPACITY);
    for (size_t i = 0; i < SURFEL_CAPACITY; ++i)
    {
        SimpleSurfel v;
        v.position = mappedSurfels[i].position;
        v.radius = mappedSurfels[i].radius;
        if (renderConfig == RenderMode::SURFELS_RADIANCE_VISUALIZATION)
        {
            v.color = mappedSurfels[i].direct_radiance;
        }
        else
        {
            v.color = mappedSurfels[i].color;
        }
        v.normal = mappedSurfels[i].normal;
        v.padding0 = 0.0f;
        v.padding1 = 0.0f;
        verts.push_back(v);
    }

    vmaUnmapMemory(BufferCreator::allocator, stagingReadAlloc);

    // Se crea el buffer de vértices que se va a utilizar en GPU para pintar los surfels
    VkDeviceSize vbSize = sizeof(SimpleSurfel) * verts.size();
    BufferCreator::createBufferVMA(
        vbSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        outVertexBuffer,
        outVertexAlloc);

    // Se crea el stagging buffer intermedio para subir la información reservada al buffer creado
    VkBuffer stagingWriteBuf;
    VmaAllocation stagingWriteAlloc;
    BufferCreator::createBufferVMA(
        vbSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        stagingWriteBuf,
        stagingWriteAlloc);

    // Se copian los datos al stagging buffer
    void *dst = nullptr;
    vmaMapMemory(BufferCreator::allocator, stagingWriteAlloc, &dst);
    memcpy(dst, verts.data(), (size_t)vbSize);
    vmaUnmapMemory(BufferCreator::allocator, stagingWriteAlloc);

    // Por último, se copian los datos del stagging buffer al buffer de salida
    VkCommandBuffer cmdFinal = CommandBufferManager::beginSingleTimeCommands(commandPool, device);
    VkBufferCopy copyRegionFinal{0, 0, vbSize};
    vkCmdCopyBuffer(cmdFinal, stagingWriteBuf, outVertexBuffer, 1, &copyRegionFinal);
    CommandBufferManager::endSingleTimeCommands(cmdFinal, graphicsQueue, device, commandPool);

    // Se eliminan los stagging buffers auxiliares
    vmaDestroyBuffer(BufferCreator::allocator, stagingReadBuf, stagingReadAlloc);
    vmaDestroyBuffer(BufferCreator::allocator, stagingWriteBuf, stagingWriteAlloc);
}

void SurfelsBufferManager::createRaytracingNoiseTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
    // Generación aleatoria de pares entre [0,1]
    std::vector<glm::vec2> *noiseTexture = new std::vector<glm::vec2>();

    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (uint32_t i = 0; i < NUM_RAYS * NUM_RAYS; i++)
    {
        float u1 = dist(rng);
        float u2 = dist(rng);
        noiseTexture->push_back(glm::vec2(u1, u2));
    }
    noiseImage.createRaysNoiseTextureImage(device, physicalDevice, commandPool, graphicsQueue, noiseTexture, NUM_RAYS, NUM_RAYS);
}

VkBuffer SurfelsBufferManager::getSurfelPositionBuffer()
{
    return surfelPositionBuffer;
}

VkBuffer SurfelsBufferManager::getSurfelBuffer()
{
    return surfelBuffer;
}

VkBuffer SurfelsBufferManager::getSurfelStatsBuffer()
{
    return surfelStatsBuffer;
}

VkBuffer SurfelsBufferManager::getSurfelGridBuffer()
{
    return surfelGridBuffer;
}

VkBuffer SurfelsBufferManager::getSurfelCellBuffer()
{
    return surfelCellBuffer;
}

VkBuffer SurfelsBufferManager::getCameraSurfelBuffer()
{
    return uniformCameraBuffer;
}

VkBuffer SurfelsBufferManager::getTranslucentMaterialsBuffer()
{
    return translucentMaterialsBuffer;
}

std::vector<VkBuffer> SurfelsBufferManager::getIndexBufferList()
{
    return indexBufferList;
}

std::vector<VkBuffer> SurfelsBufferManager::getVertexBufferList()
{
    return vertexBufferList;
}

std::vector<size_t> SurfelsBufferManager::getIndexBufferSizeList()
{
    return indexBufferSizeList;
}

std::vector<size_t> SurfelsBufferManager::getVertexBufferSizeList()
{
    return vertexBufferSizeList;
}

ImageCreator SurfelsBufferManager::getRaysNoiseImage()
{
    return noiseImage;
}

ImageCreator SurfelsBufferManager::getBlueNoiseImage()
{
    return blueNoiseImage;
}

void SurfelsBufferManager::cleanup(VkDevice device)
{
    vmaDestroyBuffer(BufferCreator::allocator, surfelPositionBuffer, surfelPositionBufferAllocation);
    vmaDestroyBuffer(BufferCreator::allocator, surfelBuffer, surfelBufferAllocation);
    vmaDestroyBuffer(BufferCreator::allocator, surfelStatsBuffer, surfelStatsBufferAllocation);
    vmaDestroyBuffer(BufferCreator::allocator, surfelGridBuffer, surfelGridBufferAllocation);
    vmaDestroyBuffer(BufferCreator::allocator, surfelCellBuffer, surfelCellBufferAllocation);

    for (int i = 0; i < vertexBufferList.size(); i++)
    {
        vmaDestroyBuffer(BufferCreator::allocator, indexBufferList[i], indexBufferAllocationList[i]);
        vmaDestroyBuffer(BufferCreator::allocator, vertexBufferList[i], vertexBufferAllocationList[i]);
    }

    vkDestroyBuffer(device, uniformCameraBuffer, nullptr);
    vkFreeMemory(device, uniformCameraBufferMemory, nullptr);
    vkDestroyBuffer(device, translucentMaterialsBuffer, nullptr);
    vkFreeMemory(device, translucentMaterialsBufferMemory, nullptr);

    noiseImage.cleanup(device);
    blueNoiseImage.cleanup(device);
}
