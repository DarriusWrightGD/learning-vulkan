#pragma once
#include "..\FileReader.h"
#include <vector>

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
public:
	ShaderStageBuilder * AddStage(VkShaderStageFlagBits shaderStage, VkShaderModule shaderModule) {
		VkPipelineShaderStageCreateInfo shader = {};
		shader.sType = type;
		shader.module = shaderModule;
		shader.stage = shaderStage;
		shader.pName = name;
		shaders.push_back(shader);

		return this;
	}

	ShaderStageBuilder* AddVertexShader(VkShaderModule shaderModule) {
		return AddStage(VK_SHADER_STAGE_VERTEX_BIT, shaderModule);
	}

	ShaderStageBuilder* AddFragmentShader(VkShaderModule shaderModule) {
		return AddStage(VK_SHADER_STAGE_FRAGMENT_BIT, shaderModule);
	}

	std::vector<VkPipelineShaderStageCreateInfo> BuildStages() {
		return shaders;
	}
private:
	const char * name = "main";
	const VkStructureType type = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	std::vector<VkPipelineShaderStageCreateInfo> shaders;
	VkShaderModule shaderModule;
};

class VertexInputBuilder
{
public:
	
	VertexInputBuilder * WithAttributes(const VkVertexInputAttributeDescription * attributes,unsigned int count) {
		vertexAttributeCount = count;
		vertexAttributeDescriptions = attributes;
		return this;
	}

	VertexInputBuilder * WithBindings(const VkVertexInputBindingDescription * bindings, unsigned int count) {
		vertexBindingCount = count;
		vertexBindingDescriptions = bindings;
		return this;
	}

	VkPipelineVertexInputStateCreateInfo Build()
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = type;
		vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeCount;
		vertexInputInfo.vertexBindingDescriptionCount = vertexBindingCount;
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions;
		vertexInputInfo.pVertexBindingDescriptions = vertexBindingDescriptions;

		return vertexInputInfo;
	}
private:
	unsigned int vertexAttributeCount = 0;
	unsigned int vertexBindingCount = 0;
	const VkVertexInputAttributeDescription * vertexAttributeDescriptions = nullptr;
	const VkVertexInputBindingDescription * vertexBindingDescriptions = nullptr;
	VkStructureType type = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
};

class InputAssemblyBuilder
{
public:
	InputAssemblyBuilder * WithTopology(VkPrimitiveTopology topo) {
		topology = topo;
		return this;
	}

	InputAssemblyBuilder * WithTriangleListTopology(){
		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		return this;
	}

	InputAssemblyBuilder * EnablePrimitiveRestart() {
		primitiveRestartEnabled = VK_TRUE;
		return this;
	}


	InputAssemblyBuilder * DisablePrimitiveRestart() {
		primitiveRestartEnabled = VK_FALSE;
		return this;
	}

	VkPipelineInputAssemblyStateCreateInfo Build()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = type;
		inputAssembly.primitiveRestartEnable = primitiveRestartEnabled;
		inputAssembly.topology = topology;
		return inputAssembly;
	}

private:
	VkStructureType type = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	VkBool32 primitiveRestartEnabled = VK_FALSE;
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
};

class ViewportBuilder
{
public:
	ViewportBuilder(float width, float height): width(width), height(height){

	}

	ViewportBuilder* WithStartX(float x) {
		startX = x;
		return this;
	}

	ViewportBuilder* WithStartY(float y) {
		startY = y;
		return this;
	}

	ViewportBuilder* WithMinDepth(float x) {
		startX = x;
		return this;
	}

	ViewportBuilder* WithMaxDepth(float y) {
		startY = y;
		return this;
	}

	VkViewport Build() {
		VkViewport viewport = {};
		viewport.x = startX;
		viewport.y = startY;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;

		return viewport;
	}
private:
	float startX = 0.0f;
	float startY = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	float minDepth = 0.0f;
	float maxDepth = 1.0f;
};

class ScissorBuilder
{
public:
	ScissorBuilder(VkExtent2D extent) : extent(extent) {

	}

	ScissorBuilder * WithOffset(int32_t x, int32_t y) {
		xOffset = x;
		yOffset = y;
	}

	VkRect2D Build() {
		VkRect2D scissor = {};
		scissor.offset = { xOffset,yOffset };
		scissor.extent = extent;
		return scissor;
	}
private:
	int32_t xOffset = 0;
	int32_t yOffset = 0;
	VkExtent2D extent;
};

class ViewportStateBuilder
{
public:
	ViewportStateBuilder(VkViewport * viewports, VkRect2D * scissors) : viewports(viewports), scissors(scissors){

	}

