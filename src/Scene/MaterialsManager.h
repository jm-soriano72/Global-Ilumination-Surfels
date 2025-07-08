#pragma once

#include "Images/ImageCreator.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

class MaterialsManager
{
private:
    // Vector de creadores de imágenes para texturas y mapas
    // Mapa difuso y texturas difusas
    std::vector<ImageCreator> diffuseImageCreators;
    // Mapa de transparencia
    std::vector<ImageCreator> alphaImageCreators;
    // Mapa especular
    std::vector<ImageCreator> specularImageCreators;

public:

    MaterialsManager();

    void prepareTextureMaterials(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
    void cleanup(VkDevice device);

    std::vector<ImageCreator> getDiffuseImages();
    std::vector<ImageCreator> getAlphaImages();
    std::vector<ImageCreator> getSpecularImages();
    
    int getNumImages();

};