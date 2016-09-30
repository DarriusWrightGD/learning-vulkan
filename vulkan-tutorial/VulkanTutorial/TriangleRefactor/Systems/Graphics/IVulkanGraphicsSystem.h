#pragma once
#include <vulkan\vulkan.h>
#include <vector>
#include<functional>
#include <glm\glm.hpp>

#include "VkDeleter.h"
#include "VkRelease.h"
#include "VulkanDebug.h"
#include "Builders\BufferInfoBuilder.h"


struct VertexBuffer
{
	VkBuffer buffer;
	VkBuffer bufferMemory;
};

struct ShaderStage
{
	ShaderStage(VkShaderStageFlagBits flag, const char * name) : shaderFlag(flag), filename(name) {

	}

	VkShaderStageFlagBits shaderFlag = VK_SHADER_STAGE_VERTEX_BIT;
	const char * filename;
};

class IVulkanGraphicsSystem
{
public:
	IVulkanGraphicsSystem()
	{

	}
	virtual ~IVulkanGraphicsSystem() noexcept = default;

	virtual void Initialize(
		std::function<void(const VkInstance&, VkSurfaceKHR*)> createSurface,
		std::function<void(VkDevice, VkPipelineLayout*, VkPipeline*, VkExtent2D)> createGraphicsPipeline,
		std::function<void(VkCommandBuffer)> createDrawCommands,
		glm::vec2 dimensions) = 0;
	virtual void Draw() = 0;
	virtual void RecreateSwapChain(glm::vec2 dimensions) = 0;
	virtual void SetValidationLayers(std::vector<const char *> layers) = 0;
	virtual void SetDeviceExtensions(std::vector<const char *> extensions) = 0;
	virtual VkShaderModule CreateShaderModule(const char * filename) = 0;
	virtual VkPhysicalDevice GetPhysicalDevice() const = 0;
	virtual void WaitTilIdle() const = 0;
	virtual VertexBuffer CreateBuffer(VkBufferCreateInfo bufferInfo, void * data) = 0;

	virtual VkRenderPass CreateRenderPass() = 0;
	virtual VkRenderPass CreateRenderPass(const VkAttachmentDescription & colorAttachment, const  VkSubpassDescription & subpassDescription, VkSubpassDependency const & subpassDepdency) = 0;
	virtual VkRenderPass CreateRenderPass(VkRenderPassCreateInfo renderPassInfo) = 0;
	virtual void SetRenderpass(VkRenderPass renderPass) = 0;
	virtual VkRenderPass GetRenderPass() const = 0;
	virtual std::vector<VkPipelineShaderStageCreateInfo> CreateShaderStages(const std::vector<ShaderStage> & shaderStages) = 0;
	virtual VkPipeline CreateGraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInput, const std::vector<VkPipelineShaderStageCreateInfo> & shaderStages) = 0;
	virtual void SetGraphicsPipeline(VkPipeline pipeline) = 0;
};


class VulkanGraphicsSystem : public IVulkanGraphicsSystem
{
public:
	VulkanGraphicsSystem()
	{
	}

	~VulkanGraphicsSystem()
	{


		for (auto buffer : vertexBuffers){
			buffer.Release();
		}


		for (auto shaderModule : shaderModules) {
			shaderModule.Release();
		}

		for (auto memoryBuffer : vertexMemoryBuffers){
			memoryBuffer.Release();
		}

		for (auto renderpass : renderpasses) {
			renderpass.Release();
		}

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
	}

	void Initialize(
		std::function<void(const VkInstance&, VkSurfaceKHR*)> createSurface,
		std::function<void(VkDevice, VkPipelineLayout*, VkPipeline*, VkExtent2D)> createGraphicsPipeline,
		std::function<void(VkCommandBuffer)> createDrawCommands,
		glm::vec2 dimensions) override {
		initInstance();

		this->createGraphicsPipeline = createGraphicsPipeline;
		this->createDrawCommands = createDrawCommands;

		VulkanDebug::setupDebugCallback(instance, &callback);
		width = static_cast<uint32_t>(dimensions.x);
		height = static_cast<uint32_t>(dimensions.y);
		createSurface(instance, &surface);
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		currentRenderPass = CreateRenderPass();
		createGraphicsPipeline(device, &pipelineLayout, &graphicsPipeline, swapChainExtent);
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createSemaphores();
	}


