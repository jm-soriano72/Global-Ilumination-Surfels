#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "RenderPipeline.h"

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

class ComputePipeline : public RenderPipeline
{   
public:
    virtual void createGraphicsPipeline(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) = 0;
    void cleanup(VkDevice device) override;

    VkPipelineLayout getPipelineLayout() override;
    VkPipeline getGraphicsPipeline() override;
};