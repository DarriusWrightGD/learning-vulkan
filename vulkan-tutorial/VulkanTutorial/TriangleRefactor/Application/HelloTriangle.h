#pragma once

#include "VulkanApplication.h"
#include <glm\glm.hpp>
#include <array>
#include "..\Data\Vertex.h"

class HelloTriangle : public VulkanApplication {
protected:
	virtual void CreateGraphicsPipeline() override {
		vkOk(vkCreateShaderModule(device, &ShaderModuleInfoBuilder(readFile("shaders/shader.vert.spv")).Build(), nullptr, &vertexShaderModule));
		vkOk(vkCreateShaderModule(device, &ShaderModuleInfoBuilder(readFile("shaders/shader.frag.spv")).Build(), nullptr, &fragmentShaderModule));

		auto shaderBuilder = ShaderStageBuilder();
		auto shaderStages = shaderBuilder
			.AddVertexShader(vertexShaderModule)
			->AddFragmentShader(fragmentShaderModule)
			->BuildStages();

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		auto vertexInput = VertexInputBuilder()
			.WithBindings(&bindingDescription, 1)
			->WithAttributes(attributeDescriptions.data(), attributeDescriptions.size())
			->Build();

		auto viewport = ViewportBuilder(static_cast<float>(width), static_cast<float>(height)).Build();
		auto scissor = ScissorBuilder(swapChainExtent).Build();

		auto viewportStateInfo = ViewportStateBuilder(&viewport, &scissor).Build();
		auto colorBlending = ColorBlendStateBuilder().Build();
		auto pipelineLayoutInfo = PipelineLayoutBuilder().Build();

		vkOk(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed to create the pipeline layout!");

		auto pipelineInfo = GraphicsPipelineBuilder(shaderStages, 2, &viewportStateInfo,
			&colorBlending, pipelineLayout, renderPass)
			.WithVertexInputState(&vertexInput)
			->Build();

		vkOk(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));
	}

	virtual void CreateDrawCommands(VkCommandBuffer commandBuffer) override {
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);
	}

	virtual void CreateVertexBuffer() override {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(vertices[0]) * vertices.size();
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		vkOk(vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer), "Failed to create the vertex buffer");

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkOk(vkAllocateMemory(device, &allocateInfo, nullptr, &vertexBufferMemory), "Failed to allocate vertex buffer memory");

		vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
		void * data;
		vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferInfo.size);
		vkUnmapMemory(device, vertexBufferMemory);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find a suitable memory type!");
	}

private:
	VDeleter<VkBuffer> vertexBuffer{ device, vkDestroyBuffer };
	VDeleter<VkBuffer> vertexBufferMemory{ device, vkFreeMemory };

	std::vector<Vertex> vertices = {
		{ { 0.0f, -0.5f, 0.0f },{ 1.0f, 0.0f, 1.0f } },
		{ { 0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f, 0.0f },{ 0.0f, 1.0f, 1.0f } }
	};
};