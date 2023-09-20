#pragma once

#include "UI_API.h"

#include <vulkan/vulkan.hpp>

/**
 * @brief This class represents an image that can be displayed in the UI.
 */
class UI_API UIImage
{
public:
	UIImage(vk::ImageCreateInfo imageCreateInfo, vk::ImageViewCreateInfo viewCreateInfo, vk::SamplerCreateInfo samplerCreateInfo);
	UIImage(UIImage&& other) noexcept
		: m_ImageCreateInfo(std::move(other.m_ImageCreateInfo)),
		m_ViewCreateInfo(std::move(other.m_ViewCreateInfo)),
		m_Sampler(std::move(other.m_Sampler)),
		m_Image(std::move(other.m_Image)),
		m_ImageView(std::move(other.m_ImageView)),
		m_ImageMemory(std::move(other.m_ImageMemory)),
		m_Descriptor(std::move(other.m_Descriptor))
	{
		other.m_Image = nullptr;
		other.m_ImageView = nullptr;
		other.m_Descriptor = nullptr;
	}
	UIImage& operator=(UIImage&& rhs) noexcept
	{
		m_ImageCreateInfo = std::move(rhs.m_ImageCreateInfo);
		m_ViewCreateInfo = std::move(rhs.m_ViewCreateInfo);
		m_Sampler = std::move(rhs.m_Sampler);
		m_Image = std::move(rhs.m_Image);
		m_ImageView = std::move(rhs.m_ImageView);
		m_ImageMemory = std::move(rhs.m_ImageMemory);
		m_Descriptor = std::move(rhs.m_Descriptor);

		rhs.m_Image = nullptr;
		rhs.m_ImageView = nullptr;
		rhs.m_ImageMemory = nullptr;
		rhs.m_Descriptor = nullptr;

		return *this;
	}
	~UIImage();

public:
	void Resize(uint32_t width, uint32_t height);

public:
	__forceinline operator vk::Image() const { return GetImage(); }
	__forceinline operator vk::ImageView() const { return GetView(); }
	__forceinline operator vk::DescriptorSet() const { return m_Descriptor; }
	__forceinline operator VkDescriptorSet() const { return m_Descriptor; }
	__forceinline operator intptr_t() const { return (reinterpret_cast<intptr_t>(static_cast<VkDescriptorSet>(m_Descriptor))); }

public:
	__forceinline const vk::Image& GetImage() const { return m_Image; }
	__forceinline const vk::ImageView& GetView() const { return m_ImageView; }

protected:
	vk::ImageCreateInfo m_ImageCreateInfo;
	vk::ImageViewCreateInfo m_ViewCreateInfo;

	vk::Sampler m_Sampler;
	vk::Image m_Image;
	vk::ImageView m_ImageView;
	vk::DeviceMemory m_ImageMemory;
	vk::DescriptorSet m_Descriptor;
};
