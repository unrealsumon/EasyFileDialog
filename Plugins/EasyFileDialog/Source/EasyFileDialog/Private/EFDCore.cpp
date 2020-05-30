// Copyright 2017-2020 Firefly Studio. All Rights Reserved.


#include "EFDCore.h"

#include "shlobj.h" 

#include <Runtime\Core\Public\HAL\FileManager.h>
#include <Runtime\Core\Public\Misc\Paths.h>
#include <Runtime\Core\Public\Windows\COMPointer.h>


#define MAX_FILETYPES_STR 4096
#define MAX_FILENAME_STR 65536 // This buffer has to be big enough to contain the names of all the selected files as well as the null characters between them and the null character at the end

bool EFDCore::OpenFileDialogCore(const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, uint32 Flags, TArray< FString >& OutFilenames)
{
	// Calling the FileDialogShared function using save parameter with false. 
	int OutFilterIndex=0;
	return FileDialogShared(false, nullptr, DialogTitle, DefaultPath, DefaultFile, FileTypes, Flags, OutFilenames,OutFilterIndex);
}

bool EFDCore::SaveFileDialogCore(const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, uint32 Flags, TArray< FString >& OutFilenames)
{
	// Calling the FileDialogShared function using save parameter. 
	int OutFilterIndex = 0;
	return FileDialogShared(true, nullptr, DialogTitle, DefaultPath, DefaultFile, FileTypes, Flags, OutFilenames, OutFilterIndex);
}

bool EFDCore::OpenFolderDialogCore(const FString& DialogTitle, const FString& DefaultPath, FString& OutFoldername)
{
	// Calling the main open folder dialog function.
	return OpenFolderDialogInner(NULL, DialogTitle, DefaultPath, OutFoldername);
}

bool EFDCore::FileDialogShared(bool bSave, const void* ParentWindowHandle, const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, uint32 Flags, TArray<FString>& OutFilenames, int32& OutFilterIndex)
{
#pragma region Windows
	//FScopedSystemModalMode SystemModalScope;
#if PLATFORM_WINDOWS
	WCHAR Filename[MAX_FILENAME_STR];
	FCString::Strcpy(Filename, MAX_FILENAME_STR, *(DefaultFile.Replace(TEXT("/"), TEXT("\\"))));

	// Convert the forward slashes in the path name to backslashes, otherwise it'll be ignored as invalid and use whatever is cached in the registry
	WCHAR Pathname[MAX_FILENAME_STR];
	FCString::Strcpy(Pathname, MAX_FILENAME_STR, *(FPaths::ConvertRelativePathToFull(DefaultPath).Replace(TEXT("/"), TEXT("\\"))));

	// Convert the "|" delimited list of filetypes to NULL delimited then add a second NULL character to indicate the end of the list
	WCHAR FileTypeStr[MAX_FILETYPES_STR];
	WCHAR* FileTypesPtr = NULL;
	const int32 FileTypesLen = FileTypes.Len();

	// Nicely formatted file types for lookup later and suitable to append to filenames without extensions
	TArray<FString> CleanExtensionList;

	// The strings must be in pairs for windows.
	// It is formatted as follows: Pair1String1|Pair1String2|Pair2String1|Pair2String2
	// where the second string in the pair is the extension.  To get the clean extensions we only care about the second string in the pair
	TArray<FString> UnformattedExtensions;
	FileTypes.ParseIntoArray(UnformattedExtensions, TEXT("|"), true);
	for (int32 ExtensionIndex = 1; ExtensionIndex < UnformattedExtensions.Num(); ExtensionIndex += 2)
	{
		const FString& Extension = UnformattedExtensions[ExtensionIndex];
		// Assume the user typed in an extension or doesnt want one when using the *.* extension. We can't determine what extension they wan't in that case
		if (Extension != TEXT("*.*"))
		{
			// Add to the clean extension list, first removing the * wildcard from the extension
			int32 WildCardIndex = Extension.Find(TEXT("*"));
			CleanExtensionList.Add(WildCardIndex != INDEX_NONE ? Extension.RightChop(WildCardIndex + 1) : Extension);
		}
	}

	if (FileTypesLen > 0 && FileTypesLen - 1 < MAX_FILETYPES_STR)
	{
		FileTypesPtr = FileTypeStr;
		FCString::Strcpy(FileTypeStr, MAX_FILETYPES_STR, *FileTypes);

		TCHAR* Pos = FileTypeStr;
		while (Pos[0] != 0)
		{
			if (Pos[0] == '|')
			{
				Pos[0] = 0;
			}

			Pos++;
		}

		// Add two trailing NULL characters to indicate the end of the list
		FileTypeStr[FileTypesLen] = 0;
		FileTypeStr[FileTypesLen + 1] = 0;
	}

	OPENFILENAME ofn;
	FMemory::Memzero(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)ParentWindowHandle;
	ofn.lpstrFilter = FileTypesPtr;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = Filename;
	ofn.nMaxFile = MAX_FILENAME_STR;
	ofn.lpstrInitialDir = Pathname;
	ofn.lpstrTitle = *DialogTitle;
	if (FileTypesLen > 0)
	{
		ofn.lpstrDefExt = &FileTypeStr[0];
	}

	ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_EXPLORER;

	if (bSave)
	{
		ofn.Flags |= OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_NOVALIDATE;
	}
	else
	{
		ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	}

	if (Flags & EEasyFileDialogFlags::Multiple)
	{
		ofn.Flags |= OFN_ALLOWMULTISELECT;
	}

	bool bSuccess;
	if (bSave)
	{
		bSuccess = !!::GetSaveFileName(&ofn);
	}
	else
	{
		bSuccess = !!::GetOpenFileName(&ofn);
	}

	if (bSuccess)
	{
		// GetOpenFileName/GetSaveFileName changes the CWD on success. Change it back immediately.
		//FPlatformProcess::SetCurrentWorkingDirectoryToBaseDir();

		if (Flags & EEasyFileDialogFlags::Multiple)
		{
			// When selecting multiple files, the returned string is a NULL delimited list
			// where the first element is the directory and all remaining elements are filenames.
			// There is an extra NULL character to indicate the end of the list.
			FString DirectoryOrSingleFileName = FString(Filename);
			TCHAR* Pos = Filename + DirectoryOrSingleFileName.Len() + 1;

			if (Pos[0] == 0)
			{
				// One item selected. There was an extra trailing NULL character.
				OutFilenames.Add(DirectoryOrSingleFileName);
			}
			else
			{
				// Multiple items selected. Keep adding filenames until two NULL characters.
				FString SelectedFile;
				do
				{
					SelectedFile = FString(Pos);
					new(OutFilenames) FString(DirectoryOrSingleFileName / SelectedFile);
					Pos += SelectedFile.Len() + 1;
				} while (Pos[0] != 0);
			}
		}
		else
		{
			new(OutFilenames) FString(Filename);
		}

		// The index of the filter in OPENFILENAME starts at 1.
		OutFilterIndex = ofn.nFilterIndex - 1;

		// Get the extension to add to the filename (if one doesnt already exist)
		FString Extension = CleanExtensionList.IsValidIndex(OutFilterIndex) ? CleanExtensionList[OutFilterIndex] : TEXT("");

		// Make sure all filenames gathered have their paths normalized and proper extensions added
		for (auto OutFilenameIt = OutFilenames.CreateIterator(); OutFilenameIt; ++OutFilenameIt)
		{
			FString& OutFilename = *OutFilenameIt;

			OutFilename = IFileManager::Get().ConvertToRelativePath(*OutFilename);

			if (FPaths::GetExtension(OutFilename).IsEmpty() && !Extension.IsEmpty())
			{
				// filename does not have an extension. Add an extension based on the filter that the user chose in the dialog
				OutFilename += Extension;
			}

			FPaths::NormalizeFilename(OutFilename);
		}
	}
	else
	{
		uint32 Error = ::CommDlgExtendedError();
		if (Error != ERROR_SUCCESS)
		{
			//UE_LOG(LogDesktopPlatform, Warning, TEXT("Error reading results of file dialog. Error: 0x%04X"), Error);
		}
	}

	return bSuccess;
#endif
#pragma endregion

#pragma region LINUX
#if PLATFORM_LINUX
	return false;
#endif
#pragma endregion
	return false;
}

