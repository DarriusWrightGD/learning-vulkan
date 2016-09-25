#pragma once
#include <vulkan\vulkan.h>

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndicies {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};



SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModes, nullptr);

	if (presentModes != 0) {
		details.presentModes.resize(presentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModes, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> presentModes) {
	for (const auto & presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities, uint32_t width, uint32_t height) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { width,height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndicies indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	auto i = 0;
	for (const auto & queueFamily : queueFamilies) {

		if (queueFamily.queueCount > 0) {

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			if (presentSupport) {
				indices.presentFamily = i;
			}
		}

		if (indices.isComplete()) {
			break;
		}
		i++;
	}

	return indices;
}


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

class SwapchainInfoKHRBuilder {
public:
	SwapchainInfoKHRBuilder(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height)
	:surface(surface){
		auto swapChainSupport = querySwapChainSupport(physicalDevice, surface);
		surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		extent = chooseSwapExtent(swapChainSupport.capabilities, width, height);
		
		imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		QueueFamilyIndicies indices = findQueueFamilies(physicalDevice, surface);
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

		if (indices.graphicsFamily != indices.presentFamily) {
			imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			queueFamilyIndexCount = 2;
			this->queueFamilyIndices = queueFamilyIndices;
		}
		else {
			imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			queueFamilyIndexCount = 0;
			this->queueFamilyIndices = nullptr;
		}
		preTransform = swapChainSupport.capabilities.currentTransform;
		this->presentMode = presentMode;
	}

	SwapchainInfoKHRBuilder* WithOldSwapchain(VkSwapchainKHR oldSwapchain) {
		this->oldSwapchain = oldSwapchain;
		return this;
	}

	VkSwapchainCreateInfoKHR Build() {
		VkSwapchainCreateInfoKHR createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = imageArrayLayers;
		createInfo.imageUsage = imageUseage;
		createInfo.imageSharingMode = imageSharingMode;
		createInfo.queueFamilyIndexCount = queueFamilyIndexCount;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
		createInfo.preTransform = preTransform;
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.presentMode = presentMode;
		createInfo.clipped = clipped;
		createInfo.oldSwapchain = oldSwapchain;

		return createInfo;
	}

private:
	VkExtent2D extent;
	uint32_t imageCount;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkSharingMode imageSharingMode;
	uint32_t queueFamilyIndexCount;
	const uint32_t * queueFamilyIndices;
	VkSurfaceTransformFlagBitsKHR preTransform;
	uint32_t imageArrayLayers = 1;
	VkImageUsageFlagBits imageUseage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkBool32 clipped = VK_TRUE;
	VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
};

