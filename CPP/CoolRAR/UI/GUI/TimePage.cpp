#include "StdAfx.h"
#include "TimePage.h"
#include "TimePageRes.h"

#include "../FileManager/LangUtils.h"

using namespace NWindows;


static CIDLangPair kIDLangPairs[] =
{
	{ IDD_PAGE_TIME, 0x04000540 },
	{ IDC_TIME_PAGE_GROUP_EDIT1, 0x04000541 },
	{ IDC_TIME_PAGE_BOX_EDIT_ONE, 0x04000542 },
	{ IDC_TIME_PAGE_BOX_EDIT_TWO, 0x04000543 },
	{ IDC_TIME_PAGE_BOX_EDIT_THREE, 0x04000544 },
	{ IDC_TIME_PAGE_GROUP_EDIT2, 0x04000545 },
	{ IDC_TEXT1_STATIC_TIME, 0x04000546},
	{ IDC_TIME_PAGE_GROUP_EDIT3, 0x04000547 },
	{ IDC_TEXT2_STATIC_TIME, 0x04000548}
};

bool CTimePage::OnInit()
{

	LangSetWindowText(HWND(*this), 0x04000540);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));

	CheckButton(IDC_TIME_PAGE_BOX_EDIT_ONE,true);
    return COptionPage::OnInit(); 
}

