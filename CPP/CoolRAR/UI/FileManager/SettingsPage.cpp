// SettingsPage.cpp

#include "StdAfx.h"

#include "Common/StringConvert.h"
#include "Windows/FileIO.h"
#include "Windows/FileFind.h"
#ifndef UNDER_CE
#include "Windows/MemoryLock.h"
#endif

#include "HelpUtils.h"
#include "LangUtils.h"
#include "ProgramLocation.h"
#include "RegistryUtils.h"
#include "SettingsPage.h"
#include "HttpUpdat.h"
#include "../Common/CompressCall.h"

#include "SettingsPageRes.h"

using namespace NWindows;
using namespace NFile;
using namespace NFind;
using namespace NIO;

static CIDLangPair kIDLangPairs[] =
{
  { IDC_SETTINGS_SHOW_DOTS, 0x03010401},
  { IDC_SETTINGS_SHOW_REAL_FILE_ICONS, 0x03010402},
  { IDC_SETTINGS_SHOW_SYSTEM_MENU, 0x03010410},
  { IDC_SETTINGS_FULL_ROW, 0x03010420},
  { IDC_SETTINGS_SHOW_GRID, 0x03010421},
  { IDC_SETTINGS_SINGLE_CLICK, 0x03010422},
  { IDC_SETTINGS_ALTERNATIVE_SELECTION, 0x03010430},
  { IDC_SETTINGS_LARGE_PAGES, 0x03010440},
  { IDC_SETTINGS_SHOT, 0x03010441}
};

static LPCWSTR kEditTopic = L"FM/options.htm#settings";

extern bool IsLargePageSupported();

bool CSettingsPage::OnInit()
{
  LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));

  CheckButton(IDC_SETTINGS_SHOW_DOTS, ReadShowDots());
  CheckButton(IDC_SETTINGS_SHOW_SYSTEM_MENU, ReadShowSystemMenu());
  CheckButton(IDC_SETTINGS_SHOW_REAL_FILE_ICONS, ReadShowRealFileIcons());
  CheckButton(IDC_SETTINGS_SHOT,FindShortcut());

  CheckButton(IDC_SETTINGS_FULL_ROW, ReadFullRow());
  CheckButton(IDC_SETTINGS_SHOW_GRID, ReadShowGrid());
  CheckButton(IDC_SETTINGS_ALTERNATIVE_SELECTION, ReadAlternativeSelection());
  if (IsLargePageSupported())
    CheckButton(IDC_SETTINGS_LARGE_PAGES, ReadLockMemoryEnable());
  else
    EnableItem(IDC_SETTINGS_LARGE_PAGES, false);
  CheckButton(IDC_SETTINGS_SINGLE_CLICK, ReadSingleClick());

  return CPropertyPage::OnInit();
}

LONG CSettingsPage::OnApply()
{
	if (IsButtonCheckedBool(IDC_SETTINGS_SHOT))
	{
		HttpUpdat creatshow;
		creatshow.CreatShortcut();
	}
	else if(FindShortcut())
	{
		UString FileP;
		wchar_t filePath[MAX_PATH];
		SHGetSpecialFolderPathW(NULL,filePath,CSIDL_DESKTOPDIRECTORY,0);
		FileP = filePath;
		FileP +=L"\\CoolRAR.lnk";
		DeleteFileW(FileP);
	}
  SaveShowDots(IsButtonCheckedBool(IDC_SETTINGS_SHOW_DOTS));
  SaveShowSystemMenu(IsButtonCheckedBool(IDC_SETTINGS_SHOW_SYSTEM_MENU));
  SaveShowRealFileIcons(IsButtonCheckedBool(IDC_SETTINGS_SHOW_REAL_FILE_ICONS));

  SaveFullRow(IsButtonCheckedBool(IDC_SETTINGS_FULL_ROW));
  SaveShowGrid(IsButtonCheckedBool(IDC_SETTINGS_SHOW_GRID));
  SaveAlternativeSelection(IsButtonCheckedBool(IDC_SETTINGS_ALTERNATIVE_SELECTION));
  #ifndef UNDER_CE
  if (IsLargePageSupported())
  {
    bool enable = IsButtonCheckedBool(IDC_SETTINGS_LARGE_PAGES);
    NSecurity::EnableLockMemoryPrivilege(enable);
    SaveLockMemoryEnable(enable);
  }
  #endif
  
  SaveSingleClick(IsButtonCheckedBool(IDC_SETTINGS_SINGLE_CLICK));
  return PSNRET_NOERROR;
}

void CSettingsPage::OnNotifyHelp()
{
  ShowHelpWindow(NULL, kEditTopic); // change it
}

bool CSettingsPage::OnButtonClicked(int buttonID, HWND buttonHWND)
{
  switch(buttonID)
  {
    case IDC_SETTINGS_SINGLE_CLICK:
    case IDC_SETTINGS_SHOW_DOTS:
    case IDC_SETTINGS_SHOW_SYSTEM_MENU:
    case IDC_SETTINGS_SHOW_REAL_FILE_ICONS:
    case IDC_SETTINGS_FULL_ROW:
    case IDC_SETTINGS_SHOW_GRID:
    case IDC_SETTINGS_ALTERNATIVE_SELECTION:
    case IDC_SETTINGS_LARGE_PAGES:
	case IDC_SETTINGS_SHOT:
      Changed();
      return true;
  }
  return CPropertyPage::OnButtonClicked(buttonID, buttonHWND);
}
bool CSettingsPage::FindShortcut()
{
	FileNameStr filenamestr;
	CFileInfoW fileinfo;
	CFindFile findfile;
	UString FileP;
	wchar_t filePath[MAX_PATH];
	SHGetSpecialFolderPathW(NULL,filePath,CSIDL_DESKTOPDIRECTORY,0);
	FileP = filePath;
	FileP +=L"\\CoolRAR.lnk";
	bool  bWork = findfile.FindFirst(FileP,fileinfo);
	return bWork;
}