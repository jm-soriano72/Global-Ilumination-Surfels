#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include "VertexBuffer.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
// Mediante esta variable, se activan o desactivan los validationLayers en funci�n de si se est� en modo Debug o no
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class TriangleApplication {
public:

	// Funci�n para leer de un fichero de texto
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary); // Se comienza a leer por el final

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		// Al comenzar a leer por el final, se puede determinar el tama�o del fichero
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		// Se vuelve al inicio del fichero y se comienza a leer
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}


private:

	// Referencias a clases externas
	// Buffer de v�rtices
	VertexBuffer vertexData;

	GLFWwindow* window; // Referencia a la ventana
	VkInstance instance; // Instancia a Vulkan

	VkDebugUtilsMessengerEXT debugMessenger; // Gesti�n de los mensajes de Debug

	VkPhysicalDevice physicalDevice; // Referencia a la GPU seleccionada
	VkDevice device; // Referencia al dispositivo l�gico, accesible desde cualquier clase

	VkQueue graphicsQueue; // Referencia a la queue de gr�ficos

	VkSurfaceKHR surface; // Referencia a la superficie donde se van a renderizar las im�genes
	VkQueue presentQueue;

	// Lista con los requerimientos necesarios
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME // Capacidad para utilizar swap chain
	};

	VkSwapchainKHR swapChain; // Referencia al swap chain
	std::vector<VkImage> swapChainImages; // Im�genes del swap chain
	VkFormat swapChainImageFormat; // Formato 
	VkExtent2D swapChainExtent; // Resoluci�n

	std::vector<VkImageView> swapChainImageViews; // Para poder acceder a las im�genes

	// Requerimientos necesarios para soportar swap chain
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	// Funci�n que devuelve si se cumplen los requerimientos que necesita swap chain
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		// Capacidades de la superficie
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
		// Opciones de formato
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		// Modos de presentaci�n
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}
		return details;
	}

	// Funciones para escoger las mejores configuraciones para el swap chain
	// Formato
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		// Se busca si se tiene el formato deseado
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		// En caso de que no, se debuelbe el primero
		return availableFormats[0];
	}
	// Presentaci�n de las im�genes
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			// Se trata de buscar el modo que simula un triple buffer
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		// Si no, se selecciona una simple cola
		return VK_PRESENT_MODE_FIFO_KHR;
	}
	// Resoluci�n �ptima de las im�genes
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		// Soluci�n al conflicto por la funci�n max
		uint32_t maxValue = (std::numeric_limits<uint32_t>::max)();
		if (capabilities.currentExtent.width != maxValue) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};
			// Se colocan los valores dentro de los l�mites establecidos
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	VkPipelineLayout pipelineLayout; // Necesario para configurar el cauce gr�fico y pasar variables uniform a los shaders
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline; // Pipeline de renderizado

	// Vector que va a almacenar todos los framebuffers
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkCommandPool commandPool; // Maneja la memoria utilizada para guardar los buffers
	VkCommandBuffer commandBuffer;

	// Herramientas de sincronizaci�n

	const uint32_t MAX_FRAMES_IN_FLIGHT = 1;
	uint32_t currentFrame = 0;

	std::vector<VkSemaphore> imageAvailableSemaphores = std::vector<VkSemaphore>(MAX_FRAMES_IN_FLIGHT);
	std::vector<VkSemaphore> renderFinishedSemaphores = std::vector<VkSemaphore>(MAX_FRAMES_IN_FLIGHT);
	std::vector<VkFence> inFlightFences = std::vector<VkFence>(MAX_FRAMES_IN_FLIGHT);


	bool framebufferResized = false; // Se indica si se ha redimensionado el framebuffer

	// FUNCIONES

	// Funci�n para inicializar la ventana, con la librer�a GLFW
	void initWindow() {
		// Primero se inicializa la librer�a
		glfwInit();
		// GLFW suele tratar con OpenGL, por lo que se debe evitar que se cree un contexto tal
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// Tras establecer la configuraci�n, se crea la ventana y se almacena una referencia a ella
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		// Se establece un puntero a la ventana para poder acceder a ella correctamente
		glfwSetWindowUserPointer(window, this);
		// Funci�n que se llamar� cuando se redimensione la ventana
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<TriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	// Funci�n para inicializar todos los objetos necesarios para el funcionamiento de Vulkan
	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		vertexData.createBufferData(device, physicalDevice, commandPool, graphicsQueue);
		createCommandBuffer();
		createSyncObjects();
	}

	// Creaci�n de la instancia de Vulkan
	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}
		// Se crea una estructura en la que se almacena informaci�n sobre la aplicaci�n
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // Se indica el tipo del objeto de forma expl�cita
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Se crea una nueva estructura en la que se indican las extensiones que se van a usar
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Se obtienen las extensiones necesarias mediante la funci�n auxiliar
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Creaci�n del objeto de Debug con las layers que se necesitan activar
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		// Una vez hechas todas las especificaciones, se puede crear la instancia como tal
		// Se necesitan:
		// - Un puntero a la clase createInfo
		// - Un puntero a callbacks personalizados
		// - Otro puntero a la variable que almacena el nuevo objeto

		// Se comprueba si la instancia se crea correctamente
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}

	}

	// Esta funci�n se utiliza para obtener todos los validationLayers disponibles
	bool checkValidationLayerSupport() {
		// Primero se obtiene el n�mero de layers disponibles
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		// Se crea el vector con dicho tama�o y se incluyen todos los disponibles
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		// Despu�s se comprueba si todos los validationLayers indicados en el vector se encuentran en el vector de los disponibles
		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	// Esta funci�n sirve para obtener las extensiones necesarias bas�ndose en s� se han activado los validationLayers o no
	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	// Creaci�n de la estructura de Debug
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void setupDebugMessenger() {
		// Si no se han activado los validationLayers, no se prepara la configuraci�n para Debug
		if (!enableValidationLayers) return;
		// Se crea una estructura 
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);
		// Se comprueba si se ha podido cargar la funci�n
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	// VKDebugUtilsMessengerEXT es parte de una extensi�n de Vulkan, por lo que no se carga autom�ticamente al crear una instancia de Vulkan
	// Por lo tanto, se necesita obtener la direcci�n de esta funci�n utilizando vkGetInstanceProcAddr
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	// Se necesita destruir el objeto DebugUtils
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	// Funci�n para Debug:
	// - El primer miembro indica la severidad o gravedad del mensaje
	// - El segundo par�metro indica el tipo de mensaje, es decir, si el error se debe a las especificaciones o no, o si no se est� usando bien Vulkan
	// - El tercero es una estructura que almacena el mensaje (string), un array de objetos Vulkan y el n�mero de dichos objetos
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void createSurface() {
		// Creaci�n de la superficie donde se va a pintar
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	// Esta funci�n se utiliza para seleccionar una tarjeta gr�fica que soporte las caracter�sticas necesarias
	void pickPhysicalDevice() {

		physicalDevice = VK_NULL_HANDLE;
		// Se listan las tarjetas gr�ficas disponibles
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		// Si no hay ninguna, no se continua
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// Se recorre la lista de gpus para comprobar si hay alguna �til
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		// Se buscan las queues compatibles con la GPU
		// Primero se obtiene el tama�o
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		// Despu�s se rellena la lista
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// Se rellena la lista de �ndices
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// Se busca tambi�n una queue family que sea posible de presentar la surface
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			if (indices.isComplete()) {
				break;
			}
			i++;
		}

		return indices;
	}

	// Funci�n auxiliar que comprueba si una tarjeta gr�fica es adecuada o no
	bool isDeviceSuitable(VkPhysicalDevice device) {
		// Se comprueba que el dispositivo tenga familias de queues asociadas
		QueueFamilyIndices indices = findQueueFamilies(device);
		// Se comprueba si el dispositivo soporta las extensiones requeridas
		bool extensionsSupported = checkDeviceExtensionSupport(device);
		// Se comprueban tambi�n los requerimientos
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}
		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	// Se crea un dispositivo l�gico con las caracter�sticas necesarias
	void createLogicalDevice() {
		// Primero se describe el n�mero de queues que se necesitan para cada familia
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // Se hace un vector para varias queues
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		// Se asignan prioridades
		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		// Se crea una estructura con todas las caracter�sticas (queues, layers...)
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}
		// Se crea el dispositivo y se comprueba si produce alg�n error
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}
		// Se obtiene la queue de gr�ficos
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		// Se obtiene la queue de presentaci�n
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	// Funci�n para crear el swap chain como tal
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
		// Se obtienen las caracter�sticas
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		swapChainImageFormat = surfaceFormat.format;
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
		swapChainExtent = extent;
		// Se establece el n�mero de im�genes que se tendr�n. Es recomendable tener una m�s que el m�nimo establecido
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		// Se comprueba respecto al m�ximo n�mero de im�genes permitido
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		// Estructura para su creaci�n
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; // Siempre se vuelve a indicar el tipo
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Se especifica c�mo manejar las im�genes, entre las distintas familias
		// En esta caso, los gr�ficos y la presentaci�n
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			// La imagen ser� utilizada por varias familias que se la ir�n pasando
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			// La imagen primero la tiene una familia y despu�s pasa a otra
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // Se puede aplicar una transformaci�n a las im�genes
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Se ignora el canal alfa
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // Se activa el clipping
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		// Se prepara a la lista de im�genes
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size()); // Se deja espacio para la creaci�n de todas las im�genes
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			// Creaci�n de la vista de cada imagen
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // C�mo se va a interpretar la imagen
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			// Im�genes sin mipmapping
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			// Se comprueba que no haya ning�n problema al crear cada view
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	/// IMPLEMENTACI�N DEL CAUCE GR�FICO ///

	void createGraphicsPipeline() {
		auto vertShaderCode = readFile(RESOURCES_PATH "shaders/vertex.spv");
		auto fragShaderCode = readFile(RESOURCES_PATH "shaders/frag.spv");

		// Se crean los m�dulos
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		// Creaci�n de las etapas del cauce gr�fico
		// Etapa de v�rtices
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; // Se indica en qu� etapa se va a usar el shader
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule; // M�dulo del shader
		vertShaderStageInfo.pName = "main"; // Punto de entrada
		// vertShaderStageInfo.pSpecializationInfo se puede utilizar para establecer valores para las constantes de los shaders
		// Etapa de fragmentos
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";
		// Array con las etapas programables
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// Especificaci�n del formato de los v�rtices de entrada
		// Se toma la informaci�n de la clase VertexBuffer
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		// Se obtiene la informaci�n para bind
		auto bindingDescription = VertexBuffer::Vertex::getBindingDescription();
		// Se obtiene la especificaci�n para los atributos de los v�rtices
		auto attributeDescriptions = VertexBuffer::Vertex::getAttributeDescriptions();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// Tratamiento de los v�rtices, c�mo se va a dibujar la geometr�a
		// Topolog�a:
		// - Cada v�rtice es un punto
		// - Se crea una l�nea entre cada dos v�rtices, sin reutilizarlos
		// - Se crea una l�nea entre cada dos v�rtices, reutilizando el �ltimo
		// - Se crea un tri�ngulo entre cada 3 v�rtices, sin reutilizarlos
		// - Se crea un tri�ngulo entre cada 3 v�rtices, reutilizando el 2 y 3
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Configuraci�n del viewport (din�mico)
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		// El scisson determina la parte de la imagen que se va a mostrar, se puede recortar
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		// Definici�n de elementos que van a tener un estado din�mico -> viewport y scissor
		// Al ser din�micos, habr� que indicar su valor en el momento de la ejecuci�n

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// Se indica que el viewport y el scissor van a ser inmutables
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// El rasterizador toma la geometr�a creada mediante los v�rtices y lo convierte en fragmentos tratables por el shader
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE; // Si se activa esto, ninguna geometr�a sobrepasa la etapa de fragmentos y nada se muestra por pantalla
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Se indica c�mo se generan los fragmentos a partir de la geometr�a. Este indica que se van rellenando los pol�gonos con fragmentos
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Configuraci�n del culling
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Sentido horario o antihorario
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		// Configuraci�n del multisampling, para evitar aliasing
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Color blending -> configuraci�n del color del fragmento en funci�n de la salida del shader y el color en el framebuffer
		// Se puede hacer con una combinaci�n de ambos o una operaci�n bitwise
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		// Array de estructuras para todos los framebuffers, que permite establecer constantes para calcular el color en el proceso de blend
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE; // Para seguir la segunda opci�n, con bitwise
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// Creaci�n del pipeline layout, para pasar variables uniform a los shaders
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		// Se combinan todos los elementos anteriores para crear el pipeline de renderizado como tal
		// Las partes necesarias son:
		// - M�dulos de las etapas programables (shaders)
		// - Conjunto de estructuras que definen caracter�sticas del pipeline (rasterizador, color blending, supersampling...)
		// - Pipeline layout, que indica las variables uniformes que pueden ser utilizadas por los shaders
		// - Render pass, que indica todas las asignaciones referenciadas por los shaders (framebuffer de color)

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2; // N�mero de etapas programables
		pipelineInfo.pStages = shaderStages; // M�dulos de las etapas progamables
		// Estructuras
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		// Pipeline layout
		pipelineInfo.layout = pipelineLayout;
		// Render passes
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0; // �ndice del subpass que se va a utilizar
		// Vulkan permite derivar un pipeline de renderizado que derive de otro, ya que es m�s eficiente si tiene muchas funcionalidades en com�n
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		// Creaci�n del pipeline
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// Los m�dulos s�lo se necesitan para la creaci�n del cauce gr�fico, una vez hecho, se destruyen
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	VkShaderModule createShaderModule(const std::vector<char>& code) {
		// Se crea un m�dulo o estructura que almacena el c�digo le�do de los ficheros de los shaders
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	// Esta funci�n se utiliza para determinar cu�ntos framebuffers se van a utilizar durante el renderizado
	void createRenderPass() {
		// Solo se va a tener un buffer de color, representado por una imagen del swap chain
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Como no se va a hacer multisampling, solo se toma 1 muestra
		// Se indica qu� hacer con los datos antes de renderizar (pintar encima, limpiar, o ignorar)
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// Se indica qu� hacer con los datos. Como se quiere mantener el tri�ngulo, se guardan
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// Se ignora lo que hubiera en el layout antes de renderizar, ya que se va a limpiar
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// Como se va a mostrar la imagen por pantalla:
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Una pasada de renderizado puede estar compuesta por m�ltiples subpasadas
		// Cada subpasada es secuencial, es decir, utiliza el contenido de los framebuffer de la pasada anterior
		// Esto puede ser �til para operaciones de post procesado por ejemplo
		// En este caso s�lo se crea una subpasada
		// Hace referencia al �nico buffer de color
		VkAttachmentReference colorAttachmentRef{}; // Referencia al buffer del color
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef; // El �ndice es referenciado desde el shader de fragmentos con layout(location = 0) out vec4 outColor

		// Dependencias entre subpasses
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0; // Primera y �nica subpass
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Se necesita esperar a la salida de color
		dependency.srcAccessMask = 0;
		// Se evita que comience directamente la transici�n de im�genes hasta que sea necesario
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// Se crea la pasada de renderizado
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1; // Se asigna un �nico framebuffer de color
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1; // Se indica el �nico subpass
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency; // Dependencias

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

	}

	void createFramebuffers() {
		// Se necesita crear un framebuffer para cada una de las im�genes del swap chain
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass; // Se especifica el render pass
			framebufferInfo.attachmentCount = 1; // S�lo se va a utilizar el color
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

	}

	// Las operaciones de dibujado y transferencia de memoria no se ejecutan directamente, sino que se deben almacenar
	// en un pool

	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// Solo hay dos posibles flags, para reestablecer los commands en conjunto muy frecuentemente, o poder hacerlo individualmente
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	// Creaci�n del espacio de memoria para los comandos
	void createCommandBuffer() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Nivel primario, no pueden ser llamados desde otros command buffers, se llaman directamente
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	// Funci�n que escribe el comando que se quiere ejecutar en el commandBuffer
	// Se indica el �ndice de la imagen en la que se quiere escribir
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// Se comienza el render pass para escribir el comando
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		// Se relaciona el framebuffer seg�n el �ndice de la imagen a escribir, as� se escoge el correcto
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		// Tama�o del �rea a renderizar
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		// Color al limpiar la escena
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} }; // Negro con 100% de opacidad
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		// Se comienza con el comando del render pass
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		// Se relaciona el pipeline gr�fico creado
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		// Se relaciona el Vertex Buffer 
		// Se obtiene el buffer de la clase externa
		VkBuffer vertexBuffers[] = { vertexData.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		// Se pasa el Buffer de �ndices
		vkCmdBindIndexBuffer(commandBuffer, vertexData.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

		// Se indic� que el Viewport y el Scissor eran din�micos, por lo tanto hay que indicar sus caracter�sticas
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Se a�ade el comando de dibujado, utilizando los datos de los �ndices de los v�rtices
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(vertexData.indices.size()), 1, 0, 0, 0);
		// 1 significa que no se realiza instance rendering
		// Los dos par�metros siguientes indican el offset con el que se comienza a utilizar los v�rtices y la instancia, en el caso de instanced rendering

		// Una vez hecho esto, se puede terminar el Render Pass
		vkCmdEndRenderPass(commandBuffer);
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void createSyncObjects() {
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Se indica que por defecto al crearse, la se�al es de finalizado, para evitar que se bloquee en la primera instrucci�n

			// Se crean todas las herramientas de sincronizaci�n (dos sem�foros y un fence)
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create semaphores!");
			}
		}
	}

	// Funci�n encargada de renderizar cada frame, que se ir� ejecutando hasta que se cierre la ventana
	void mainLoop() {
		// Se hace que la funci�n se ejecute hasta que se cierre la ventana
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(device);
	}

	void drawFrame() {
		// Los pasos para renderizar cada frame son los siguientes:

		// 1. Esperar que termine el frame anterior
		// (Esto es problem�tico la primera vez que se ejecute, porque no habr� una se�al previa)
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX); // Esta funci�n toma un array de fences, al indicar Vk_TRUE hay que esperar a todos (no influye porque s�lo tenemos uno)

		// 2. Tomar una imagen del swap chain
		uint32_t imageIndex; // �ndice de la imagen tomada, para escgoer el framebuffer asociado
		// Se referencia al dispositivo l�gico y el swap chain, junto con la herramienta de sincronizaci�n
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		// Si se detecta alguna anomal�a, se recrea la SwapChain
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		// Se resetea el fence s�lo si se est� presentando trabajo
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		// 3. Almacenar la acci�n en un command buffer que dibuja la escena en dicha imagen
		// Primero se resetea el command buffer
		vkResetCommandBuffer(commandBuffer, 0);
		recordCommandBuffer(commandBuffer, imageIndex);

		// 4. Presentar dicho command buffer a las queues de ejecuci�n
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1; // N�mero de sem�foros hay que esperar antes de ejecutar
		submitInfo.pWaitSemaphores = waitSemaphores; // Lista de sem�foros que hay que esperar
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer; // Referencia al command buffer para 
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores; // Lista de sem�foros que activar cuando se finaliza la ejecuci�n
		// Se pasa a la queue
		// El fence hace que la CPU espere a que se termine esta ejecuci�n antes de comenzar a calcular el siguiente frame
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// 5. Mostrar la imagen del swap chain
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores; // Se debe esperar a que termine la ejecuci�n del command buffer
		// Im�genes de la swap chain a presentar
		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional
		// Funci�n de presentaci�n
		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		// Se comprueba si la SwapChain no es �ptima
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	// Funci�n para recrear la Swap Chain, para adaptarse a los cambios en la ventana, por ejemplo, de tama�o
	void recreateSwapChain() {
		// Redimensai�n del framebuffer
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		// Se evita tocar recursos que est� en uso
		vkDeviceWaitIdle(device);
		// Se vac�a la swap chain actual
		cleanupSwapChain();
		// Se recrean todos los objetos involucrados (la Image Views conforman la Swap Chain)
		createSwapChain();
		createImageViews();
		// Los Framebuffers dependen directamente de las im�genes
		createFramebuffers();
	}

	void cleanupSwapChain() {
		// Antes de destruir el Pipeline y el Render Pass, se eliminan los framebuffer asociados
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		// Se eliminan las view images
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
		// Antes de eliminar el dispositivo l�gico, se destruye el swap chain
		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	// Funci�n para liberar los recursos utilizados una vez se termine
	void cleanup() {
		// Se limpian todos los recursos de la swap chain
		cleanupSwapChain();
		// Se liberan los recursos del buffer de v�rtices
		vertexData.cleanup(device);
		// Se eliminan las herramientas de sincronizaci�n
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		// Destrucci�n del command pool
		vkDestroyCommandPool(device, commandPool, nullptr);
		// Se destruye el Pipeline de renderizado
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		// Se destruye el Pipeline Layout
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		// Se destruye el pass de renderizado
		vkDestroyRenderPass(device, renderPass, nullptr);

		// Se destruye el dispositivo l�gico
		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		// Antes de destruir la instancia, se elimina la superficie asociada
		vkDestroySurfaceKHR(instance, surface, nullptr);
		// Antes de destruir la ventana, se destruye la instancia
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}
};
