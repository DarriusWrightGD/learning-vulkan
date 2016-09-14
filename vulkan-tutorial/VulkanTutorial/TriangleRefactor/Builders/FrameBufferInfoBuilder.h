#pragma once
#include <vulkan\vulkan.h>

class FrameBufferInfoBuilder {
public:
	FrameBufferInfoBuilder(VkRenderPass renderPass,VkExtent2D dimensions, VkImageView * attachments, uint32_t attachmentCount)
	:renderPass(renderPass), width(dimensions.width), height(dimensions.height), attachments(attachments), attachmentCount(attachmentCount){

	}

	FrameBufferInfoBuilder * WithLayerCount(uint32_t layerCount) {
		layers = layerCount;
	}

	VkFramebufferCreateInfo Build() {
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = type;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachmentCount;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.height = height;
		framebufferInfo.width = width;
		framebufferInfo.layers = layers;

		return framebufferInfo;
	}
private:
	const VkStructureType type = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	uint32_t layers = 1;
	uint32_t attachmentCount = 1;
	uint32_t height;
	uint32_t width;
	VkImageView * attachments;
	VkRenderPass renderPass;
};