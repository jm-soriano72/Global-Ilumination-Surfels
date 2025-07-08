#pragma once

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

class RenderPipeline
{
protected:
    VkPipelineLayout pipelineLayout;
    VkPipeline geometryPipeline;     

public:
    virtual void cleanup(VkDevice device) = 0;

    virtual VkPipelineLayout getPipelineLayout() = 0;
    virtual VkPipeline getGraphicsPipeline() = 0;
};