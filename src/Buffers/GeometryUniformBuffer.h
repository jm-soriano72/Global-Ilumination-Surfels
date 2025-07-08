#pragma once

#include "Images/ImageCreator.h"
#include "Camera/Camera.h"
#include "Scene/Illumination/LightsData.h"
#include "ShadowMappingUniformBuffer.h"

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

// ESTRUCTURAS //
// Se define la estructura del uniform buffer object que se va a transmitir al shader de vértices
// Incluye las matrices de modelado (con las transformaciones geométricas), de vista (con las coordenadas de la cámara) y de proyección (para proyectar una imagen 3D en 2D)
// Alignas se utiliza para que haya una correspondencia entre los espaciados de memoria con los shaders
struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class GeometryUniformBuffer
{

private:
    // Matrices de transformación
    std::vector<VkBuffer> uniformMVPBuffers;
    std::vector<VkDeviceMemory> uniformMVPBuffersMemory;
    std::vector<void *> uniformMVPBuffersMapped;
    // Fuentes de luz
    std::vector<VkBuffer> lightBuffers;
    std::vector<VkDeviceMemory> lightBuffersMemory;
    std::vector<void *> lightBuffersMapped;
    // Matriz de transformación de la luz
    std::vector<VkBuffer> mainLightDataBuffers;
    std::vector<VkDeviceMemory> mainLightDataBuffersMemory;
    std::vector<void *> mainLightDataBuffersMapped;

public:
    void createGeometryUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT,
                                      uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights);
    void updateGeometryUniformBuffers(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera, LightsData sceneLights);
    void cleanup(VkDevice device);

    std::vector<VkBuffer> getMVPBuffers();
    std::vector<VkBuffer> getLightsBuffers();
    std::vector<VkBuffer> getMainLightBuffers();

private:
    void createMVPUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, uint32_t width, uint32_t height, Camera *camera);
    void createLightsBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT, LightsData sceneLights);
    void createMainLightDataBuffers(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t MAX_FRAMES_IN_FLIGHT);

    void updateMVPUniformBuffer(uint32_t currentImage, uint32_t width, uint32_t height, Camera *camera);
    void updateLightsBuffer(uint32_t currentImage, LightsData sceneLights);
    void updateMainLightDataBuffer(uint32_t currentImage);
};