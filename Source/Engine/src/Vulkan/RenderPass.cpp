#include "pch.h"
#include "RenderPass.h"

#include "Vulkan/SwapChain.h"
#include "VulkanContext.h"
#include "VulkanFramebuffer.h"
#include "VulkanDevice.h"

#include "Core/Engine.h"
#include "Core/Window.h"


namespace PAL
{

	RenderPass::RenderPass(const SharedPtr<VulkanDevice>& vulkanDevice, const RenderpassSpecification& renderpassSpec)
		: m_RenderpassSpec(renderpassSpec)
	{
		VkAttachmentDescription attachmentDescription = {};
		attachmentDescription.flags = 0;
		attachmentDescription.format = Engine::Get()->GetWindow()->GetSwapChain()->GetSwapChainImageFormat();
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //VK_ATTACHMENT_LOAD_OP_CLEAR or do not clear on load use => VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (renderpassSpec.Target == PresentTarget::Swapchain)
			attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		else if (renderpassSpec.Target == PresentTarget::CustomViewport)
			attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachmentDescription;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(vulkanDevice->GetVkDevice(), &renderPassInfo, nullptr, &m_RenderPass));
	}

	void RenderPass::Shutdown()
	{
		vkDestroyRenderPass(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_RenderPass, nullptr);
	}

	void RenderPass::Begin(VkCommandBuffer currentCommandBuffer, uint32_t imageIndex)
	{
		const SharedPtr<VulkanSwapChain>& swapchain = Engine::Get()->GetWindow()->GetSwapChain();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_Framebuffer->GetFramebuffer(imageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { m_Framebuffer->GetSpecification().Width, m_Framebuffer->GetSpecification().Height };;

		VkClearValue clearValues[1];
		clearValues[0].color = { {0.0f, 0.0f,0.0f, 1.0f} };

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPass::End(VkCommandBuffer currentCommandBuffer)
	{
		vkCmdEndRenderPass(currentCommandBuffer);
	}
}