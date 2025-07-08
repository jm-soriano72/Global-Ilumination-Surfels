#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>

class VulkanDeviceCreator
{

private:
    VkInstance instance;                     // Instancia a Vulkan
    VkDebugUtilsMessengerEXT debugMessenger; // Gestión de los mensajes de Debug
    VkSurfaceKHR surface;                    // Referencia a la superficie donde se van a renderizar las imágenes
    VkPhysicalDevice physicalDevice;         // Referencia a la GPU seleccionada
    VkDevice device;                         // Referencia al dispositivo lógico
    VkQueue graphicsQueue;                   // Referencia a la queue de gráficos
    VkQueue presentQueue;

    // Lista con los requerimientos necesarios
    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
        "VK_KHR_ray_query"
    };

    void createInstance(bool enableValidationLayers, const std::vector<const char *> &validationLayers);
    void setupDebugMessenger(bool enableValidationLayers);
    void createSurface(GLFWwindow *window);
    void pickPhysicalDevice();
    void createLogicalDevice(bool enableValidationLayers, const std::vector<const char *> &validationLayers);

    bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);
    std::vector<const char *> getRequiredExtensions(bool enableValidationLayers);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

public:

    VulkanDeviceCreator();
    
    void createDevices(bool enableValidationLayers, const std::vector<const char *> &validationLayers, GLFWwindow* window);
    void cleanupDevices(bool enableValidationLayers);

    VkSurfaceKHR getVkSurface() const;
    VkPhysicalDevice getVkPhysicalDevice() const;
    VkDevice getVkDevice() const;
    VkQueue getVkGraphicsQueue() const;
    VkQueue getVkPresentQueue() const;

};