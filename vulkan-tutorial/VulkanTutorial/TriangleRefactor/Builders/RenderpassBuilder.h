#pragma once

#include <vulkan\vulkan.h>

class AttachmentDescriptionBuilder
{
public:
	AttachmentDescriptionBuilder(VkFormat imageFormat) : attachmentImageFormat(imageFormat) {

	}

	AttachmentDescriptionBuilder* WithSampleFlags(VkSampleCountFlagBits flags) {
		samples = flags;
		return this;
	}

	AttachmentDescriptionBuilder* WithLoadOperation(VkAttachmentLoadOp operation) {
		loadOp = operation;
		return this;
	}

	AttachmentDescriptionBuilder* WithStoreOperation(VkAttachmentStoreOp operation) {
		storeOp = operation;
		return this;
	}

	AttachmentDescriptionBuilder* WithStencilLoadOperation(VkAttachmentLoadOp operation) {
		stencilLoadOp = operation;
		return this;
	}

	AttachmentDescriptionBuilder* WithStencilStoreOperation(VkAttachmentStoreOp operation) {
		stencilStoreOp = operation;
		return this;
	}

	AttachmentDescriptionBuilder* WithInitialImageLayout(VkImageLayout imageLayout) {
		initialLayout = imageLayout;
		return this;
	}

	AttachmentDescriptionBuilder* WithFinalImageLayout(VkImageLayout imageLayout) {
		finalLayout = imageLayout;
		return this;
	}

	VkAttachmentDescription Build()
	{
		VkAttachmentDescription attachment = {};
		attachment.format = attachmentImageFormat;
		attachment.samples = samples;
		attachment.loadOp = loadOp;
		attachment.storeOp = storeOp;
		attachment.stencilLoadOp = stencilLoadOp;
		attachment.stencilStoreOp = stencilStoreOp;
		attachment.initialLayout = initialLayout;
		attachment.finalLayout = finalLayout;

		return attachment;
	}
private:
	VkFormat attachmentImageFormat; 
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
};

class AttachmentReferenceBuilder
{
public:
	AttachmentReferenceBuilder* WithAttachmentIndex(uint32_t index) {
		attachment = index;
		return this;
	}

	AttachmentReferenceBuilder* WithImageLayout(VkImageLayout imageLayout) {
		layout = imageLayout;
		return this;
	}

	VkAttachmentReference Build(){
		VkAttachmentReference attachmentRef = {};
		attachmentRef.attachment = attachment;
		attachmentRef.layout = layout;
		return attachmentRef;
	}
private:
	uint32_t attachment = 0;
	VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
};

class SubpassDescriptionBuilder
{
public:
	SubpassDescriptionBuilder(const VkAttachmentReference * colorAttachments, uint32_t attachmentCount = 1)
		:colorAttachments(colorAttachments), colorAttachmentCount(attachmentCount)
	{

	}

	SubpassDescriptionBuilder* WithPipelineBindPoint(VkPipelineBindPoint bindPoint)
	{
		pipelineBindPoint = bindPoint;
		return this;
	}

	VkSubpassDescription Build()
	{
		VkSubpassDescription subPass = {};
		subPass.pipelineBindPoint = pipelineBindPoint;
		subPass.colorAttachmentCount = colorAttachmentCount;
		subPass.pColorAttachments = colorAttachments;
		return subPass;
	}
private:
	const VkAttachmentReference * colorAttachments;
	uint32_t colorAttachmentCount = 1;
	VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
};


class SubpassDependencyBuilder
{
public:
	SubpassDependencyBuilder* WithSrcSubpass(uint32_t src)
	{
		srcSubpass = src;
		return this;
	}

	SubpassDependencyBuilder* WithDstSubpass(uint32_t dst)
	{
		dstSubpass = dst;
		return this;
	}

	SubpassDependencyBuilder* WithSrcStageMask(VkPipelineStageFlags flags)
	{
		srcStageMask = flags;
		return this;
	}

	SubpassDependencyBuilder* WithDstStageMask(VkPipelineStageFlags flags)
	{
		dstStageMask = flags;
		return this;
	}

	SubpassDependencyBuilder* WithSrcAccessMask(VkAccessFlags flags)
	{
		srcAccessMask = flags;
		return this;
	}

	SubpassDependencyBuilder* WithDstAccessMask(VkAccessFlags flags)
	{
		dstAccessMask = flags;
		return this;
	}

	VkSubpassDependency Build()
	{
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = srcSubpass;
		dependency.dstSubpass = dstSubpass;

		dependency.srcStageMask = srcStageMask;
		dependency.srcAccessMask = srcAccessMask;
		dependency.dstStageMask = dstStageMask;
		dependency.dstAccessMask =  dstAccessMask;
		return dependency;
	}

private:
	uint32_t srcSubpass = VK_SUBPASS_EXTERNAL;
	uint32_t dstSubpass = 0;

	VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkAccessFlags srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

};

class RenderpassInfoBuilder
{
public:
	RenderpassInfoBuilder(const VkAttachmentDescription * attachments, const VkSubpassDescription * subpasses, const VkSubpassDependency * subpassDependencies)
	: attachments(attachments), subpasses(subpasses), subpassDependencies(subpassDependencies){

	}

	RenderpassInfoBuilder* WithAttachmentCount(uint32_t count)
	{
		attachmentCount = count;
		return this;
	}

	RenderpassInfoBuilder* WithSubpassCount(uint32_t count)
	{
		subpassCount = count;
		return this;
	}

	RenderpassInfoBuilder* WithSubpassDepdencyCount(uint32_t count)
	{
		dependencyCount = count;
		return this;
	}


	VkRenderPassCreateInfo Build()
	{
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentCount;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = subpassCount;
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = dependencyCount;
		renderPassInfo.pDependencies = subpassDependencies;
		return renderPassInfo;
	}
private:
	const VkAttachmentDescription * attachments;
	uint32_t attachmentCount = 1;
	const VkSubpassDescription * subpasses;
	uint32_t subpassCount = 1;
	const VkSubpassDependency * subpassDependencies;
	uint32_t dependencyCount = 1;
};