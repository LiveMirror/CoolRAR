// RootFolder.cpp

#include "StdAfx.h"

#include "Common/StringConvert.h"

#include "Windows/DLL.h"
#include "FSFolder.h"
#include "LangUtils.h"
#include "RootFolder.h"
#include "../FileManager/SysIconUtils.h"

#include "resource.h"

using namespace NWindows;

	


UString RootFolder_GetName_Computer(int &iconIndex)
{
  #ifdef UNDER_CE
  GetRealIconIndex(L"\\", FILE_ATTRIBUTE_DIRECTORY, iconIndex);
  #else
  iconIndex = GetIconIndexForCSIDL(CSIDL_DRIVES);
  #endif
  return LangString(IDS_COMPUTER, 0x03020300);
}

UString RootFolder_GetName_Network(int &iconIndex)
{
  iconIndex = GetIconIndexForCSIDL(CSIDL_NETWORK);
  return LangString(IDS_NETWORK, 0x03020301);
}

UString RootFolder_GetName_Documents(int &iconIndex)
{
  iconIndex = GetIconIndexForCSIDL(CSIDL_PERSONAL);
  return LangString(IDS_DOCUMENTS, 0x03020302); ;
}

UString RootFolder_GetName_Desktop(int &iconIndex)
{
	iconIndex = GetIconIndexForCSIDL(CSIDL_DESKTOP);
	return L"×ÀÃæ";
}

typedef BOOL (WINAPI *SHGetSpecialFolderPathWp)(HWND hwnd, LPWSTR pszPath, int csidl, BOOL fCreate);
typedef BOOL (WINAPI *SHGetSpecialFolderPathAp)(HWND hwnd, LPSTR pszPath, int csidl, BOOL fCreate);

UString GetMyDesktopPath()
{
	UString us;
	WCHAR s[MAX_PATH + 1];
	SHGetSpecialFolderPathWp getW = (SHGetSpecialFolderPathWp)
#ifdef UNDER_CE
		My_GetProcAddress(GetModuleHandle(TEXT("coredll.dll")), "SHGetSpecialFolderPath");
#else
		My_GetProcAddress(GetModuleHandle(TEXT("shell32.dll")), "SHGetSpecialFolderPathW");
#endif
	if (getW && getW(0, s, CSIDL_DESKTOP, FALSE))
		us = s;
#ifndef _UNICODE
	else
	{
		SHGetSpecialFolderPathAp getA = (SHGetSpecialFolderPathAp)
			::GetProcAddress(::GetModuleHandleA("shell32.dll"), "SHGetSpecialFolderPathA");
		CHAR s2[MAX_PATH + 1];
		if (getA && getA(0, s2, CSIDL_DESKTOP, FALSE))
			us = GetUnicodeString(s2);
	}
#endif
	if (us.Length() > 0 && us[us.Length() - 1] != WCHAR_PATH_SEPARATOR)
		us += WCHAR_PATH_SEPARATOR;
	return us;
}

UString GetMyDocumentsPath()
{
	UString us;
	WCHAR s[MAX_PATH + 1];
	SHGetSpecialFolderPathWp getW = (SHGetSpecialFolderPathWp)
#ifdef UNDER_CE
		My_GetProcAddress(GetModuleHandle(TEXT("coredll.dll")), "SHGetSpecialFolderPath");
#else
		My_GetProcAddress(GetModuleHandle(TEXT("shell32.dll")), "SHGetSpecialFolderPathW");
#endif
	if (getW && getW(0, s, CSIDL_PERSONAL, FALSE))
		us = s;
#ifndef _UNICODE
	else
	{
		SHGetSpecialFolderPathAp getA = (SHGetSpecialFolderPathAp)
			::GetProcAddress(::GetModuleHandleA("shell32.dll"), "SHGetSpecialFolderPathA");
		CHAR s2[MAX_PATH + 1];
		if (getA && getA(0, s2, CSIDL_PERSONAL, FALSE))
			us = GetUnicodeString(s2);
	}
#endif
	if (us.Length() > 0 && us[us.Length() - 1] != WCHAR_PATH_SEPARATOR)
		us += WCHAR_PATH_SEPARATOR;
	return us;
}