	VertexBuffer CreateBuffer(VkBufferCreateInfo bufferInfo, void * data) override {
		vertexBuffers.push_back({ device,vkDestroyBuffer });
		vkOk(vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffers[vertexBuffers.size() - 1]), "Failed to create the vertex buffer");
		auto vertexBuffer = vertexBuffers[vertexBuffers.size() - 1];

		vertexMemoryBuffers.push_back({ device,vkFreeMemory });

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, GetPhysicalDevice());

		vkOk(vkAllocateMemory(device, &allocateInfo, nullptr, &vertexMemoryBuffers[vertexMemoryBuffers.size() - 1]), "Failed to allocate vertex buffer memory");

		vkBindBufferMemory(device, vertexBuffer, vertexMemoryBuffers[vertexMemoryBuffers.size() - 1], 0);
		void * newData;
		vkMapMemory(device, vertexMemoryBuffers[vertexMemoryBuffers.size() - 1], 0, bufferInfo.size, 0, &newData);
		memcpy(newData, data, (size_t)bufferInfo.size);
		vkUnmapMemory(device, vertexMemoryBuffers[vertexMemoryBuffers.size() - 1]);

		auto vertexBufferMemory = vertexMemoryBuffers[vertexMemoryBuffers.size() - 1];
		return {vertexBuffer, vertexBufferMemory};
	}

	virtual VkShaderModule CreateShaderModule(const char * filename) override {
		shaderModules.push_back({ device,vkDestroyShaderModule });
		vkOk(vkCreateShaderModule(device, &ShaderModuleInfoBuilder(readFile(filename)).Build(), nullptr, &shaderModules[shaderModules.size() - 1]));
		return shaderModules[shaderModules.size() - 1];
	}

	virtual VkPhysicalDevice GetPhysicalDevice() const {
		return physicalDevice;
	}

	void Draw() {
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
			RecreateSwapChain(glm::vec2(width,height));
		}
		else if (result != VK_SUCCESS) {
			vkOk(result, "Failed to present swap chain image!");
		}
	}

	void WaitTilIdle()const override{
		vkDeviceWaitIdle(device);
	}
	void SetValidationLayers(std::vector<const char *> layers) override {
		validationLayers = std::move(layers);
	}

	void SetDeviceExtensions(std::vector<const char *> extensions) override{
		deviceExtensions = std::move(extensions);
	}

	void RecreateSwapChain(glm::vec2 dimensions) override{
		width = static_cast<uint32_t>(dimensions.x);
		height = static_cast<uint32_t>(dimensions.y);
		vkDeviceWaitIdle(device);

		createSwapChain();
		createImageViews();
		CreateRenderPass();
		createGraphicsPipeline(device,&pipelineLayout,&graphicsPipeline,swapChainExtent);
		createFramebuffers();
		createCommandBuffers();
	}

	void AddGraphicsPipeline()
	{

	}

	VkRenderPass GetRenderPass() const { return currentRenderPass; }

	VkRenderPass CreateRenderPass(const VkAttachmentDescription & colorAttachment,const  VkSubpassDescription & subpassDescription, VkSubpassDependency const & subpassDepdency) override {
		auto renderPassInfo = RenderpassInfoBuilder(&colorAttachment, &subpassDescription, &subpassDepdency).Build();
		return CreateRenderPass(renderPassInfo);
	}

	VkRenderPass CreateRenderPass(VkRenderPassCreateInfo renderPassInfo) override {
		renderpasses.push_back({ device,vkDestroyRenderPass });
		vkOk(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderpasses[renderpasses.size()-1]), "Failed to create render pass!");
		return renderpasses[renderpasses.size() - 1];
	}

	VkRenderPass CreateRenderPass() override {
		auto colorAttachment = AttachmentDescriptionBuilder(swapChainImageFormat).Build();
		auto colorAttachmentRef = AttachmentReferenceBuilder().Build();
		auto subPass = SubpassDescriptionBuilder(&colorAttachmentRef).Build();
		auto subpassDependancy = SubpassDependencyBuilder().Build();

		return CreateRenderPass(colorAttachment, subPass, subpassDependancy);
	}

	VkPipeline CreateGraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInput, const std::vector<VkPipelineShaderStageCreateInfo> & shaderStages){
		auto viewport = ViewportBuilder(static_cast<float>(width), static_cast<float>(height)).Build();
		auto scissor = ScissorBuilder(swapChainExtent).Build();

		auto viewportStateInfo = ViewportStateBuilder(&viewport, &scissor).Build();
		auto colorBlending = ColorBlendStateBuilder().Build();
		auto pipelineLayoutInfo = PipelineLayoutBuilder().Build();

		VkPipelineLayout layout;
		vkOk(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout), "Failed to create the pipeline layout!");

		auto pipelineInfo = GraphicsPipelineBuilder(shaderStages, &viewportStateInfo,
			&colorBlending, layout, this->GetRenderPass())
			.WithVertexInputState(&vertexInput)
			->Build();

		auto pipeline = CreateGraphicsPipeline(pipelineInfo);
		vkDestroyPipelineLayout(device, layout, nullptr);
		return pipeline;
	}

	VkPipeline CreateGraphicsPipeline(
		VkPipelineVertexInputStateCreateInfo vertexInput,
		const std::vector<VkPipelineShaderStageCreateInfo> & shaderStages,
		VkPipelineViewportStateCreateInfo viewport,
		VkPipelineColorBlendStateCreateInfo colorBlending,
		VkPipelineLayoutCreateInfo pipelineLayoutInfo) {

		VkPipelineLayout layout;
		vkOk(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout), "Failed to create the pipeline layout!");

		auto pipelineInfo = GraphicsPipelineBuilder(shaderStages, &viewport,
			&colorBlending, layout, this->GetRenderPass())
			.WithVertexInputState(&vertexInput)
			->Build();

		auto pipeline = CreateGraphicsPipeline(pipelineInfo);
		vkDestroyPipelineLayout(device, layout, nullptr);

		return pipeline;
	}

	VkPipeline CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo graphicsCreateInfo) {
		VkPipeline pipeline;
		vkOk(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsCreateInfo, nullptr, &pipeline));
		return pipeline;
	}
	
	void SetGraphicsPipeline(VkPipeline pipeline) {
		graphicsPipeline = pipeline;
	}

	std::vector<VkPipelineShaderStageCreateInfo> CreateShaderStages(const std::vector<ShaderStage> & shaderStages) override {
		ShaderStageBuilder shaderStageBuilder;
		for (auto shaderStage : shaderStages) {
			auto module = this->CreateShaderModule(shaderStage.filename);
			shaderStageBuilder.AddStage(shaderStage.shaderFlag, module);
		}

		return shaderStageBuilder.BuildStages();
	}

	void SetRenderpass(VkRenderPass renderPass) override {
		currentRenderPass = renderPass;
	}

