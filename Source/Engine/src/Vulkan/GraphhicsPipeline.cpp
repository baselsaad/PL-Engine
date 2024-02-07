#include "pch.h"
#include "GraphhicsPipeline.h"
#include "Shader.h"
#include "RenderPass.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanVertexBuffer.h"

namespace PAL
{
	PipeLine::PipeLine(const SharedPtr<RenderPass>& renderpass)
	{
		auto vertShaderCode = Shader::ReadFile(PROJECT_ROOT "/Source/Engine/res/shaders/vert.spv");
		auto fragShaderCode = Shader::ReadFile(PROJECT_ROOT "/Source/Engine/res/shaders/frag.spv");

		VkShaderModule vertShaderModule = Shader::CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = Shader::CreateShaderModule(fragShaderCode);

		//Shader stage creation
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		 
		// Vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		auto bindingDescription = VulkanQuadVertex::GetBindingDescription();
		auto attributeDescriptions = VulkanQuadVertex::GetAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		/*
		*	VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
		*	VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
		*	VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
		*	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
		*	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
		*/
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};

		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;

		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		if (renderpass->GetRenderpassSpec().Target == PresentTarget::CustomViewport)
		{
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}

		std::vector<VkPipelineColorBlendAttachmentState> states;
		states.push_back(colorBlendAttachment);
		// another color attachment 
		// states.push_back(colorBlendAttachment);

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount = states.size();
		colorBlending.pAttachments = states.data();

		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// Push Constant for Vertex Shader
		VkPushConstantRange vertexPushConstantRange{};
		vertexPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		vertexPushConstantRange.offset = 0;
		vertexPushConstantRange.size = sizeof(glm::mat4); // Size of transformation matrix

		// Push Constant for Fragment Shader
		//VkPushConstantRange fragmentPushConstantRange{};
		//fragmentPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		//fragmentPushConstantRange.offset = vertexPushConstantRange.size; // Adjusted offset
		//fragmentPushConstantRange.size = sizeof(glm::vec2); // Size of vec2 for mouse position

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1; // count to include both push constants
		//VkPushConstantRange pushConstantRanges[] = { vertexPushConstantRange, fragmentPushConstantRange };
		VkPushConstantRange pushConstantRanges[] = { vertexPushConstantRange };
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;

		VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::GetVulkanDevice()->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = renderpass->GetVkRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(VulkanContext::GetVulkanDevice()->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline));

		vkDestroyShaderModule(VulkanContext::GetVulkanDevice()->GetVkDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(VulkanContext::GetVulkanDevice()->GetVkDevice(), vertShaderModule, nullptr);
	}

	void PipeLine::Shutdown()
	{
		vkDestroyPipeline(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_PipelineLayout, nullptr);
	}
}