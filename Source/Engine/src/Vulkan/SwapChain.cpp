#include "pch.h"
#include "SwapChain.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "Core/Engine.h"
#include <GLFW/glfw3.h>
#include "RenderPass.h"
#include "VulkanRenderer.h"
#include "Renderer/Renderer.h"
#include "CommandBuffer.h"
#include "Utilities/Timer.h"
#include "VulkanFramebuffer.h"


namespace PAL
{
	VulkanSwapChain::VulkanSwapChain(const SharedPtr<VulkanDevice>& device)
		: m_Device(device)
		, m_ImageIndex(0)
	{
	}

	VulkanSwapChain::~VulkanSwapChain()
	{

	}

	void VulkanSwapChain::Create()
	{
		auto phyDevice = m_Device->GetPhysicalDevice()->GetVkPhysicalDevice();
		auto surface = VulkanContext::GetSurface();

		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(phyDevice, surface);
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes, Engine::Get()->GetWindow()->IsVsyncOn());
		m_SwapChainExtent = ChooseSwapExtent(swapChainSupport.capabilities);

		// it is recommended to request at least one more image than the minimum
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageExtent = m_SwapChainExtent;

		m_SwapChainImageFormat = surfaceFormat.format;

		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = m_Device->GetPhysicalDevice()->FindQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		// alpha channel
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK_RESULT(vkCreateSwapchainKHR(m_Device->GetVkDevice(), &createInfo, nullptr, &m_SwapChain));

		// Retrieving the swap chain images
		vkGetSwapchainImagesKHR(m_Device->GetVkDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device->GetVkDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

		CreateSyncObjects();
	}

