// AdvancedPage.cpp

#include "StdAfx.h"
#include "FMAdvancedPage.h"
#include "FMAdvancedPageRes.h"
#include "../Common/ZipRegistry.h"
#include "../FileManager/HttpUpdat.h"
#include "Windows/Synchronization.h"
#include "Windows/Registry.h"
#include "LangUtils.h"
using namespace NWindows;
using namespace NRegistry;
static CIDLangPair kIDLangPairs[] =
{
	{ IDD_PAGE_FMADVANCED, 0x04000572 },
	{ IDC_ADV_PAGE_GROUP_TIME, 0x04000573 },
	{ IDC_ADV_PAGE_BOX_TIME_ONE, 0x04000574 },
	{ IDC_ADV_PAGE_BOX_TIME_TWO, 0x04000575 },
	{ IDC_ADV_PAGE_BOX_TIMEE_THREE, 0x04000576 },
	{ IDC_ADV_PAGE_GROUP_PATH, 0x04000577 },
	{ IDC_ADV_PAGE_BOX_PATH_ONE, 0x04000578 },
	{ IDC_ADV_PAGE_BOX_PATH_TWO, 0x04000579 },
	{ IDC_ADV_PAGE_BOX_PATH_THREE, 0x04000580 },
	{ IDC_ADV_PAGE_GROUP_PROPTREY, 0x04000581 },
	{ IDC_ADV_PAGE_BOX_PROPTREY_ONE, 0x04000582 },
	{ IDC_ADV_PAGE_BOX_PROPTREY_TWO, 0x04000583 },
	{ IDC_ADV_PAGE_BOX_PROPTREY_THREE, 0x04000584 },
	{ IDC_ADV_PAGE_GROUP_DEL, 0x04000585 },
	{ IDC_ADV_PAGE_BOX_DEL_ONE, 0x04000586 },
	{ IDC_ADV_PAGE_BOX_DEL_TWO, 0x04000587 },
	{ IDC_ADV_PAGE_BOX_DEL_THREE, 0x04000588 },
	{ IDC_ADV_PAGE_GROUP_OTHER, 0x04000589 },
	{ IDC_ADV_PAGE_BOX_OTHER_ONE, 0x04000590 },
	{ IDC_ADV_PAGE_BOX_OTHER_TWO, 0x04000591 }
};

extern HttpUpdat httpworkdat;
static NSynchronization::CCriticalSection g_CS;

static const TCHAR *kKeyName = TEXT("Extraction");
static const TCHAR *kDeleteMode = TEXT("DeleteSourceFile");;
static const TCHAR *kCuPrefix = TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR") TEXT(STRING_PATH_SEPARATOR);

static CSysString GetKeyPath(const CSysString &path) { return kCuPrefix + path; }

static LONG OpenMainKey(CKey &key, LPCTSTR keyName)
{
	return key.Open(HKEY_CURRENT_USER, GetKeyPath(keyName), KEY_READ);
}

static LONG CreateMainKey(CKey &key, LPCTSTR keyName)
{
	return key.Create(HKEY_CURRENT_USER, GetKeyPath(keyName));
}
bool CAdvancedPage::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000572);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	deletemode =1;//读取注册表中删除源文件的信息
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	if (OpenMainKey(key, kKeyName) == ERROR_SUCCESS)
	
	{   
		UInt32 deletevalue;
		if( key.QueryValue(kDeleteMode,deletevalue) != ERROR_SUCCESS )
		{
			deletemode =1;
		}
		else
			deletemode = deletevalue;
	}
	switch(deletemode)
	{
	case 1:
		CheckButton(IDC_ADV_PAGE_BOX_DEL_ONE,true);
		break;
	case 2:
		CheckButton(IDC_ADV_PAGE_BOX_DEL_TWO,true);
		break;
	case 3:
		CheckButton(IDC_ADV_PAGE_BOX_DEL_THREE,true);
		break;
	default:break;
	}

		CheckButton(IDC_ADV_PAGE_BOX_TIME_ONE,true);
		CheckButton(IDC_ADV_PAGE_BOX_PROPTREY_TWO,true);
		CheckButton(IDC_ADV_PAGE_BOX_OTHER_ONE,true);
	
	 switch(PathMode)
	 {
	 	case NExtract::NPathMode::kFullPathnames:
	 			CheckRadioButton(IDC_ADV_PAGE_BOX_PATH_ONE,IDC_ADV_PAGE_BOX_PATH_THREE,IDC_ADV_PAGE_BOX_PATH_ONE);
	 			break;
		case  NExtract::NPathMode::kNoPathnames:
	 			CheckRadioButton(IDC_ADV_PAGE_BOX_PATH_ONE,IDC_ADV_PAGE_BOX_PATH_THREE,IDC_ADV_PAGE_BOX_PATH_TWO);
	 			break;
	 	case  NExtract::NPathMode::kCurrentPathnames:
	 			CheckRadioButton(IDC_ADV_PAGE_BOX_PATH_ONE,IDC_ADV_PAGE_BOX_PATH_THREE,IDC_ADV_PAGE_BOX_PATH_THREE);
				break;
		default:break;
	}

 return COptionPage::OnInit();
}


bool CAdvancedPage::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_ADV_PAGE_BOX_PATH_ONE:
		PathMode = NExtract::NPathMode::kFullPathnames;
		return true;
	case IDC_ADV_PAGE_BOX_PATH_TWO:
		PathMode = NExtract::NPathMode::kNoPathnames;
		return true;
	case IDC_ADV_PAGE_BOX_PATH_THREE:
		PathMode = NExtract::NPathMode::kCurrentPathnames;
		return true;
	case IDC_ADV_PAGE_BOX_DEL_ONE:
		if (deletemode != 1)
		{
			deletemode =1;
			NSynchronization::CCriticalSectionLock lock(g_CS);
			CKey key;
			CreateMainKey(key, kKeyName);
			key.SetValue(kDeleteMode, (UInt32)deletemode);
		
		}
		return true;
	case IDC_ADV_PAGE_BOX_DEL_TWO:
		if (deletemode != 2)
		{
			deletemode =2;
			NSynchronization::CCriticalSectionLock lock(g_CS);
			CKey key;
			CreateMainKey(key, kKeyName);
			key.SetValue(kDeleteMode, (UInt32)deletemode);
		}
		return true;
	case IDC_ADV_PAGE_BOX_DEL_THREE:
		if (deletemode != 3)
		{
			deletemode =3;
			UInt32 deletevalue = 3;
			NSynchronization::CCriticalSectionLock lock(g_CS);
			CKey key;
			CreateMainKey(key, kKeyName);
			key.SetValue(kDeleteMode, deletevalue);
		}
		return true;
	}
	return COptionPage::OnButtonClicked(buttonID, buttonHWND);
}