// Copyright 2017-2020 Firefly Studio. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FEasyFileDialogModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
