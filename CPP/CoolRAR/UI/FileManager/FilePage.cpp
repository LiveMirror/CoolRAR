#include "StdAfx.h"
#include "FilePage.h"
#include "FilePageRes.h"
#include "LangUtils.h"
using namespace NWindows;
static CIDLangPair kIDLangPairs[] =
{
	{ IDD_PAGE_FILE, 0x04000534 },
	{ IDC_FILE_PAGE_GROUP_EDIT, 0x04000535 },
	{ IDC_FILE_PAGE_BOX_EDIT_ONE, 0x04000536 },
	{ IDC_FILE_PAGE_BOX_EDIT_TWO, 0x04000537 },
	{ IDC_FILE_PAGE_BOX_EDIT_THREE, 0x04000538 },
	{ IDC_FILE_PAGE_BOX_EDIT_FOUR, 0x04000539 }
};


bool CFilePage::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000534);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	CheckButton(IDC_FILE_PAGE_BOX_EDIT_ONE,true);
	
	
	
 return COptionPage::OnInit();
}
