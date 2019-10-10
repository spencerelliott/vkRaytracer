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

#ifdef DEBUG_BUILD
	#define DEBUG_ERR(m) std::cerr << m
	#define DEBUG_OUT(m) std::cout << m
#else
	#define DEBUG_ERR(m)
	#define DEBUG_OUT(m)
#endif

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

				auto result = std::find_if(std::begin(allExtensions), std::end(allExtensions), [i, extensionToCheck](VkExtensionProperties prop) -> bool { return std::string(prop.extensionName).compare(extensionToCheck) == 0; });
				
				if (result == std::end(allExtensions)) {
					return false;
				}
			}
		}

		return true;
	}

	void createInstance() {
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

		// Retrieve the needed extensions for GLFW to function
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		if (!checkExtensions(glfwExtensionCount, glfwExtensions)) {

		}

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		createInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Could not create Vulkan instance");
		}
	}

	void initVulkan() {
		createInstance();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	int width;
	int height;

	GLFWwindow* window;
	VkInstance instance;
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