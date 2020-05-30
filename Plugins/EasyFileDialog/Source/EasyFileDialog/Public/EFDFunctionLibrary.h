// Copyright 2017-2020 Firefly Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EFDCore.h"
#include "EFDFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class EASYFILEDIALOG_API UEFDFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

    // Open File Dialog
    UFUNCTION(BlueprintCallable, Category = "EasyFileDialog")
        static bool OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, EEasyFileDialogFlags Flags, TArray< FString >& OutFilenames);

    // Save File Dialog
    UFUNCTION(BlueprintCallable, Category = "EasyFileDialog")
        static bool SaveFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypeDescription, const FString& FileType, EEasyFileDialogFlags Flags, TArray< FString >& OutFilenames);

    // Open Folder Dialog
    UFUNCTION(BlueprintCallable, Category = "EasyFileDialog")
        static bool OpenFolderDialog(const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName);

};
