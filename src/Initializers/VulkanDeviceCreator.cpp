#include "VulkanDeviceCreator.h"

#include "Tools/VulkanUtils.h"
#include "Buffers/Tools/BufferCreator.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <string>
#include <iostream>

VulkanDeviceCreator::VulkanDeviceCreator() {}

void VulkanDeviceCreator::createDevices(bool enableValidationLayers, const std::vector<const char *> &validationLayers, GLFWwindow *window)
{
    createInstance(enableValidationLayers, validationLayers);
    setupDebugMessenger(enableValidationLayers);
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice(enableValidationLayers, validationLayers);

    BufferCreator::initVMA(instance, physicalDevice, device);
}

void VulkanDeviceCreator::createInstance(bool enableValidationLayers, const std::vector<const char *> &validationLayers)
{
    if (enableValidationLayers && !checkValidationLayerSupport(validationLayers))
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Surfels_Rendering";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan_Illumination_Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    // Se obtienen las extensiones necesarias
    auto extensions = getRequiredExtensions(enableValidationLayers);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanDeviceCreator::setupDebugMessenger(bool enableValidationLayers)
{
    // Si no se han activado los validationLayers, no se prepara la configuración para Debug
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VulkanDeviceCreator::createSurface(GLFWwindow *window)
{
    // Creación de la superficie donde se va a pintar
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void VulkanDeviceCreator::pickPhysicalDevice()
{
    physicalDevice = VK_NULL_HANDLE;
    // Se listan las tarjetas gráficas disponibles
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Se recorre la lista de GPUs para comprobar si hay alguna útil
    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void VulkanDeviceCreator::createLogicalDevice(bool enableValidationLayers, const std::vector<const char *> &validationLayers)
{
    // Primero se describe el número de queues que se necesitan para cada familia
    QueueFamilyIndices indices = VulkanUtils::findQueueFamilies(physicalDevice, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // Se hace un vector para varias queues
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    // Se asignan prioridades
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Extensiones Vulkan 1.2
    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;
    features12.bufferDeviceAddress = VK_TRUE;

    // Ray tracing
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
    enabledRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
    enabledRayTracingPipelineFeatures.pNext = &features12;

    // Acceleration structures
    VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
    enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
    enabledAccelerationStructureFeatures.pNext = &enabledRayTracingPipelineFeatures;

    // Ray query
    VkPhysicalDeviceRayQueryFeaturesKHR enabledRayQueryFeatures{};
    enabledRayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    enabledRayQueryFeatures.rayQuery = VK_TRUE;
    enabledRayQueryFeatures.pNext = &enabledAccelerationStructureFeatures;

    // uint16_t en SSBOs
    VkPhysicalDevice16BitStorageFeatures storage16{};
    storage16.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
    storage16.storageBuffer16BitAccess = VK_TRUE;
    storage16.storagePushConstant16 = VK_FALSE;
    storage16.storageInputOutput16 = VK_FALSE;
    storage16.uniformAndStorageBuffer16BitAccess = VK_FALSE;
    storage16.pNext = &enabledRayQueryFeatures;

    // Encabezado del árbol
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;
    deviceFeatures.shaderInt16 = VK_TRUE;

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.features = deviceFeatures;
    features2.pNext = &storage16;

    // VkDeviceCreateInfo
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pNext = &features2;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    // Se crea el dispositivo y se comprueba si produce algún error
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
    // Se obtiene la queue de gráficos
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    // Se obtiene la queue de presentación
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanDeviceCreator::cleanupDevices(bool enableValidationLayers)
{
    // Se destruye el dispositivo lógico
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers)
    {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    // Antes de destruir la instancia, se elimina la superficie asociada
    vkDestroySurfaceKHR(instance, surface, nullptr);
    // Antes de destruir la ventana, se destruye la instancia
    vkDestroyInstance(instance, nullptr);
}

bool VulkanDeviceCreator::checkValidationLayerSupport(const std::vector<const char *> &validationLayers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            return false;
        }
    }
    return true;
}

std::vector<const char *> VulkanDeviceCreator::getRequiredExtensions(bool enableValidationLayers)
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanDeviceCreator::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDeviceCreator::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

VkResult VulkanDeviceCreator::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanDeviceCreator::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

bool VulkanDeviceCreator::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = VulkanUtils::findQueueFamilies(device, surface);
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = VulkanUtils::querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanDeviceCreator::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkSurfaceKHR VulkanDeviceCreator::getVkSurface() const { return this->surface; }
VkPhysicalDevice VulkanDeviceCreator::getVkPhysicalDevice() const { return this->physicalDevice; }
VkDevice VulkanDeviceCreator::getVkDevice() const { return this->device; }
VkQueue VulkanDeviceCreator::getVkGraphicsQueue() const { return this->graphicsQueue; }
VkQueue VulkanDeviceCreator::getVkPresentQueue() const { return this->presentQueue; }