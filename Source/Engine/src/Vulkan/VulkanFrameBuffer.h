#pragma once
#include <vulkan/vulkan.h>
#include "VulkanAPI.h"
#include "VulkanContext.h"
#include "SwapChain.h"

namespace PAL
{
	struct FramebufferSpecification;

	struct VulkanImage
	{
		VkImage ColorImage;
		VkImageView ColorImageView;
		VkSampler TextureSampler;
		VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer(VkRenderPass renderPass, const FramebufferSpecification& spec);
		virtual ~VulkanFramebuffer();

		void Resize(uint32_t width, uint32_t height);
		void Shutdown();

		inline VkFramebuffer GetFramebuffer(uint32_t index) const { return m_Framebuffers[index]; };

		inline VulkanImage* GetFrameBufferImage(uint32_t index) { return &m_FramebufferImages[index]; }
		
		inline VulkanImage* GetFrameBufferImage() 
		{ 
			return &m_FramebufferImages[Engine::Get()->GetWindow()->GetSwapChain()->GetImageIndex()];
		}

		inline VkImageView GetDepthImageView() const { return m_DepthImageView; }
		inline VkRenderPass GetRenderPass() const { return m_RenderPass; }
		inline FramebufferSpecification& GetSpecification() { return m_Spec; }

		inline void SetRenderPass(VkRenderPass renderPass) { m_RenderPass = renderPass; }

	private:
		void CreateFramebuffer(uint32_t index);
		void CreateColorResources();
		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			 VkImage& image, VmaAllocation& outAllocation);

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		void CreateTextureSampler();
		void CreateDepthResources();

		VkFormat FindDepthFormat();
	private:
		VkDevice m_Device;
		VkPhysicalDevice m_PhysicalDevice;

		FramebufferSpecification m_Spec;

		std::vector<VulkanImage> m_FramebufferImages;

		VkImage m_DepthImage;
		VkImageView m_DepthImageView;

		std::vector<VkFramebuffer> m_Framebuffers;
		VkRenderPass m_RenderPass;

		std::vector<VmaAllocation> m_ImageAllocations;
	};
}

