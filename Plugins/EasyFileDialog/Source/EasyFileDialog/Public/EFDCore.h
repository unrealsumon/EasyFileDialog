// Copyright 2017-2020 Firefly Studio. All Rights Reserved.

#pragma once
#pragma warning(disable : 4668)

// Flag Enum for saving multiple or single file's
UENUM(BlueprintType)
enum EEasyFileDialogFlags
{
	Single = 0x00, // No flags
	Multiple = 0x01  // Allow multiple file selections
};

#include "CoreMinimal.h"

/**
 * 
 */
class EASYFILEDIALOG_API EFDCore
{
public:

	// Open file dialog core, called from the blueprint function library
	static bool OpenFileDialogCore(const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, uint32 Flags, TArray< FString >& OutFilenames);

	// Save file dialog core, called from the blueprint function library
	static bool SaveFileDialogCore(const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, uint32 Flags, TArray< FString >& OutFilenames);

	// Open folder dialog core, called from the blueprint function library
	static bool OpenFolderDialogCore(const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName);

	

private:
	// Both open file dialog and save file dialog using this same function with different save parameter.
	static bool FileDialogShared(bool bSave, const void* ParentWindowHandle, const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, uint32 Flags, TArray<FString>& OutFilenames, int32& OutFilterIndex);
	
	// The main Open folder dialog functionalities
	static bool OpenFolderDialogInner(const void* ParentWindowHandle, const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName);
};
