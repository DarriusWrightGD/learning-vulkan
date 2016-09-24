#pragma once
#include <vulkan\vulkan.h>

class SwapchainImageViewInfoBuilder
{
public:
	SwapchainImageViewInfoBuilder(VkImage image, VkFormat format) : swapchainImage{ image }, swapchainFormat{format} {

	}

	SwapchainImageViewInfoBuilder* WithViewType(VkImageViewType type){
		viewType = type;
		return this;
	}

	SwapchainImageViewInfoBuilder* WithComponents(VkComponentMapping components) {
		componentMapping = components;
		return this;
	}

	SwapchainImageViewInfoBuilder* WithComponents(VkImageSubresourceRange resourceRange) {
		subresourceRange = resourceRange;
		return this;
	}


	VkImageViewCreateInfo Build()
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImage;
		createInfo.viewType = viewType;
		createInfo.format = swapchainFormat;
		createInfo.components = componentMapping;
		createInfo.subresourceRange = subresourceRange;
		return createInfo;
	}
private:
	VkImage swapchainImage;
	VkFormat swapchainFormat;
	VkComponentMapping componentMapping{ VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY };
	VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , 0,1,0,1 };
	VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
};