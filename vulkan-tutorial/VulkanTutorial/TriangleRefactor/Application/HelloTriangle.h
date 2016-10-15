#pragma once
#define GLM_FORCE_RADIANS
#include "VulkanApplication.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <array>
#include "..\Data\Vertex.h"
#include "..\Builders\BufferInfoBuilder.h"
#include <chrono>
using namespace std;

/// <summary>
/// Mountain
/// Trees 
/// Lake in the center 
/// Sky (clear)
/// </summary>

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class HelloTriangle : public VulkanApplication {
public:
	HelloTriangle(shared_ptr<IVulkanGraphicsSystem> graphicsSystem) : VulkanApplication(graphicsSystem)
	{

	}

	~HelloTriangle()
	{

	}
protected:
	virtual void Update() override {
		updateUniformBuffer();
	}

	virtual void CreateGraphicsPipeline(VkDevice device) override {
		auto shaderStages = graphicsSystem->CreateShaderStages({
			ShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "shaders/uniforms/uniforms.vert.spv"),
			ShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/uniforms/uniforms.frag.spv"),
		});

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		auto vertexInput = VertexInputBuilder()
			.WithBindings(&bindingDescription, 1)
			->WithAttributes(attributeDescriptions.data(), attributeDescriptions.size())
			->Build();

		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;


		vkOk(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

		VkDescriptorSetLayout setLayouts[] = { descriptorSetLayout };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = PipelineLayoutBuilder(1, setLayouts)
			.Build();

		auto graphicsPipeline = graphicsSystem->StartGraphicsPipeline(vertexInput, shaderStages)
			->WithPipelineLayout(pipelineLayoutInfo)
			->Create();

		graphicsSystem->SetGraphicsPipeline(graphicsPipeline.pipeline);

		//auto graphicsPipeline = graphicsSystem->CreateGraphicsPipeline(vertexInput, shaderStages, pipelineLayoutInfo);
		//graphicsSystem->SetGraphicsPipeline(graphicsPipeline);
	}

	virtual void CreateDrawCommands(VkCommandBuffer commandBuffer) override {
		VkBuffer vertexBuffers[] = { vertexBuffer.mainBuffer.buffer , uniformBuffer.mainBuffer.buffer};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer.mainBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
		auto layout = graphicsSystem->GetPipelineLayout();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
		vkCmdDrawIndexed(commandBuffer,indices.size(), 1, 0, 0, 0);
	}

	virtual void CreateBuffers() override {
		auto vertexBufferSize = sizeof(vertices[0]) * vertices.size();
		auto indexBufferSize = sizeof(indices[0]) * indices.size();
		auto uniformBufferSize = sizeof(UniformBufferObject);

		vertexBuffer = graphicsSystem->MapToLocalMemory(vertexBufferSize, vertices.data());
		indexBuffer = graphicsSystem->MapToLocalMemory(indexBufferSize, indices.data());
		uniformBuffer = graphicsSystem->MapToLocalMemory(uniformBufferSize, &ubo, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
		VkDescriptorPoolSize poolSize = {};
		poolSize.descriptorCount = 1;
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		poolInfo.maxSets = 1;
		vkOk(vkCreateDescriptorPool(graphicsSystem->GetDevice(), &poolInfo, nullptr, &descriptorPool));

		VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		vkOk(vkAllocateDescriptorSets(graphicsSystem->GetDevice(), &allocInfo, &descriptorSet));

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffer.mainBuffer.buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(graphicsSystem->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	

private:
	void updateUniformBuffer() {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
		ubo = {};
		ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.projection = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		
		ubo.projection[1][1] *= -1;

		graphicsSystem->MapToLocalMemory(uniformBuffer, &ubo);
	}

	TransferBuffer vertexBuffer;
	TransferBuffer indexBuffer;
	TransferBuffer uniformBuffer;

	uint32_t verticesCount;
	uint32_t indiicesCount;
	UniformBufferObject ubo;

	VDeleter<VkDescriptorSetLayout> descriptorSetLayout{graphicsSystem->GetDevice(), vkDestroyDescriptorSetLayout };
	VDeleter<VkDescriptorPool> descriptorPool{ graphicsSystem->GetDevice(), vkDestroyDescriptorPool };
	VkDescriptorSet descriptorSet;
	std::vector<Vertex> vertices = {
		{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 1.0f } },
		{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f } }
	};

	std::vector<uint16_t> indices = {
		0,1,2,2,3,0
	};
};