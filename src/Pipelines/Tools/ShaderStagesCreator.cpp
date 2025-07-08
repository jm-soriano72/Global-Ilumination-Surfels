#include "ShaderStagesCreator.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <cstdint>
#include <limits>
#include <algorithm>

std::vector<char> ShaderStagesCreator::readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary); // Se comienza a leer por el final

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }
    // Al comenzar a leer por el final, se puede determinar el tamaño del fichero
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    // Se vuelve al inicio del fichero y se comienza a leer
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule ShaderStagesCreator::createShaderModule(const std::vector<char> &code, VkDevice device)
{
    // Se crea un módulo o estructura que almacena el código leído de los ficheros de los shaders
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}