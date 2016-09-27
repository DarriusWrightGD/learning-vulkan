#pragma once

#include <vulkan\vulkan.h>
#include <stdexcept>

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find a suitable memory type!");
}

class BufferInfoBuilder
{
public:
	BufferInfoBuilder(uint32_t size): bufferSize(size) {

	}

	BufferInfoBuilder(uint32_t size, VkBufferUsageFlagBits usage): bufferSize(size), bufferUsage(usage) {

	}

	VkBufferCreateInfo Build() {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferSize;
		bufferInfo.usage = bufferUsage;
		bufferInfo.sharingMode = sharingMode;
		return bufferInfo;
	}
private:
	uint32_t bufferSize;
	VkBufferUsageFlagBits bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
};

//
//class BufferBuilder
//{
//
//
//	VkBuffer Build() {
//		VkBufferCreateInfo bufferInfo = {};
//		//bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//		//bufferInfo.size = sizeof(vertices[0]) * vertices.size();
//		//bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//		//bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//		//vkOk(vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer), "Failed to create the vertex buffer");
//
//		//VkMemoryRequirements memoryRequirements;
//		//vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);
//
//		//VkMemoryAllocateInfo allocateInfo = {};
//		//allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//		//allocateInfo.allocationSize = memoryRequirements.size;
//		//allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//
//		//vkOk(vkAllocateMemory(device, &allocateInfo, nullptr, &vertexBufferMemory), "Failed to allocate vertex buffer memory");
//
//		//vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
//		//void * data;
//		//vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
//		//memcpy(data, vertices.data(), (size_t)bufferInfo.size);
//		//vkUnmapMemory(device, vertexBufferMemory);
//		return bufferInfo;
//	}
//
//private:
//	VkBuffer buffer;
//};