bool EFDCore:: OpenFolderDialogInner(const void* ParentWindowHandle, const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName)
{
	//FScopedSystemModalMode SystemModalScope;

	bool bSuccess = false;

	TComPtr<IFileOpenDialog> FileDialog;
	if (SUCCEEDED(::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&FileDialog))))
	{
		// Set this up as a folder picker
		{
			DWORD dwFlags = 0;
			FileDialog->GetOptions(&dwFlags);
			FileDialog->SetOptions(dwFlags | FOS_PICKFOLDERS);
		}

		// Set up common settings
		FileDialog->SetTitle(*DialogTitle);
		if (!DefaultPath.IsEmpty())
		{
			// SHCreateItemFromParsingName requires the given path be absolute and use \ rather than / as our normalized paths do
			FString DefaultWindowsPath = FPaths::ConvertRelativePathToFull(DefaultPath);
			DefaultWindowsPath.ReplaceInline(TEXT("/"), TEXT("\\"), ESearchCase::CaseSensitive);

			TComPtr<IShellItem> DefaultPathItem;
			if (SUCCEEDED(::SHCreateItemFromParsingName(*DefaultWindowsPath, nullptr, IID_PPV_ARGS(&DefaultPathItem))))
			{
				FileDialog->SetFolder(DefaultPathItem);
			}
		}

		// Show the picker
		if (SUCCEEDED(FileDialog->Show((HWND)ParentWindowHandle)))
		{
			TComPtr<IShellItem> Result;
			if (SUCCEEDED(FileDialog->GetResult(&Result)))
			{
				PWSTR pFilePath = nullptr;
				if (SUCCEEDED(Result->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath)))
				{
					bSuccess = true;

					OutFolderName = pFilePath;
					FPaths::NormalizeDirectoryName(OutFolderName);

					::CoTaskMemFree(pFilePath);
				}
			}
		}
	}

	return bSuccess;
}