	ViewportStateBuilder * WithViewportCount(uint32_t count)
	{
		viewportCount = count;
		return this;
	}

	ViewportStateBuilder * WithScissorCount(uint32_t count)
	{
		scissorCount = count;
		return this;
	}

	VkPipelineViewportStateCreateInfo Build()
	{
		VkPipelineViewportStateCreateInfo viewportStateInfo = {};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = viewportCount;
		viewportStateInfo.pViewports = viewports;
		viewportStateInfo.scissorCount = scissorCount;
		viewportStateInfo.pScissors = scissors;

		return viewportStateInfo;
	}
private:
	VkViewport * viewports;
	VkRect2D * scissors;
	uint32_t viewportCount = 1;
	uint32_t scissorCount = 1;
};

class RasterizationStateBuilder
{
public:
	RasterizationStateBuilder* WithCullMode(VkCullModeFlagBits mode) {
		cullMode = mode;
		return this;
	}

	RasterizationStateBuilder* WithBackCulling() {
		return WithCullMode(VK_CULL_MODE_BACK_BIT);
	}

	RasterizationStateBuilder* WithFrontCulling() {
		return WithCullMode(VK_CULL_MODE_FRONT_BIT);
	}

	RasterizationStateBuilder* WithNoCulling() {
		return WithCullMode(VK_CULL_MODE_NONE);
	}

	RasterizationStateBuilder* EnabledDepthClamp() {
		depthClampEnable = VK_TRUE;
		return this;
	}
	RasterizationStateBuilder* EnableDiscard() {
		rasterizerDiscardEnable = VK_TRUE;
		return this;
	}
	RasterizationStateBuilder* EnabledDepthBias() {
		depthBiasEnable = VK_TRUE;
		return this;
	}

	RasterizationStateBuilder* DisableDepthClamp() {
		depthClampEnable = VK_FALSE;
		return this;
	}
	RasterizationStateBuilder* DisableDiscard() {
		rasterizerDiscardEnable = VK_FALSE;
		return this;
	}
	RasterizationStateBuilder* DisableDepthBias() {
		depthBiasEnable = VK_FALSE;
		return this;
	}

	RasterizationStateBuilder* WithPolygonMode(VkPolygonMode mode) {
		polygonMode = mode;
		return this;
	}

	RasterizationStateBuilder* WithPolygonFillMode() {
		return WithPolygonMode(VK_POLYGON_MODE_FILL);
	}

	RasterizationStateBuilder* WithFrontFace(VkFrontFace face) {
		frontFace = face;
		return this;
	}

	RasterizationStateBuilder* WithClockwiseFace() {
		return WithFrontFace(VK_FRONT_FACE_CLOCKWISE);
	}

	RasterizationStateBuilder* WithCounterClockwiseFace() {
		return WithFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
	}

	RasterizationStateBuilder* WithLineWidth(float width) {
		lineWidth = width;
		return this;
	}

	RasterizationStateBuilder* WithDepthBiasConstantFactor(float factor) {
		depthBiasConstantFactor = factor;
		return this;
	}

	RasterizationStateBuilder* WithDepthBiasSlopeFactor(float factor) {
		depthBiasSlopeFactor = factor;
		return this;
	}

	RasterizationStateBuilder* WithDepthBiasClamp(float clamp) {
		depthBiasClamp = clamp;
		return this;
	}

	VkPipelineRasterizationStateCreateInfo Build()
	{
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = depthClampEnable;
		rasterizer.rasterizerDiscardEnable = rasterizerDiscardEnable;
		rasterizer.polygonMode = polygonMode;
		rasterizer.lineWidth = lineWidth;
		rasterizer.cullMode = cullMode;
		rasterizer.frontFace = frontFace;
		rasterizer.depthBiasEnable = depthBiasEnable;
		rasterizer.depthBiasConstantFactor = depthBiasConstantFactor;
		rasterizer.depthBiasClamp = depthBiasClamp;
		rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;
	
		return rasterizer;
	}

private:
	VkBool32 depthClampEnable = VK_FALSE;
	VkBool32 rasterizerDiscardEnable = VK_FALSE;
	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
	float lineWidth = 1.0f;
	VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
	VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
	VkBool32 depthBiasEnable = VK_FALSE;
	float depthBiasConstantFactor = 0.0f;
	float depthBiasClamp = 0.0f;
	float depthBiasSlopeFactor = 0.0f;
};

class MultisampleStateBuilder
{
public:

