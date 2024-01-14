#include "pch.h"
#include "VulkanFrameBuffer.h"
#include "VulkanContext.h"
#include "SwapChain.h"
#include "Event/Event.h"
#include "Core/Engine.h"
#include "Renderer/RuntimeRenderer.h"
#include "Renderer/RenderAPI.h"

namespace PAL
{
	VulkanFramebuffer::VulkanFramebuffer(VkRenderPass renderPass, const FramebufferSpecification& spec)
		: m_Device(VulkanContext::GetVulkanDevice()->GetVkDevice())
		, m_PhysicalDevice(VulkanContext::GetVulkanDevice()->GetPhysicalDevice()->GetVkPhysicalDevice())
		, m_RenderPass(renderPass)
		, m_Spec(spec)
	{
		Resize(spec.Width, spec.Height);

		if (spec.Target == PresentTarget::Swapchain)
			Engine::Get()->GetWindow()->GetSwapChain()->BindResizeCallback(BIND_FUN(this, VulkanFramebuffer::Resize));
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
	}

	void VulkanFramebuffer::Shutdown()
	{
		VulkanMemoryAllocator allocator("FrameBuffer Image");

		for (int i = 0; i < m_Framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_Device, m_Framebuffers[i], nullptr);

			if (m_Spec.Target == PresentTarget::CustomViewport)
			{
				allocator.DestroyImage(m_FramebufferImages[i].ColorImage, m_ImageAllocations[i]);
				vkDestroyImageView(m_Device, m_FramebufferImages[i].ColorImageView, nullptr);
				vkDestroySampler(m_Device, m_FramebufferImages[i].TextureSampler, nullptr);
			}
		}
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		VulkanMemoryAllocator allocator("FrameBuffer Image");
		for (int i = 0; i < m_Framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_Device, m_Framebuffers[i], nullptr);

			if (m_Spec.Target == PresentTarget::CustomViewport)
			{
				vkDestroyImageView(m_Device, m_FramebufferImages[i].ColorImageView, nullptr);
				allocator.DestroyImage(m_FramebufferImages[i].ColorImage, m_ImageAllocations[i]);
				vkDestroySampler(m_Device, m_FramebufferImages[i].TextureSampler, nullptr);
			}
		}

		m_Spec.Width = width;
		m_Spec.Height = height;
		auto& swapChain = Engine::Get()->GetWindow()->GetSwapChain();

		m_FramebufferImages.resize(VulkanAPI::GetMaxFramesInFlight());
		m_ImageAllocations.resize(VulkanAPI::GetMaxFramesInFlight());

		if (m_Spec.Target == PresentTarget::Swapchain)
		{
			for (int i = 0; i < swapChain->GetSwapChainImageViews().size(); i++)
			{
				m_FramebufferImages[i].ColorImage = swapChain->GetSwapChainImages().at(i);
				m_FramebufferImages[i].ColorImageView = swapChain->GetSwapChainImageViews().at(i);
				m_FramebufferImages[i].ImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
		}
		else
		{
			CreateColorResources();
			if (m_Spec.UseDepth)
			{
				CreateDepthResources();
			}
		}

		m_Framebuffers.resize(VulkanAPI::GetMaxFramesInFlight());
		for (uint32_t i = 0; i < VulkanAPI::GetMaxFramesInFlight(); ++i)
		{
			CreateFramebuffer(i);
		}
	}

	void VulkanFramebuffer::CreateFramebuffer(uint32_t index)
	{
		m_FramebufferImages[index].ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		std::array<VkImageView, 2> attachments;
		attachments[0] = m_FramebufferImages[index].ColorImageView; // Color attachment

		//if (m_Spec.useDepth)
		//{
		//	attachments[1] = m_DepthImageView; // Depth attachment
		//}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = m_Spec.UseDepth ? 2 : 1;
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_Spec.Width;
		framebufferInfo.height = m_Spec.Height;
		framebufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_Framebuffers[index]));
	}

	void VulkanFramebuffer::CreateColorResources()
	{
		auto format = Engine::Get()->GetWindow()->GetSwapChain()->GetSwapChainImageFormat();

		for (uint32_t i = 0; i < VulkanAPI::GetMaxFramesInFlight(); ++i)
		{
			CreateImage(m_Spec.Width, m_Spec.Height, m_Spec.ColorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
						m_FramebufferImages[i].ColorImage, m_ImageAllocations[i]);

			m_FramebufferImages[i].ColorImageView = CreateImageView(m_FramebufferImages[i].ColorImage, m_Spec.ColorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		CreateTextureSampler();
	}

	void VulkanFramebuffer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkImage& image, VmaAllocation& outAllocation)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VulkanMemoryAllocator allocator("FrameBuffer Image");
		outAllocation = allocator.AllocateImage(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, image);
	}

	VkImageView VulkanFramebuffer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};

		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.flags = 0;
		viewInfo.components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};

		viewInfo.subresourceRange = {};

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VK_CHECK_RESULT(vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView));

		return imageView;
	}

	void VulkanFramebuffer::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.maxAnisotropy = 1.0f;

		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 100.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		for (int i = 0; i < m_FramebufferImages.size(); i++)
		{
			VK_CHECK_RESULT(vkCreateSampler(VulkanContext::GetVulkanDevice()->GetVkDevice(), &samplerCreateInfo, nullptr, &m_FramebufferImages[i].TextureSampler));
		}
	}

	void VulkanFramebuffer::CreateDepthResources()
	{
		//VkFormat depthFormat = FindDepthFormat();
		//CreateImage(m_Spec.width, m_Spec.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
		//m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		//
		//TransitionImageLayout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	// Helper function to find a suitable depth format (implementation may vary)
	VkFormat VulkanFramebuffer::FindDepthFormat()
	{
		/* @TODO: Choose a format that supports depth stencil */
		return VK_FORMAT_UNDEFINED;
	}

}