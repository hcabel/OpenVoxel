#include "VulkanModule.h"
#include "MacrosHelper.h"
#include "VulkanContext.h"

IMPLEMENT_MODULE(VulkanModule)

void VulkanModule::StartupModule()
{
}

void VulkanModule::ShutdownModule()
{
	VulkanContext::Get().Destroy();
}