	VkPipelineMultisampleStateCreateInfo Build()
	{
		VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.sampleShadingEnable = sampleShadingEnable;
		multisampleInfo.rasterizationSamples = rasterizationSamples;
		multisampleInfo.minSampleShading = minSampleShading;
		multisampleInfo.pSampleMask = sampleMask;
		multisampleInfo.alphaToOneEnable = alphaToOneEnable;
		multisampleInfo.alphaToCoverageEnable = alphaToCoverageEnable;
		return multisampleInfo;
	}
private:
	VkBool32 sampleShadingEnable = VK_FALSE;
	VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	float minSampleShading = 1.0f;
	const VkSampleMask * sampleMask = nullptr;
	VkBool32 alphaToOneEnable = VK_FALSE;
	VkBool32 alphaToCoverageEnable = VK_FALSE;
};

class ColorBlendAttachmentStateBuilder
{
public:

	ColorBlendAttachmentStateBuilder* WithColorWriteMask(VkColorComponentFlags flags) {
		colorWriteMask = flags;
		return this;
	}

	ColorBlendAttachmentStateBuilder* EnableBlend() {
		blendEnable = VK_TRUE;
		return this;
	}

	ColorBlendAttachmentStateBuilder* DisableBlend() {
		blendEnable = VK_FALSE;
		return this;
	}

	ColorBlendAttachmentStateBuilder* WithSourceColorBlendFactor(VkBlendFactor factor) {
		srcColorBlendFactor = factor;
		return this;
	}

	ColorBlendAttachmentStateBuilder* WithDistColorBlendFactor(VkBlendFactor factor) {
		dstColorBlendFactor = factor;
		return this;
	}

	ColorBlendAttachmentStateBuilder* WithSourceAlphaBlendFactor(VkBlendFactor factor) {
		srcAlphaBlendFactor = factor;
		return this;
	}

	ColorBlendAttachmentStateBuilder* WithDistColorAlphaFactor(VkBlendFactor factor) {
		dstAlphaBlendFactor = factor;
		return this;
	}

	ColorBlendAttachmentStateBuilder* WithColorBlendOperation(VkBlendOp op) {
		colorBlendOp = op;
		return this;
	}

	ColorBlendAttachmentStateBuilder* WithAlphaBlendOperation(VkBlendOp op) {
		alphaBlendOp = op;
		return this;
	}

	VkPipelineColorBlendAttachmentState Build()
	{
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = colorWriteMask;
		colorBlendAttachment.blendEnable = blendEnable;
		colorBlendAttachment.srcColorBlendFactor = srcColorBlendFactor;
		colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor;
		colorBlendAttachment.colorBlendOp = colorBlendOp;
		colorBlendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor;
		colorBlendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor;
		colorBlendAttachment.alphaBlendOp = alphaBlendOp;

		return colorBlendAttachment;
	}
private:
	VkColorComponentFlags colorWriteMask = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
	VkBool32 blendEnable = VK_FALSE;
	VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
	VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
};

class ColorBlendStateBuilder
{
public:
	ColorBlendStateBuilder() {

	}

	ColorBlendStateBuilder(VkPipelineColorBlendAttachmentState * attachments, uint32_t attachmentCount) : colorAttachments(attachments), attachmentCount(attachmentCount) {

	}

	ColorBlendStateBuilder* EnableLogicOperation() {
		logicOpEnable = VK_TRUE;
		return this;
	}

	ColorBlendStateBuilder* DisableLogicOperation() {
		logicOpEnable = VK_FALSE;
		return this;
	}

	ColorBlendStateBuilder* WithLogicOperation(VkLogicOp operation) {
		logicOp = operation;
		return this;
	}

	ColorBlendStateBuilder* WithBlendConstants(float r, float g, float b, float a) {
		blendConstants[0] = r;
		blendConstants[1] = g;
		blendConstants[2] = b;
		blendConstants[3] = a;
		
		return this;
	}

	VkPipelineColorBlendStateCreateInfo Build()
	{
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = logicOpEnable;
		colorBlending.logicOp = logicOp;
		colorBlending.attachmentCount = attachmentCount;
		colorBlending.pAttachments = colorAttachments;
		colorBlending.blendConstants[0] = blendConstants[0];
		colorBlending.blendConstants[1] = blendConstants[1];
		colorBlending.blendConstants[2] = blendConstants[2];
		colorBlending.blendConstants[3] = blendConstants[3];

		return colorBlending;
	}
private:
	const VkPipelineColorBlendAttachmentState basicBlendAttachment = ColorBlendAttachmentStateBuilder().Build();

	VkBool32 logicOpEnable = VK_FALSE;
	VkLogicOp logicOp = VK_LOGIC_OP_COPY;
	uint32_t attachmentCount = 1;
	const VkPipelineColorBlendAttachmentState * colorAttachments = &basicBlendAttachment;
	float blendConstants[4] = { 0.0f };
};

class PipelineLayoutBuilder
{
public:
	PipelineLayoutBuilder()
	{

	}

