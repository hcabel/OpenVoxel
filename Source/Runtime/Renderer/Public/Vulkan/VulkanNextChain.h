#pragma once

#include "RendererModule.h"

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>

/**
 * This class contain a chain linked between them with the pNext pointer.
 * The purpose of this class is to create a chain of pNext pointer to be used in the vulkan API.
 */
class RENDERER_API VulkanNextChain final
{
public:
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
