#pragma once

#include "Module.h"

class VulkanModule : public Module
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};