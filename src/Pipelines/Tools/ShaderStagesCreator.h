#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <fstream>
#include <vector>
#include <string>

class ShaderStagesCreator
{
public:
    static std::vector<char> readFile(const std::string &filename);
    static VkShaderModule createShaderModule(const std::vector<char> &code, VkDevice device);
};