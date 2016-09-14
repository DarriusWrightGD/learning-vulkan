#pragma once
#include "..\FileReader.h"

class ShaderModuleInfoBuilder {
public:
	ShaderModuleInfoBuilder(size_t codeSize, const char * data)
		:codeSize(codeSize), data(data)
	{

	}

	ShaderModuleInfoBuilder(const std::vector<char> & fileContents) {
		codeSize = fileContents.size();
		data = fileContents.data();
	}

	VkShaderModuleCreateInfo Build() {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = codeSize;
		createInfo.pCode = (uint32_t*)data;

		return createInfo;
	}
private:
	size_t codeSize;
	const char * data = nullptr;
};

class ShaderStageBuilder {

private:
	const char * name = "main";
	VkShaderModule shaderModule;
};