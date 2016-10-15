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
#include <memory>

struct GraphicsPipeline
{
	VkPipeline pipeline;
	std::string id;
};

class GraphicsPipelineCreator {
public:
	void Cleanup();
	void Initialize(const VRelease<VkDevice> & device, const VkExtent2D & swapChainExtent, const glm::vec2 & dimensions);
	GraphicsPipelineCreator * StartGraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInput, const std::vector<VkPipelineShaderStageCreateInfo> & shaderStages);
	GraphicsPipelineCreator* WithPipelineLayout(VkPipelineLayoutCreateInfo pipelineLayoutInfo);
	GraphicsPipelineCreator* WithVertexInputState(VkPipelineVertexInputStateCreateInfo inputState);
	GraphicsPipelineCreator* WithInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo inputState);
	GraphicsPipelineCreator* WithMultisampleState(VkPipelineMultisampleStateCreateInfo inputState);
	GraphicsPipelineCreator* WithRasterizationState(VkPipelineRasterizationStateCreateInfo inputState);
	GraphicsPipelineCreator* WithDepthStencilState(VkPipelineDepthStencilStateCreateInfo inputState);
	GraphicsPipelineCreator* WithDynamicState(VkPipelineDynamicStateCreateInfo inputState);
	GraphicsPipelineCreator* WithSubpass(uint32_t subpass);
	GraphicsPipelineCreator* WithBasePipeline(VkPipeline pipelineHandle, uint32_t pipelineIndex);
	GraphicsPipeline Create();

	void SetDimensions(glm::vec2 dimensions);

	void SetSwapchainExtent(VkExtent2D extent);

	void SetPipelineLayout(VkPipelineLayout layout);

	VkRenderPass GetRenderPass();

	VkPipelineLayout GetPipelineLayout();

	void SetRenderpass(VkRenderPass renderPass);

private:
	void DestoryPipelineLayout();

	std::unique_ptr<VkViewport> currentViewPort = std::unique_ptr<VkViewport>(new VkViewport);
	std::unique_ptr<VkRect2D> currentScissors = std::unique_ptr<VkRect2D>(new VkRect2D);


	VRelease<VkDevice> device;
	VkPipelineLayout pipelineLayout = {};

	VkRenderPass currentRenderPass;
	VkPipelineColorBlendStateCreateInfo colorBlending = ColorBlendStateBuilder().Build();
	std::map<std::string, VkPipeline> pipelines;
	std::string currentPipelineId;
	std::unique_ptr<GraphicsPipelineBuilder> currentPipelineBuilder;
	glm::vec2 dimensions;
	VkExtent2D swapChainExtent;

	static char genRandom();
	static std::string generateId(unsigned int length = 10);
};