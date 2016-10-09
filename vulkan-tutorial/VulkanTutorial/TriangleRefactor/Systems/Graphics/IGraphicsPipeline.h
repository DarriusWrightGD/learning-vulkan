#pragma once
#include <vulkan\vulkan.h>
#include <map>
#include <vector>
#include <string>
#include <glm\glm.hpp>
#include <Builders\GraphicsPipelineBuilder.h>
#include <VkRelease.h>
#include <VkDeleter.h>
#include <Exception.h>
static const char alphanum[] =
"0123456789"
"!@#$%^&*"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

static const int stringLength = sizeof(alphanum) - 1;
struct GraphicsPipeline
{
	VkPipeline pipeline;
	std::string id;
};

class GraphicsPipelineCreator {
public:
	GraphicsPipelineCreator()
	{
	}
	
	~GraphicsPipelineCreator()
	{
		CleanupBuilder();
	}

	void Initialize(VDeleter<VkDevice> device, VkExtent2D swapChainExtent, glm::vec2 dimensions) {
		this->device = device;
		this->swapChainExtent = swapChainExtent;
		this->dimensions = dimensions;
	}

	GraphicsPipelineCreator * StartGraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInput, const std::vector<VkPipelineShaderStageCreateInfo> & shaderStages) {
		currentPipelineId = generateId();
		CleanupBuilder();

		auto viewport = ViewportBuilder(dimensions.x, dimensions.y).Build();
		auto scissor = ScissorBuilder(swapChainExtent).Build();

		auto pipelineLayoutInfo = PipelineLayoutBuilder().Build();
		auto viewportStateInfo = ViewportStateBuilder(&viewport, &scissor).Build();
		auto colorBlending = ColorBlendStateBuilder().Build();

		auto rasterizerState = RasterizationStateBuilder()
			.WithCounterClockwiseFace()
			->WithBackCulling()
			->Build();

		vkOk(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed to create the pipeline layout!");

		currentPipelineBuilder = new GraphicsPipelineBuilder(shaderStages, viewportStateInfo, colorBlending, pipelineLayout, currentRenderPass);
	
		return this->WithVertexInputState(vertexInput)
			   ->WithRasterizationState(rasterizerState);
	}

	GraphicsPipelineCreator* WithPipelineLayout(VkPipelineLayoutCreateInfo pipelineLayoutInfo) {
		vkOk(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed to create the pipeline layout!");
		currentPipelineBuilder->WithPipelineLayout(pipelineLayout);
		return this;
	}

	GraphicsPipelineCreator* WithVertexInputState(VkPipelineVertexInputStateCreateInfo inputState) {
		currentPipelineBuilder->WithVertexInputState(inputState);
		return this;
	}

	GraphicsPipelineCreator* WithInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo inputState) {
		currentPipelineBuilder->WithInputAssemblyState(inputState);
		return this;
	}

	GraphicsPipelineCreator* WithMultisampleState(VkPipelineMultisampleStateCreateInfo inputState) {
		currentPipelineBuilder->WithMultisampleState(inputState);
		return this;
	}

	GraphicsPipelineCreator* WithRasterizationState(VkPipelineRasterizationStateCreateInfo inputState) {
		currentPipelineBuilder->WithRasterizationState(inputState);
		return this;
	}

	GraphicsPipelineCreator* WithDepthStencilState(VkPipelineDepthStencilStateCreateInfo inputState) {
		currentPipelineBuilder->WithDepthStencilState(inputState);
		return this;
	}

	GraphicsPipelineCreator* WithDynamicState(VkPipelineDynamicStateCreateInfo inputState) {
		currentPipelineBuilder->WithDynamicState(inputState);
		return this;
	}

	GraphicsPipelineCreator* WithSubpass(uint32_t subpass) {
		currentPipelineBuilder->WithSubpass(subpass);
		return this;
	}

	GraphicsPipelineCreator* WithBasePipeline(VkPipeline pipelineHandle, uint32_t pipelineIndex) {
		currentPipelineBuilder->WithBasePipeline(pipelineHandle, pipelineIndex);
		return this;
	}


	GraphicsPipeline Create() {
		GraphicsPipeline pipeline;
		pipeline.id = currentPipelineId;
		vkOk(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &currentPipelineBuilder->Build(), nullptr, &pipeline.pipeline));
		return pipeline;
	}

	void SetDimensions(glm::vec2 dimensions) {
		this->dimensions = dimensions;
	}

	void SetSwapchainExtent(VkExtent2D extent) {
		this->swapChainExtent = extent;
	}

	VkRenderPass GetRenderPass() {
		return currentRenderPass;
	}

	void SetRenderpass(VkRenderPass renderPass) {
		currentRenderPass = renderPass;
	}



private:
	VDeleter<VkDevice> device;
	VDeleter<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };

	VkRenderPass currentRenderPass;
	std::map<std::string, VkPipeline> pipelines;
	std::string currentPipelineId;
	GraphicsPipelineBuilder * currentPipelineBuilder = nullptr;
	glm::vec2 dimensions;
	VkExtent2D swapChainExtent;

	static char genRandom(){
		return alphanum[rand() % stringLength];
	}

	static std::string generateId(unsigned int length = 10) {
		std::string id = "";
		for (size_t i = 0; i < length; i++)
		{
			id += genRandom();
		}

		return id;
	}

	void CleanupBuilder()
	{
		if (currentPipelineBuilder != nullptr) {
			currentPipelineBuilder = nullptr;
			delete currentPipelineBuilder;
		}
	}
};