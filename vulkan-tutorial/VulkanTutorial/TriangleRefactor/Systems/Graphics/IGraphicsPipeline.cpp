#pragma once
#include "IGraphicsPipeline.h"
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
static const char alphanum[] =
"0123456789"
"!@#$%^&*"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";


static const int stringLength = sizeof(alphanum) - 1;

void GraphicsPipelineCreator::Initialize(const VRelease<VkDevice> & device, const VkExtent2D & swapChainExtent, const glm::vec2 & dimensions) {
	this->device = device;
	this->swapChainExtent = swapChainExtent;
	this->dimensions = dimensions;
}


GraphicsPipelineCreator * GraphicsPipelineCreator::StartGraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInput, const std::vector<VkPipelineShaderStageCreateInfo> & shaderStages) {
	currentPipelineId = generateId();


	*currentViewPort = ViewportBuilder(dimensions.x, dimensions.y).Build();
	*currentScissors = ScissorBuilder(swapChainExtent).Build();

	auto pipelineLayoutInfo = PipelineLayoutBuilder().Build();
	auto viewportStateInfo = ViewportStateBuilder(currentViewPort.get(), currentScissors.get()).Build();


	auto rasterizerState = RasterizationStateBuilder()
		.WithCounterClockwiseFace()
		->WithBackCulling()
		->Build();

	DestoryPipelineLayout();
	vkOk(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed to create the pipeline layout!");

	this->currentPipelineBuilder = std::unique_ptr<GraphicsPipelineBuilder>(new GraphicsPipelineBuilder(shaderStages, viewportStateInfo, colorBlending, pipelineLayout, currentRenderPass));

	return this->WithVertexInputState(vertexInput)
		->WithRasterizationState(rasterizerState);
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithPipelineLayout(VkPipelineLayoutCreateInfo pipelineLayoutInfo) {
	DestoryPipelineLayout();
	vkOk(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed to create the pipeline layout!");
	currentPipelineBuilder->WithPipelineLayout(pipelineLayout);
	return this;
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithVertexInputState(VkPipelineVertexInputStateCreateInfo inputState) {
	currentPipelineBuilder->WithVertexInputState(inputState);
	return this;
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo inputState) {
	currentPipelineBuilder->WithInputAssemblyState(inputState);
	return this;
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithMultisampleState(VkPipelineMultisampleStateCreateInfo inputState) {
	currentPipelineBuilder->WithMultisampleState(inputState);
	return this;
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithRasterizationState(VkPipelineRasterizationStateCreateInfo inputState) {
	currentPipelineBuilder->WithRasterizationState(inputState);
	return this;
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithDepthStencilState(VkPipelineDepthStencilStateCreateInfo inputState) {
	currentPipelineBuilder->WithDepthStencilState(inputState);
	return this;
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithDynamicState(VkPipelineDynamicStateCreateInfo inputState) {
	currentPipelineBuilder->WithDynamicState(inputState);
	return this;
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithSubpass(uint32_t subpass) {
	currentPipelineBuilder->WithSubpass(subpass);
	return this;
}

GraphicsPipelineCreator* GraphicsPipelineCreator::WithBasePipeline(VkPipeline pipelineHandle, uint32_t pipelineIndex) {
	currentPipelineBuilder->WithBasePipeline(pipelineHandle, pipelineIndex);
	return this;
}


GraphicsPipeline GraphicsPipelineCreator::Create() {
	GraphicsPipeline pipeline = {};
	pipeline.id = currentPipelineId;
	auto pipelineInfo = currentPipelineBuilder->Build();
	vkOk(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline));
	return pipeline;
}

void GraphicsPipelineCreator::SetDimensions(glm::vec2 dimensions) {
	this->dimensions = dimensions;
}

void GraphicsPipelineCreator::SetSwapchainExtent(VkExtent2D extent) {
	this->swapChainExtent = extent;
}

void GraphicsPipelineCreator::SetPipelineLayout(VkPipelineLayout layout) {
	pipelineLayout = layout;
}

VkRenderPass GraphicsPipelineCreator::GetRenderPass() {
	return currentRenderPass;
}

VkPipelineLayout GraphicsPipelineCreator::GetPipelineLayout() {
	return pipelineLayout;
}

void GraphicsPipelineCreator::SetRenderpass(VkRenderPass renderPass) {
	currentRenderPass = renderPass;
}

void GraphicsPipelineCreator::DestoryPipelineLayout() {
	if (pipelineLayout) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}
}


char GraphicsPipelineCreator::genRandom() {
	return alphanum[rand() % stringLength];
}

std::string GraphicsPipelineCreator::generateId(unsigned int length) {
	std::string id = "";
	for (size_t i = 0; i < length; i++)
	{
		id += genRandom();
	}

	return id;
}

void GraphicsPipelineCreator::Cleanup() {
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
