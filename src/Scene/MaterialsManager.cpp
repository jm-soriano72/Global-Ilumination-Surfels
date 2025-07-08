#include "MaterialsManager.h"

#include "Images/ImageCreator.h"
#include "SponzaResources.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

MaterialsManager::MaterialsManager() {}

void MaterialsManager::prepareTextureMaterials(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
    diffuseImageCreators.resize(diffuseTexturesPath.size());

    for (uint32_t i = 0; i < diffuseTexturesPath.size(); i++)
    {
        diffuseImageCreators[i].createTextureImage(device, physicalDevice, commandPool, graphicsQueue, diffuseTexturesPath[i]);
        diffuseImageCreators[i].createTextureImageView(device);
        diffuseImageCreators[i].createTextureSampler(device, physicalDevice);
    }
    // Texturas de transparencia
    alphaImageCreators.resize(alphaTexturesPath.size());

    for (uint32_t i = 0; i < alphaTexturesPath.size(); i++)
    {
        alphaImageCreators[i].createTextureImage(device, physicalDevice, commandPool, graphicsQueue, alphaTexturesPath[i]);
        alphaImageCreators[i].createTextureImageView(device);
        alphaImageCreators[i].createTextureSampler(device, physicalDevice);
    }
    // Texturas especulares
    specularImageCreators.resize(specularTexturesPath.size());

    for (uint32_t i = 0; i < specularTexturesPath.size(); i++)
    {
        specularImageCreators[i].createTextureImage(device, physicalDevice, commandPool, graphicsQueue, specularTexturesPath[i]);
        specularImageCreators[i].createTextureImageView(device);
        specularImageCreators[i].createTextureSampler(device, physicalDevice);
    }
}

void MaterialsManager::cleanup(VkDevice device) {
    for (uint32_t i = 0; i < diffuseTexturesPath.size(); i++)
    {
        diffuseImageCreators[i].cleanup(device);
        alphaImageCreators[i].cleanup(device);
        specularImageCreators[i].cleanup(device);
    }
}

std::vector<ImageCreator> MaterialsManager::getDiffuseImages() {
    return diffuseImageCreators;
}

std::vector<ImageCreator> MaterialsManager::getAlphaImages() {
    return alphaImageCreators;
}

std::vector<ImageCreator> MaterialsManager::getSpecularImages() {
    return specularImageCreators;
}

int MaterialsManager::getNumImages() {
    return diffuseImageCreators.size();
}