	void VulkanSwapChain::CreateSyncObjects()
	{
		m_ImageAvailableSemaphore.resize(VulkanAPI::GetMaxFramesInFlight());
		m_RenderFinishedSemaphore.resize(VulkanAPI::GetMaxFramesInFlight());
		m_InFlightFence.resize(VulkanAPI::GetMaxFramesInFlight());

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		//fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// Each Frame has it's own Semaphores and Fences
		for (size_t i = 0; i < VulkanAPI::GetMaxFramesInFlight(); i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore[i]));
			VK_CHECK_RESULT(vkCreateSemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore[i]));
			VK_CHECK_RESULT(vkCreateFence(VulkanContext::GetVulkanDevice()->GetVkDevice(), &fenceInfo, nullptr, &m_InFlightFence[i]));
		}
	}

	SwapChainSupportDetails VulkanSwapChain::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			//if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			//{
			//	return availableFormat;
			//}

			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync)
	{
		/*
		*	VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away, which may result in tearing.

		*	VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from the front of the queue when the display is refreshed and the program inserts rendered images at the back of the queue.
				If the queue is full then the program has to wait. This is most similar to vertical sync as found in modern games.
				The moment that the display is refreshed is known as "vertical blank".

		*	VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank.
					Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing.

		*	VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode.
				Instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones. This mode can be used to render frames as fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync. This is commonly known as "triple buffering", although the existence of three buffers alone does not necessarily mean that the framerate is unlocked.
		*/
		VkPresentModeKHR swapchainPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		if (vsync)
		{
			return swapchainPresentMode;
		}

		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}

			if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		return swapchainPresentMode;
	}

	VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			Engine::Get()->GetWindow()->GetFramebufferSize(width, height);
			Debug::LogWarn("{0} {1}", width, height);

			VkExtent2D actualExtent =
			{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			//
			actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	uint32_t VulkanSwapChain::AcquireNextImage(const SharedPtr<RenderPass>& renderpass)
	{
		m_ImageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(m_Device->GetVkDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphore[VulkanAPI::s_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return m_ImageIndex;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			ASSERT(false, "failed to acquire swap chain image!");
		}

		return m_ImageIndex;
	}

	void VulkanSwapChain::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());
		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapChainImages[i];


			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VK_CHECK_RESULT(vkCreateImageView(m_Device->GetVkDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]));
		}
	}

	void VulkanSwapChain::CleanupSwapChain()
	{
		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			vkDestroyImageView(m_Device->GetVkDevice(), m_SwapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(m_Device->GetVkDevice(), m_SwapChain, nullptr);

		//Delete Semaphores
		for (size_t i = 0; i < VulkanAPI::GetMaxFramesInFlight(); i++)
		{
			vkDestroySemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_RenderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_ImageAvailableSemaphore[i], nullptr);
			vkDestroyFence(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_InFlightFence[i], nullptr);
		}

	}

	void VulkanSwapChain::RecreateSwapChain()
	{
		const UniquePtr<Window>& window = Engine::Get()->GetWindow();
		
		int width, height;
		window->GetFramebufferSize(width, height);
		while (width == 0 || height == 0)
		{
			window->GetFramebufferSize(width, height);
			window->WaitEvents();
		}

		Debug::Log("RecreateSwapChain\n--------------------------------------------------------------------------");

		// wait for resources
		vkDeviceWaitIdle(m_Device->GetVkDevice());

		// Delete old swapchain
		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			vkDestroyImageView(m_Device->GetVkDevice(), m_SwapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(m_Device->GetVkDevice(), m_SwapChain, nullptr);

		//Delete Semaphores
		for (size_t i = 0; i < VulkanAPI::GetMaxFramesInFlight(); i++)
		{
			vkDestroySemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_RenderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_ImageAvailableSemaphore[i], nullptr);
			vkDestroyFence(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_InFlightFence[i], nullptr);
		}

		Create();
		CreateImageViews();

		for (auto& callback : m_ResizeCallbacks)
		{
			callback(width, height);
		}
	}

	void VulkanSwapChain::PresentFrame(const SharedPtr<CommandBuffer>& commandBuffer)
	{
		auto& commandBuffers = commandBuffer->GetCommandBuffers();
		uint32_t& currentFrame = VulkanAPI::s_CurrentFrame; // by ref because we want to increment the value

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
	
		{
			CORE_PROFILER_SCOPE("GPU Frame");

			VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::GetVulkanDevice()->GetVkGraphicsQueue(), 1, &submitInfo, m_InFlightFence[currentFrame])); // execution of the recorded commands
			VK_CHECK_RESULT(vkWaitForFences(m_Device->GetVkDevice(), 1, &m_InFlightFence[currentFrame], VK_TRUE, UINT64_MAX)); // wait for GPU to complete
			VK_CHECK_RESULT(vkResetFences(VulkanContext::GetVulkanDevice()->GetVkDevice(), 1, &m_InFlightFence[currentFrame])); // set Fence to unsignaled state 
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_ImageIndex;
		VkResult result;

		// Presenting the actual frame 
		{
			CORE_PROFILER_SCOPE("vkQueuePresentKHR");
			result = vkQueuePresentKHR(VulkanContext::GetVulkanDevice()->GetVkPresentQueue(), &presentInfo);
			VK_CHECK_RESULT(result);
		}

		{
			CORE_PROFILER_SCOPE("Check_RecreateSwapChain");

			bool needRecreate = result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || VulkanAPI::s_ResizeFrameBuffer || VulkanAPI::s_RecreateSwapChainRequested;
			if (needRecreate)
			{
				RecreateSwapChain();

				// reset
				VulkanAPI::s_ResizeFrameBuffer = false;
				VulkanAPI::s_RecreateSwapChainRequested = false;
			}
		}

		currentFrame = (currentFrame + 1) % VulkanAPI::GetMaxFramesInFlight();
	}

	void VulkanSwapChain::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		CORE_PROFILER_FUNC();

		VkCommandBuffer commandBuffer = m_Device->BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else
		{
			ASSERT(false, "unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		m_Device->EndSingleTimeCommands(commandBuffer);
	}


	

}