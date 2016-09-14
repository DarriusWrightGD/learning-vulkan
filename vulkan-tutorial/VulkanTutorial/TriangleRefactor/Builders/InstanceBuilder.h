#pragma once
#include <vulkan\vulkan.h>
#include <string>
class ApplicationInfoBuilder
{
public:
	VkApplicationInfo Build() {
		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = type;
		applicationInfo.pApplicationName = applicationName;
		applicationInfo.pEngineName = engineName;

		applicationInfo.applicationVersion = applicationVersion;
		applicationInfo.engineVersion = engineVersion;
		applicationInfo.apiVersion = apiVersion;

		return applicationInfo;
	}

	ApplicationInfoBuilder * WithApplicationName(const char * name) {
		applicationName = name;
		return this;
	}

	ApplicationInfoBuilder * WithEngineName(const char * name) {
		engineName = name;
		return this;
	}

	ApplicationInfoBuilder * WithApplicationVersion(uint32_t major, uint32_t minor, uint32_t patch) {
		applicationVersion = VK_MAKE_VERSION(major, minor, patch);
		return this;
	}

	ApplicationInfoBuilder * WithEngineVersion(uint32_t major, uint32_t minor, uint32_t patch) {
		engineVersion = VK_MAKE_VERSION(major, minor, patch);
		return this;
	}

private:
	VkStructureType type = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	const char * applicationName = "Vulkan Application";
	uint32_t applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0);
	const char * engineName = "Vulkan Engine";
	uint32_t apiVersion = VK_API_VERSION_1_0;
};

class InstanceInfoBuilder
{
public:
	InstanceInfoBuilder * WithApplicationInfo(VkApplicationInfo appInfo) {
		applicationInfo = appInfo;
		return this;
	}

	InstanceInfoBuilder * WithEnabledExtensions(uint32_t count, const char * const * extensions) {
		extensionCount = count;
		extensionNames = extensions;
		return this;
	}

	InstanceInfoBuilder * WithEnabledLayers(uint32_t count, const char * const* layers) {
		layerCount = count;
		layerNames = layers;

		return this;
	}

	VkInstanceCreateInfo Build() {
		VkInstanceCreateInfo createInstanceInfo = {};
		createInstanceInfo.sType = type;
		createInstanceInfo.pApplicationInfo = &applicationInfo;
		createInstanceInfo.enabledExtensionCount = extensionCount;
		createInstanceInfo.ppEnabledExtensionNames = extensionNames;
		createInstanceInfo.enabledLayerCount = layerCount;
		createInstanceInfo.ppEnabledLayerNames = layerNames;

		return createInstanceInfo;
	}
private:
	VkStructureType type = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	VkApplicationInfo applicationInfo = ApplicationInfoBuilder().Build();
	
	uint32_t extensionCount = 0;
	const char * const* extensionNames = nullptr;

	uint32_t layerCount = 0;
	const char * const * layerNames = nullptr;
};