// SdDialog.cpp

#include "StdAfx.h"

#include "SdDialog.h"
#include "SdDialogRes.h"
#include "BrowseDialog.h"
#include "HelpUtils.h"
#include "LangUtils.h"

using namespace NWindows;
static LPCWSTR kFMHelpTopic = L"dialog/wen_jian.htm";

static CIDLangPair kIDLangPairs[] =
{
	{ IDC_SD_TEXT_FILE                                     , 0x04000426 },
	{ IDC_SD_TEXT_ONE                                      , 0x04000427 },
	{ IDC_SD_BUTTON_SEARCH                                 , 0x04000428 },
	{ IDC_SD_TEXT_TWO                                      , 0x04000429 },
	{ IDC_SD_BUTTON_ONE                                    , 0x05000001 },
	{ IDC_SD_BUTTON_TWO                                    , 0x05000002 },
	{ IDC_SD_BUTTON_HELP                                   , 0x05000003 },

};
bool CSdDialog::OnInit()
{
  
	LangSetWindowText(HWND(*this), 0x04000430);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	_comBox.Attach(GetItem(IDC_SD_COMBO_FILE));

	_comBox.AddString("用户自定义");
	_comBox.SetCurSel(0);

	_filepathControl.Attach(GetItem(IDC_SDE_EDITTEXT_TWO));
	_pathControl.Attach(GetItem(IDC_SDE_EDITTEXT_ONE));

	_filepathControl.SetText(_filepath->GetBuffer(_filepath->Length()));
	
	_pathControl.SetText(_path->GetBuffer(_path->Length()));






  return CModalDialog::OnInit();
}

bool CSdDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
   
	case IDC_SD_BUTTON_SEARCH:
		{
			
			UString title =L"选择病毒扫描软件";
			UString s =L"";
				UString st=L"";
			s += L"*.exe";
			UString resPath;
			if (!MyBrowseForFile(HWND(*this), title, st, s, resPath))
				return false;
			_pathControl.SetText(resPath);
			return true;
		}
	case IDC_SD_BUTTON_ONE:
		{
			UString s;
			_pathControl.GetText(s);
			if (s.Length() == 0)
			{
				MessageBoxW(_window,LangString(0x07000028),LangString(0x07000029),NULL);
			}
			else
			{
				_pathControl.GetText(*_path);
				_filepathControl.GetText(*_filepath);
				OnOK();
				
			}
			
			return true;
		}
	case IDC_SD_BUTTON_TWO:
		{
			OnCancel();
			return true;
		}
	case IDC_SD_BUTTON_HELP:
		{
			ShowHelpWindow(NULL,kFMHelpTopic);
			return true;
		}

	}
	return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
}

