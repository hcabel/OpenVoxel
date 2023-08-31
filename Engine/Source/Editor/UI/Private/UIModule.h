#pragma once

#include "Module.h"

class UIModule : public Module
{

public:
	//~ Begin Module Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End Module Interface
};
