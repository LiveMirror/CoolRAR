#include "StdAfx.h"
#include "FavoritesDialog.h"
#include "resource.h"
#include "App.h"
#include "Panel.h"
#include "HelpUtils.h"
#include "LangUtils.h"


using namespace NWindows;
static LPCWSTR kFMHelpTopic = L"dialog/gong_ju.htm";
static CIDLangPair kIDLangPairs[] =
{
	{ IDC_FAVORITE_ADD_TEXT                    , 0x04000411 },
	{ IDC_FAVORITE_PRESSFILE_TEXT              , 0x04000412 },
	{ IDC_FAVORITE_SAY_TEXT                    , 0x04000413 },
	{ IDC_FAVORITE_BUTTON_OK                   , 0x05000001 },
	{ IDC_FAVORITE_BUTTON_CHANEL               , 0x05000002 },
	{ IDC_FAVORITE_BUTTON_HELP                 , 0x05000003 },
	
};
extern CApp g_App;
bool CFavoritesDialog::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000414);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	_floder.Attach( GetItem(IDC_FAVORITE_ADD_EDIT));
	_pressfile.Attach( GetItem(IDC_FAVORITE_PRESSFILE_EDIT));
	_sign.Attach( GetItem(IDC_FAVORITE_SAY_EDIT));

	if(g_App.FileTypeJudge(_folderPrefix))        //如果是压缩文件则激活压缩文件子文件相关控件
	{
		EnableItem(IDC_FAVORITE_PRESSFILE_TEXT,true);
		EnableItem(IDC_FAVORITE_PRESSFILE_EDIT,true);
	}
	SetItemText(IDC_FAVORITE_ADD_EDIT,_folderPrefix);
	SetItemText(IDC_FAVORITE_SAY_EDIT,sign);
	return CModalDialog::OnInit();
}

void CFavoritesDialog::GetFloderPath(const UString _currentFolderPrefix)
{
	_folderPrefix =_currentFolderPrefix;
}

void CFavoritesDialog::GetSign(const UString getsign)
{
	sign = getsign;
}
void CFavoritesDialog::SetFavoritesPath()
{
	_floder.GetText(_folderPrefix);
	_pressfile.GetText(_pressfloder);
	if (!_pressfloder.IsEmpty())
	{
		_folderPrefix +=_pressfloder;
		_folderPrefix +=WCHAR_PATH_SEPARATOR;
	}
	g_App.SetBookmark(_folderPrefix);

	sign.Empty();
	_sign.GetText(sign);

	g_App.SetFavoritesSign(_folderPrefix,sign);
}

bool CFavoritesDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_FAVORITE_BUTTON_OK:
		SetFavoritesPath();
		OnOK();
		break;
	case IDC_FAVORITE_BUTTON_CHANEL:
		OnCancel();
		break;
	case IDC_FAVORITE_BUTTON_HELP:
		ShowHelpWindow(NULL,kFMHelpTopic);
		break;
	default:
		return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
	}
	return false;
}

