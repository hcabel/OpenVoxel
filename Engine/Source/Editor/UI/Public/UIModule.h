#pragma once

#include "UI_API.h"
#include "Module.h"

class UI_API UIModule : public Module
{

public:
	//~ Begin Module Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End Module Interface
};
