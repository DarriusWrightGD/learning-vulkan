#pragma once

#include "VulkanApplication.h"
#include <glm\glm.hpp>
#include <array>
#include "..\Data\Vertex.h"
#include "..\Builders\BufferInfoBuilder.h"
using namespace std;

class HelloTriangle : public VulkanApplication {
public:
	HelloTriangle(shared_ptr<IVulkanGraphicsSystem> graphicsSystem): VulkanApplication(graphicsSystem)
	{

	}

	~HelloTriangle()
	{

	}
protected:
	virtual void CreateGraphicsPipeline(VkDevice device, VkPipelineLayout * pipelineLayout, VkPipeline * pipeline, VkExtent2D swapChainExtent) override {
		auto shaderStages = graphicsSystem->CreateShaderStages({
			ShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "shaders/shader.vert.spv"),
			ShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/shader.frag.spv")
		});
		
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		auto vertexInput = VertexInputBuilder()
			.WithBindings(&bindingDescription, 1)
			->WithAttributes(attributeDescriptions.data(), attributeDescriptions.size())
			->Build();

		graphicsSystem->SetGraphicsPipeline(graphicsSystem->CreateGraphicsPipeline(vertexInput, shaderStages));
	}

	virtual void CreateDrawCommands(VkCommandBuffer commandBuffer) override {
		CreateVertexBuffer();
		VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);
	}

	void CreateVertexBuffer() {
		if (created)
		{
			auto bufferInfo = BufferInfoBuilder(sizeof(vertices[0]) * vertices.size()).Build();
			vertexBuffer = graphicsSystem->CreateBuffer(bufferInfo, vertices.data());
			created = false;
		}
	}

private:
	VertexBuffer vertexBuffer;
	bool created = true;
	std::vector<Vertex> vertices = {
		{ { 0.0f, -0.5f, 0.0f },{ 1.0f, 0.0f, 1.0f } },
		{ { 0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f, 0.0f },{ 0.0f, 1.0f, 1.0f } }
	};
};