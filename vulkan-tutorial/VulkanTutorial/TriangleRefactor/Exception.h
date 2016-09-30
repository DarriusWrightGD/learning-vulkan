#pragma once
#include <vulkan\vulkan.h>
#include <stdexcept>


static void vkOk(VkResult vkResult, const char * message) {
	if (vkResult != VK_SUCCESS) throw std::runtime_error(message);
}

static void vkOk(VkResult vkResult) {
	vkOk(vkResult, "Vulkan call failed!");
}