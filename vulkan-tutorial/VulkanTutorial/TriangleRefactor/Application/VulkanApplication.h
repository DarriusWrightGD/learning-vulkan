#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW\glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <string>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <functional>
#include <glm\vec4.hpp>
#include <glm\mat4x4.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include "..\VkDeleter.h"
#include "..\VulkanDebug.h"
#include "..\VulkanValidation.h"
#include "..\Exception.h"
#include "..\FileReader.h"
#include "..\Builders\InstanceBuilder.h"
#include "..\Builders\FrameBufferInfoBuilder.h"
#include "..\Builders\GraphicsPipelineBuilder.h"
#include "..\Builders\RenderpassBuilder.h"
#include "..\Builders\SwapchainInfoBuilder.h"

using std::cout;
using std::endl;
using std::set;
#include "..\Systems\Graphics\IVulkanGraphicsSystem.h"
#include <memory>

class VulkanApplication {
public:
	VulkanApplication(std::shared_ptr<IVulkanGraphicsSystem> graphicsSystem) : width(800), height(600) {

	}
	//virtual ~VulkanApplication() noexcept = default;
	virtual void OnInit() { }
	virtual void OnResize(int width, int height) { }
	virtual void Draw(float time) { drawFrame(); }
	virtual void Update(float time) { }