private:
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
			renderPassInfo.renderPass = currentRenderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];

			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			auto commandBuffer = commandBuffers[i];
			createDrawCommands(commandBuffer);
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
			auto frameBufferInfoBuilder = FrameBufferInfoBuilder(currentRenderPass, swapChainExtent, attachments, 1);
			vkOk(vkCreateFramebuffer(device, &frameBufferInfoBuilder.Build(), nullptr, &swapChainFramebuffers[i]), "Failed to create framebuffer");
		}
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size(), VDeleter<VkImageView>{device, vkDestroyImageView});
		for (unsigned int i = 0; i < swapChainImages.size(); i++) {
			auto createInfo = SwapchainImageViewInfoBuilder(swapChainImages[i], swapChainImageFormat).Build();
			vkOk(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]), "Failed to create image view");
		}
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

	std::function<void(VkDevice, VkPipelineLayout*, VkPipeline*, VkExtent2D)> createGraphicsPipeline;
	std::function<void(VkCommandBuffer)> createDrawCommands;
	uint32_t width;
	uint32_t height;
	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };
	VDeleter<VkDevice> device{ vkDestroyDevice };
	VDeleter<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };

	VkPipeline graphicsPipeline;

	VkRenderPass currentRenderPass;
	std::vector<VRelease<VkRenderPass>> renderpasses;

	VDeleter<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
	VDeleter<VkCommandPool> commandPool{ device, vkDestroyCommandPool };
	VDeleter<VkSemaphore> imageAvailableSemaphore{ device, vkDestroySemaphore };
	VDeleter<VkSemaphore> renderFinishedSemaphore{ device, vkDestroySemaphore };

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;
	std::vector<VDeleter<VkImageView>> swapChainImageViews;
	std::vector<VRelease<VkBuffer>> vertexBuffers;
	std::vector<VRelease<VkBuffer>> vertexMemoryBuffers;
	std::vector<VRelease<VkShaderModule>> shaderModules;


	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VDeleter<VkDebugReportCallbackEXT> callback{ instance, VulkanDebug::DestroyDebugReportCallbackEXT };
	std::vector<const char *> validationLayers;
	std::vector<const char *> deviceExtensions;
};