#include "pch.h"
#include "VulkanFrameBuffer.h"

#include "VulkanContext.h"
#include "SwapChain.h"
#include "VulkanMemoryAllocator.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"

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
		Resize(spec.Width, spec.Height, true);

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
			if (m_Spec.Target == PresentTarget::CustomViewport)
			{
				allocator.DestroyImage(m_FramebufferImages[i].ColorImage, m_ImageAllocations[i]);
				vkDestroyImageView(m_Device, m_FramebufferImages[i].ColorImageView, nullptr);
				vkDestroySampler(m_Device, m_FramebufferImages[i].TextureSampler, nullptr);

				allocator.DestroyImage(m_ObjectIDAttachment[i].ColorImage, m_ObjectIDAllocations[i]);
				vkDestroyImageView(m_Device, m_ObjectIDAttachment[i].ColorImageView, nullptr);
			}

			vkDestroyFramebuffer(m_Device, m_Framebuffers[i], nullptr);
		}
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height, bool force /*= false*/)
	{
		if (!force && (m_Spec.Width == width || m_Spec.Height == height))
			return;

		auto& swapChain = Engine::Get()->GetWindow()->GetSwapChain();
		m_Spec.Width = width;
		m_Spec.Height = height;

		m_FramebufferImages.resize(VulkanAPI::GetMaxFramesInFlight());
		m_ImageAllocations.resize(VulkanAPI::GetMaxFramesInFlight());
		m_Framebuffers.resize(VulkanAPI::GetMaxFramesInFlight());

		if (m_Spec.Target == PresentTarget::CustomViewport)
		{
			m_ObjectIDAllocations.resize(VulkanAPI::GetMaxFramesInFlight());
			m_ObjectIDAttachment.resize(VulkanAPI::GetMaxFramesInFlight());
		}

		for (int i = 0; i < VulkanAPI::GetMaxFramesInFlight(); ++i)
		{
			ResizeOnIndex(width, height, i);
		}
	}

	void VulkanFramebuffer::ResizeOnIndex(uint32_t width, uint32_t height, int index)
	{
		VulkanMemoryAllocator allocator("FrameBuffer Image");
		vkDestroyFramebuffer(m_Device, m_Framebuffers[index], nullptr);

		if (m_Spec.Target == PresentTarget::CustomViewport)
		{
			vkDestroyImageView(m_Device, m_FramebufferImages[index].ColorImageView, nullptr);
			allocator.DestroyImage(m_FramebufferImages[index].ColorImage, m_ImageAllocations[index]);
			vkDestroySampler(m_Device, m_FramebufferImages[index].TextureSampler, nullptr);

			vkDestroyImageView(m_Device, m_ObjectIDAttachment[index].ColorImageView, nullptr);
			allocator.DestroyImage(m_ObjectIDAttachment[index].ColorImage, m_ObjectIDAllocations[index]);
		}

		if (m_Spec.Target == PresentTarget::Swapchain)
		{
			auto& swapChain = Engine::Get()->GetWindow()->GetSwapChain();
			m_FramebufferImages[index].ColorImage = swapChain->GetSwapChainImages().at(index);
			m_FramebufferImages[index].ColorImageView = swapChain->GetSwapChainImageViews().at(index);
			m_FramebufferImages[index].ImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		else
		{
			CreateColorResources(index);
			if (m_Spec.Target == PresentTarget::CustomViewport)
				CreateObjectIDResources(index);
		}

		CreateFramebuffer(index);
	}

	void VulkanFramebuffer::CreateFramebuffer(uint32_t index)
	{
		m_FramebufferImages[index].ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		std::vector<VkImageView> attachments;
		attachments.push_back(m_FramebufferImages[index].ColorImageView);// Color attachment

		if (m_Spec.Target == PresentTarget::CustomViewport)
		{
			m_ObjectIDAttachment[index].ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments.push_back(m_ObjectIDAttachment[index].ColorImageView); // ObjectID attachment
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_Spec.Width;
		framebufferInfo.height = m_Spec.Height;
		framebufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_Framebuffers[index]));
	}

	void VulkanFramebuffer::CreateColorResources(int index)
	{
		auto format = Engine::Get()->GetWindow()->GetSwapChain()->GetSwapChainImageFormat();

		CreateImage(m_Spec.Width, m_Spec.Height, m_Spec.ColorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				m_FramebufferImages[index].ColorImage, m_ImageAllocations[index]);

		m_FramebufferImages[index].ColorImageView = CreateImageView(m_FramebufferImages[index].ColorImage, m_Spec.ColorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		CreateTextureSampler(&m_FramebufferImages[index].TextureSampler);
	}

	void VulkanFramebuffer::CreateObjectIDResources(int index)
	{
		auto format = Engine::Get()->GetWindow()->GetSwapChain()->GetSwapChainImageFormat();

		CreateImage(m_Spec.Width, m_Spec.Height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				m_ObjectIDAttachment[index].ColorImage, m_ObjectIDAllocations[index]);

		m_ObjectIDAttachment[index].ColorImageView = CreateImageView(m_ObjectIDAttachment[index].ColorImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
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

	void VulkanFramebuffer::CreateTextureSampler(VkSampler* sampler)
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

		VK_CHECK_RESULT(vkCreateSampler(VulkanContext::GetVulkanDevice()->GetVkDevice(), &samplerCreateInfo, nullptr, sampler));
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

	void createBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanUtilities::FindMemoryType(memRequirements.memoryTypeBits, properties);
		VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	void copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = VulkanContext::GetVulkanDevice()->BeginSingleTimeCommands();

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.layerCount = 1;
		region.imageExtent = { width, height, 1 };

		vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);

		VulkanContext::GetVulkanDevice()->EndSingleTimeCommands(commandBuffer);
	}

	uint32_t VulkanFramebuffer::ReadPixelsFromImage(uint32_t pixelX, uint32_t pixelY)
	{
		int imageIndex = Engine::Get()->GetWindow()->GetCurrentFrame();

		// Transition the image layout to transfer source optimal
		//VulkanUtilities::TransitionImageLayout(m_ObjectIDAttachment[imageIndex].ColorImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		// Create a staging buffer
		VkDeviceSize imageSize = m_Spec.Width * m_Spec.Height * 4 * sizeof(uint8_t);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(m_Device, imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copy the pixel data from the image to the staging buffer
		copyImageToBuffer(m_ObjectIDAttachment[imageIndex].ColorImage, stagingBuffer, m_Spec.Width, m_Spec.Height);

		// Map the staging buffer memory and access the pixel data
		void* data;
		vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);

		// Access and print the pixel data
		uint32_t* pixels = static_cast<uint32_t*>(data);

		// Calculate the index of the pixel in the linear array
		uint32_t pixelIndex = static_cast<uint32_t>(pixelY) * m_Spec.Width + static_cast<uint32_t>(pixelX);

		if (pixelIndex > m_Spec.Width * m_Spec.Height)
			return 0;

		// Read the pixel value from the array
		uint32_t pixelValue = pixels[pixelIndex];

		glm::vec4 objectID;
		objectID.r = static_cast<float>((pixelValue >> 0) & 0xFF);
		objectID.g = static_cast<float>((pixelValue >> 8) & 0xFF);
		objectID.b = static_cast<float>((pixelValue >> 16) & 0xFF);

		// decode the entity ID 
		uint32_t entityID = 0;
		entityID |= static_cast<uint32_t>(objectID.b) << 16;
		entityID |= static_cast<uint32_t>(objectID.g) << 8;
		entityID |= static_cast<uint32_t>(objectID.r) << 0;

		vkUnmapMemory(m_Device, stagingBufferMemory);
		// Clean up resources
		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

		return entityID;
	}


}