	void run() {
		initVulkan();
		OnInit();
		mainLoop();
	}

protected:
	virtual void CreateGraphicsPipeline() = 0;
	virtual void CreateVertexBuffer() {};
	virtual void CreateDrawCommands(VkCommandBuffer commandBuffer) = 0;
private:
	static void onWindowResized(GLFWwindow * window, int width, int height) {
		if (width == 0 || height == 0) return;

		auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
		app->recreateSwapChain();
		app->setWidth(width);
		app->setHeight(height);
		app->OnResize(width, height);
	}

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, VulkanApplication::onWindowResized);
	}

	void setWidth(int width) { this->width = width; }
	void setHeight(int height) { this->height = height; }

	void recreateSwapChain() {
		vkDeviceWaitIdle(device);

		createSwapChain();
		createImageViews();
		createRenderPass();
		CreateGraphicsPipeline();
		createFramebuffers();
		createCommandBuffers();
	}

	void initInstance() {
		if (enableValidationLayers && !VulkanValidation::checkValidationLayerSupport(validationLayers)) {
			throw std::runtime_error("validation layers requested, but not available");
		}

		auto instanceBuilder = InstanceInfoBuilder();

		instanceBuilder
			.WithApplicationInfo(ApplicationInfoBuilder()
				.WithApplicationName("Hello Triangle")->Build());

		if (enableValidationLayers) {
			instanceBuilder.WithEnabledLayers(validationLayers.size(), validationLayers.data());
		}

		auto exten = VulkanValidation::getRequiredExtensions();
		instanceBuilder.WithEnabledExtensions(exten.size(), exten.data());
		vkOk(vkCreateInstance(&instanceBuilder.Build(), nullptr, &instance), "Failed to create instance!");
	}

	void initVulkan() {
		initWindow();
		initInstance();
		VulkanDebug::setupDebugCallback(instance, &callback);
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		CreateGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		CreateVertexBuffer();
		createCommandBuffers();
		createSemaphores();
	}

	void createSemaphores() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkOk(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore), "Failed to create semaphores!");
		vkOk(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore), "Failed to create semaphores!");
	}

	void createCommandBuffers() {
		if (commandBuffers.size() > 0) {
			vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());
		}

		commandBuffers.resize(swapChainFramebuffers.size());
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		vkOk(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()), "Failed to allocate command buffers");

		for (unsigned int i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;

			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];

			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			CreateDrawCommands(commandBuffers[i]);
			vkCmdEndRenderPass(commandBuffers[i]);
			vkOk(vkEndCommandBuffer(commandBuffers[i]), "Failed to record command buffer");
		}

	}

	void createCommandPool() {
		QueueFamilyIndicies queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = 0;

		vkOk(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
	}

	void createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size(), VDeleter<VkFramebuffer>(device, vkDestroyFramebuffer));

		for (unsigned int i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = { swapChainImageViews[i] };
			auto frameBufferInfoBuilder = FrameBufferInfoBuilder(renderPass, swapChainExtent, attachments, 1);
			vkOk(vkCreateFramebuffer(device, &frameBufferInfoBuilder.Build(), nullptr, &swapChainFramebuffers[i]), "Failed to create framebuffer");
		}
	}

	void createRenderPass() {
		auto colorAttachment = AttachmentDescriptionBuilder(swapChainImageFormat).Build();
		auto colorAttachmentRef = AttachmentReferenceBuilder().Build();

		auto subPass = SubpassDescriptionBuilder(&colorAttachmentRef).Build();
		auto subpassDependancy = SubpassDependencyBuilder().Build();
		auto renderPassInfo = RenderpassInfoBuilder(&colorAttachment, &subPass, &subpassDependancy).Build();

		vkOk(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass), "Failed to create render pass!");
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size(), VDeleter<VkImageView>{device, vkDestroyImageView});
		for (unsigned int i = 0; i < swapChainImages.size(); i++) {
			auto createInfo = SwapchainImageViewInfoBuilder(swapChainImages[i], swapChainImageFormat).Build();
			vkOk(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]), "Failed to create image view");
		}
	}

	void createSurface() {
		vkOk(glfwCreateWindowSurface(instance, window, nullptr, &surface), "Failed to create window surface!");
	}

	void createSwapChain() {
		auto createInfo = SwapchainInfoKHRBuilder(physicalDevice, surface, width, height)
			.WithOldSwapchain(swapChain)
			->Build();

		VkSwapchainKHR newSwapchain;
		vkOk(vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapchain), "Failed to create the swap chain");
		*&swapChain = newSwapchain;

		vkGetSwapchainImagesKHR(device, swapChain, &createInfo.minImageCount, nullptr);
		swapChainImages.resize(createInfo.minImageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &createInfo.minImageCount, swapChainImages.data());

		swapChainImageFormat = createInfo.imageFormat;
		swapChainExtent = createInfo.imageExtent;
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find any GPUS with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto & device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU!");
		}

	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		bool extensionsSupported = VulkanValidation::checkDeviceExtensionSupport(device, deviceExtensions);
		bool swapChainAdequate = false;

		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader &&
			findQueueFamilies(device, surface).isComplete() &&
			extensionsSupported &&
			swapChainAdequate;
	}

	void createLogicalDevice() {
		QueueFamilyIndicies indices = findQueueFamilies(physicalDevice, surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;

		for (auto queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		vkOk(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device), "Failed to create logical device");
		vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			Update(0.0f);
			Draw(0.0f);
		}

		vkDeviceWaitIdle(device);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void drawFrame() {
		uint32_t imageIndex;
		vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkOk(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "Failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		auto result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			vkOk(result, "Failed to present swap chain image!");
		}
	}

protected:
	GLFWwindow* window;
	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };
	VDeleter<VkDevice> device{ vkDestroyDevice };
	VDeleter<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
	VDeleter<VkShaderModule> vertexShaderModule{ device, vkDestroyShaderModule };
	VDeleter<VkShaderModule> fragmentShaderModule{ device, vkDestroyShaderModule };
	VDeleter<VkPipeline> graphicsPipeline{ device, vkDestroyPipeline };
	VDeleter<VkRenderPass> renderPass{ device, vkDestroyRenderPass };
	VDeleter<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
	VDeleter<VkCommandPool> commandPool{ device, vkDestroyCommandPool };
	VDeleter<VkSemaphore> imageAvailableSemaphore{ device, vkDestroySemaphore };
	VDeleter<VkSemaphore> renderFinishedSemaphore{ device, vkDestroySemaphore };

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;
	std::vector<VDeleter<VkImageView>> swapChainImageViews;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VDeleter<VkDebugReportCallbackEXT> callback{ instance, VulkanDebug::DestroyDebugReportCallbackEXT };
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t width;
	uint32_t height;

	const std::vector<const char *> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};
	const std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
};
