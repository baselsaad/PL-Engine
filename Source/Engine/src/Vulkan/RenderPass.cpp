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
		std::vector<VkAttachmentDescription> attachmentDescriptions;

		VkAttachmentDescription colorAttachmentDesc;
		colorAttachmentDesc.flags = 0;
		colorAttachmentDesc.format = Engine::Get()->GetWindow()->GetSwapChain()->GetSwapChainImageFormat();
		colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //VK_ATTACHMENT_LOAD_OP_CLEAR or do not clear on load use => VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (renderpassSpec.Target == PresentTarget::Swapchain)
			colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		else if (renderpassSpec.Target == PresentTarget::CustomViewport)
			colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		attachmentDescriptions.push_back(colorAttachmentDesc);

	#if 0
		// Object ID attachment description (if used)
		if (renderpassSpec.UseObjectIdAttachment)
		{
			VkAttachmentDescription objectIdAttachmentDesc = {};
			objectIdAttachmentDesc.flags = 0;
			objectIdAttachmentDesc.format = m_RenderpassSpec.ColorFormat;
			objectIdAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			objectIdAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			objectIdAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			objectIdAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			objectIdAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			objectIdAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			objectIdAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			attachmentDescriptions.push_back(objectIdAttachmentDesc);
		}
	#endif

		std::vector<VkAttachmentReference> colorAttachmentRefs;
		colorAttachmentRefs.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		//colorAttachmentRefs.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colorAttachmentRefs.size();
		subpass.pColorAttachments = colorAttachmentRefs.data();

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentDescriptions.size();
		renderPassInfo.pAttachments = attachmentDescriptions.data();
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

		std::array<VkClearValue, 1> clearValues;
		clearValues[0] = { 0.0f, 0.0f,0.0f, 1.0f };

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPass::End(VkCommandBuffer currentCommandBuffer)
	{
		vkCmdEndRenderPass(currentCommandBuffer);
	}
}