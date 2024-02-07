#pragma once

namespace PAL
{
	class DescriptorSet
	{

	public:
		DescriptorSet();
		void Shutdown();

		void CreateStorageBuffer(VkBuffer& buffer);
		void Bind(VkPipelineLayout pipelineLayout, size_t frameIndex);

	private:
		std::vector<VkDescriptorSet> m_DescriptorSets;

		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;

		std::vector<VkBuffer> m_StorageBuffers;
	};
}
