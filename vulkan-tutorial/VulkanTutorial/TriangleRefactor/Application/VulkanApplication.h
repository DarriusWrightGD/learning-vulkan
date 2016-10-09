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

#include "VkDeleter.h"
#include "VulkanDebug.h"
#include "VulkanValidation.h"
#include "Exception.h"
#include "FileReader.h"
#include "Builders\InstanceBuilder.h"
#include "Builders\FrameBufferInfoBuilder.h"
#include "Builders\GraphicsPipelineBuilder.h"
#include "Builders\RenderpassBuilder.h"
#include "Builders\SwapchainInfoBuilder.h"

using std::cout;
using std::endl;
using std::set;
#include "..\Systems\Graphics\IVulkanGraphicsSystem.h"
#include <memory>

class VulkanApplication {
public:
	VulkanApplication(std::shared_ptr<IVulkanGraphicsSystem> graphicsSystem, uint32_t width = 800, uint32_t height = 600) : width(width), height(height), graphicsSystem(graphicsSystem) {

	}
	virtual void OnInit() { }
	virtual void OnResize(int width, int height) { }
	virtual void Update() { }

	void run() {
		initVulkan();
		OnInit();
		mainLoop();
	}

protected:
	/// <summary>
	/// This is used to describe the pipeline(s) for the current running application.
	/// </summary>
	virtual void CreateGraphicsPipeline(VkDevice device) = 0;
	/// <summary>
	/// This will be called once, and will be used to create the draw calls that will be called on each frame.
	/// </summary>
	/// <param name="commandBuffer"></param>
	virtual void CreateDrawCommands(VkCommandBuffer commandBuffer) = 0;
	/// <summary>
	/// Used to initialize all of the vertex buffers that are needed for viewing in the application.
	/// </summary>
	virtual void CreateBuffers() {};
	std::shared_ptr<IVulkanGraphicsSystem> graphicsSystem;
	GLFWwindow* window;
	uint32_t width;
	uint32_t height;

	const std::vector<const char *> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation",
		//"VK_LAYER_LUNARG_api_dump"
	};
	const std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
private:
	static void onWindowResized(GLFWwindow * window, int width, int height) {
		if (width == 0 || height == 0) return;

		auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
		app->setWidth(width);
		app->setHeight(height);
		app->recreateSwapChain();
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
		graphicsSystem->RecreateSwapChain(glm::vec2(width, height));
	}

	void initVulkan() {
		initWindow();
		graphicsSystem->SetValidationLayers(validationLayers);
		graphicsSystem->SetDeviceExtensions(deviceExtensions);
		graphicsSystem->Initialize([this](const VkInstance & instance, VkSurfaceKHR * surface) { createSurface(instance, surface); },
			[this](VkDevice device) { return CreateGraphicsPipeline(device); },
			[this](VkCommandBuffer commandBuffer) {CreateDrawCommands(commandBuffer); },
			[this]() {CreateBuffers(); },
		glm::vec2(width,height));
		OnInit();
	}

	void createSurface(const VkInstance & instance, VkSurfaceKHR * surface) {
		vkOk(glfwCreateWindowSurface(instance, window, nullptr, surface), "Failed to create window surface!");
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			Update();
			graphicsSystem->Draw();
		}

		graphicsSystem->WaitUntilDeviceIdle();
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};
