#pragma once
#include <vulkan\vulkan.h>
#include <functional>


template <typename T> class VDeleter {
public:
	VDeleter() : VDeleter([](T, VkAllocationCallbacks*) {}) {}
	VDeleter(std::function<void(T, VkAllocationCallbacks*)> deleteFunction) {
		this->deleter = [=](T vkObject) {deleteFunction(vkObject, nullptr); };
	}
	VDeleter(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleteFunction) {
		this->deleter = [&instance, deleteFunction](T vkObject) {deleteFunction(instance, vkObject, nullptr); };
	}
	VDeleter(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deleteFunction) {
		this->deleter = [&device, deleteFunction](T vkObject) {deleteFunction(device, vkObject, nullptr); };
	}

	~VDeleter() {
		cleanup();
	}

	T* operator &() {
		cleanup();
		return &object;
	}

	operator T() const {
		return object;
	}

private:
	T object{ VK_NULL_HANDLE };
	std::function<void(T)> deleter;

	void cleanup() {
		if (object != VK_NULL_HANDLE) {
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}
};
