#pragma once

#include "Vulkan_API.h"

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>

/**
 * This class handle a pnext linked chain.
 * The purpose of this class is to create a chain of pNext pointer to be used in the vulkan API.
 */
class VULKAN_API VulkanPNextChain final
{

public:
	VulkanPNextChain(const VulkanPNextChain& rhs)
	{
		m_ChainStart = rhs.m_ChainStart;
		m_ChainEnd = rhs.m_ChainEnd;
	}

	VulkanPNextChain& operator=(const VulkanPNextChain& rhs)
	{
		m_ChainStart = rhs.m_ChainStart;
		m_ChainEnd = rhs.m_ChainEnd;
		return (*this);
	}

public:
	VulkanPNextChain() = default;
	VulkanPNextChain(std::nullptr_t) : VulkanPNextChain() {}

	struct ChainElement
	{
		uint32_t sType = 0;
		ChainElement* pNext = nullptr;
	};

	void PushBack(void* entry)
	{
		ChainElement* newElement = reinterpret_cast<ChainElement*>(entry);
		if (m_ChainStart == nullptr)
		{
			m_ChainStart = newElement;
			m_ChainEnd = newElement;
		}
		else
		{
			m_ChainEnd->pNext = newElement;
			m_ChainEnd = newElement;
		}
		m_ChainEnd->pNext = nullptr;
	}

	ChainElement* Begin() const { return m_ChainStart; }
	ChainElement* End() const { return m_ChainEnd; }

private:
	ChainElement* m_ChainStart = nullptr;
	ChainElement* m_ChainEnd = nullptr;
};
