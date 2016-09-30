#pragma once
#include <vulkan\vulkan.h>
#include <VkDeleter.h>
#include <functional>

template <typename T> class VRelease {
public:
	VRelease() : VRelease([](T, VkAllocationCallbacks*) {}) {}
	VRelease(std::function<void(T, VkAllocationCallbacks*)> deleteFunction) {
		this->deleter = [=](T vkObject) {deleteFunction(vkObject, nullptr); };
	}
	VRelease(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleteFunction) {
		this->deleter = [&instance, deleteFunction](T vkObject) {deleteFunction(instance, vkObject, nullptr); };
	}
	VRelease(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deleteFunction) {
		this->deleter = [&device, deleteFunction](T vkObject) {deleteFunction(device, vkObject, nullptr); };
	}

	~VRelease() {
	}

	T* operator &() {
		Release();
		return &object;
	}

	operator T() const {
		return object;
	}

	void Release() {
		if (object != VK_NULL_HANDLE) {
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}
private:
	T object{ VK_NULL_HANDLE };
	std::function<void(T)> deleter;


};