	PipelineLayoutBuilder(uint32_t layoutCount , const VkDescriptorSetLayout * layouts) : setLayoutCount(layoutCount),
		layouts(layouts)
	{

	}
	VkPipelineLayoutCreateInfo Build()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = setLayoutCount;
		pipelineLayoutInfo.pSetLayouts = layouts;
		pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangeCount;
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;

		return pipelineLayoutInfo;
	}
private:
	uint32_t setLayoutCount = 0;
	const VkDescriptorSetLayout * layouts = nullptr;
	uint32_t pushConstantRangeCount = 0;
	const VkPushConstantRange * pushConstantRanges = nullptr;
};

class GraphicsPipelineBuilder
{
public:
	GraphicsPipelineBuilder(const std::vector<VkPipelineShaderStageCreateInfo> & shaders,
		VkPipelineViewportStateCreateInfo viewportState, VkPipelineColorBlendStateCreateInfo colorBlendState,
		VkPipelineLayout pipelineLayout, VkRenderPass renderPass)
	: shaderStages(shaders.data()), shaderCount(shaders.size()), viewportState(viewportState),
		colorBlendState(colorBlendState), layout(pipelineLayout), renderPass(renderPass){

	}

	GraphicsPipelineBuilder* WithVertexInputState(VkPipelineVertexInputStateCreateInfo inputState) {
		vertexInputState = inputState;
		return this;
	}

	GraphicsPipelineBuilder* WithInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo inputState) {
		inputAssemblyState = inputState;
		return this;
	}

	GraphicsPipelineBuilder* WithPipelineLayout(VkPipelineLayout layout) {
		this->layout = layout;
		return this;
	}

	GraphicsPipelineBuilder* WithMultisampleState(VkPipelineMultisampleStateCreateInfo inputState) {
		multisampleState = inputState;
		return this;
	}

	GraphicsPipelineBuilder* WithRasterizationState(VkPipelineRasterizationStateCreateInfo inputState) {
		rasterizationState = inputState;
		return this;
	}

	GraphicsPipelineBuilder* WithDepthStencilState(VkPipelineDepthStencilStateCreateInfo inputState) {
		depthStencilState = inputState;
		return this;
	}

	GraphicsPipelineBuilder* WithDynamicState(VkPipelineDynamicStateCreateInfo inputState) {
		dynamicState = inputState;
		return this;
	}

	GraphicsPipelineBuilder* WithSubpass(uint32_t subpass) {
		this->subpass = subpass;
		return this;
	}

	GraphicsPipelineBuilder* WithBasePipeline(VkPipeline pipelineHandle, uint32_t pipelineIndex) {
		basePipelineHandle = pipelineHandle;
		basePipelineIndex = pipelineIndex;

		return this;
	}

	VkGraphicsPipelineCreateInfo Build(){
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderCount;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputState;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pRasterizationState = &rasterizationState;
		pipelineInfo.pDepthStencilState = (depthStencilState.sType == NULL_TYPE) ? nullptr: &depthStencilState;
		pipelineInfo.pColorBlendState = (colorBlendState.sType == NULL_TYPE) ? nullptr : &colorBlendState;
		pipelineInfo.pDynamicState = (dynamicState.sType == NULL_TYPE) ? nullptr : &dynamicState;
		pipelineInfo.layout = layout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = subpass;
		pipelineInfo.basePipelineHandle = basePipelineHandle;
		pipelineInfo.basePipelineIndex = basePipelineIndex;

		return pipelineInfo;
	}

private:
	const VkStructureType NULL_TYPE = (VkStructureType)-858993460;
	uint32_t shaderCount = 2;
	const VkPipelineVertexInputStateCreateInfo basicVertexInput = VertexInputBuilder().Build();
	const VkPipelineInputAssemblyStateCreateInfo basicInputState = InputAssemblyBuilder().Build();
	const VkPipelineMultisampleStateCreateInfo basicMultisampleState = MultisampleStateBuilder().Build();
	const VkPipelineRasterizationStateCreateInfo basicRasterizationState = RasterizationStateBuilder().Build();

	const VkPipelineShaderStageCreateInfo * shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputState = basicVertexInput;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = basicInputState;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineMultisampleStateCreateInfo multisampleState = basicMultisampleState;
	VkPipelineRasterizationStateCreateInfo rasterizationState = basicRasterizationState;
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	VkPipelineDynamicStateCreateInfo dynamicState;
	VkPipelineLayout layout;
	VkRenderPass renderPass;
	uint32_t subpass = 0;
	VkPipeline basePipelineHandle = VK_NULL_HANDLE;
	uint32_t basePipelineIndex = -1;
};