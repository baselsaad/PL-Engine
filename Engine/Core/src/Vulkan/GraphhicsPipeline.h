#pragma once
#include "VulkanAPI.h"

namespace PL_Engine
{
	class RenderPass;

	class PipeLine
	{
	public:
		PipeLine(const SharedPtr<RenderPass>& renderpass);
		void Shutdown();

		inline VkPipelineLayout GetPipelineLayout() { return m_PipelineLayout; }
		inline VkPipeline GetGraphicsPipeline() { return m_GraphicsPipeline; }

	private:
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
	};


}