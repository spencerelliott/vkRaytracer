#define DEBUG_BUILD

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <algorithm>

const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
};

#ifdef DEBUG_BUILD
#define DEBUG_ERR(m) std::cerr << m
#define DEBUG_OUT(m) std::cout << m
#else
#define DEBUG_ERR(m)
#define DEBUG_OUT(m)
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

class HelloTriangleApplication {
public:
	HelloTriangleApplication(const int width, const int height) : width(width), height(height) { }

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		DEBUG_OUT("validation layer: " << pCallbackData->pMessage << std::endl);

		return VK_FALSE;
	}

	bool checkExtensions(int extensionCheckCount, const char* const* extensionCheck) {
		uint32_t extensionCount = 0;

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> allExtensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, allExtensions.data());

		DEBUG_OUT("Extensions (" << extensionCount << "):" << std::endl);

		for (const auto& extension : allExtensions) {
			DEBUG_OUT("\t" << extension.extensionName << std::endl);
		}

		if (extensionCheckCount > 0) {
			for (int i = 0; i < extensionCheckCount; i++) {
				std::string extensionToCheck(extensionCheck[i]);

				auto result = std::find_if(std::begin(allExtensions), std::end(allExtensions), [extensionToCheck](VkExtensionProperties prop) -> bool { return std::string(prop.extensionName).compare(extensionToCheck) == 0; });

				if (result == std::end(allExtensions)) {
					return false;
				}
			}
		}

		return true;
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;

		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> allProperties(layerCount);

		vkEnumerateInstanceLayerProperties(&layerCount, allProperties.data());

		for (auto layer : validationLayers) {
			std::string layerName(layer);

			auto layerProperties = std::find_if(std::begin(allProperties), std::end(allProperties), [layerName](VkLayerProperties prop) -> bool { return layerName.compare(prop.layerName) == 0; });

			if (layerProperties == std::end(allProperties)) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions() {
		// Retrieve the needed extensions for GLFW to function
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> allExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef DEBUG_BUILD
		allExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return allExtensions;
	}

	void createInstance() {
#ifdef DEBUG_BUILD
		if (!checkValidationLayerSupport()) {
			throw std::runtime_error("Missing required validation layer");
		}
#endif

		// Setup the application info
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		// Create the create instance info and add the application info
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

#ifdef DEBUG_BUILD
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;
#endif		

		// Collect all of the extensions we will be using
		std::vector<const char*> allExtensions = getRequiredExtensions();

		if (!checkExtensions(static_cast<int>(allExtensions.size()), allExtensions.data())) {
			throw std::runtime_error("Missing required extension");
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(allExtensions.size());
		createInfo.ppEnabledExtensionNames = allExtensions.data();

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Could not create Vulkan instance");
		}
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo) {
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = nullptr;
	}

	void setupDebugMessenger() {
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		populateDebugMessengerCreateInfo(debugCreateInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("Could not set up debug messenger");
		}
	}

	void initVulkan() {
		createInstance();
#ifdef DEBUG_BUILD
		setupDebugMessenger();
#endif
		pickPhysicalDevice();
	}

	bool isSuitableDevice(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
			deviceFeatures.geometryShader;
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;

		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount <= 0) {
			throw std::runtime_error("Could not find a Vulkan GPU");
		}

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);

		vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

		for (auto device : physicalDevices) {
			if (isSuitableDevice(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Could not find a suitable GPU");
		}
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
#ifdef DEBUG_BUILD
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif

		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	int width;
	int height;

	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger;
};

int main() {
	HelloTriangleApplication app(800, 600);

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}