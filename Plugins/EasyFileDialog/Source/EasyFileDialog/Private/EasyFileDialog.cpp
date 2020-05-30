// Copyright 2017-2020 Firefly Studio. All Rights Reserved.

#include "EasyFileDialog.h"

#define LOCTEXT_NAMESPACE "FEasyFileDialogModule"

void FEasyFileDialogModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FEasyFileDialogModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEasyFileDialogModule, EasyFileDialog)