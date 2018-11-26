#include "StdAfx.h"
#include "BrowseDialog.h"
#include "PsWordDialog.h"
#include "HelpUtils.h"
#include "LangUtils.h"
#include "App.h"
#include "../Common/CompressCall.h"
using namespace NWindows;

static CIDLangPair kIDLangPairs[] =
{
	{ IDC_TEXT1_STATIC_PsWord                 , 0x04000373 },
	{ IDC_TEXT2_STATIC_PsWord                 , 0x04000374 },
	{ IDC_BUTTON_ENSURE                       , 0x05000001 },
	{ ID_BUTTON_CANCEL                        , 0x05000002 },
	{ ID_BUTTON_HELP                          , 0x05000003 },
	{ IDC_CHECK_PASSWORD_PsWord               , 0x04000376 },
	{ IDC_CHECK_FILENAME_PsWord               , 0x04000375 },
};
bool CPsWordDialog::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000377);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	_psWordin.Attach(GetItem(IDC_CIN_PsWord));
	_psWordinnormal.Attach(GetItem(IDC_CIN_PSWORD_NORMAL));
	_psWordjudeg.Attach(GetItem(IDC_EDIT_PsWord));
	::ShowWindow(GetItem(IDC_CIN_PSWORD_NORMAL),SW_HIDE);
	return CModalDialog::OnInit();
}
//œ‘ æ√‹¬Î°£
void CPsWordDialog::ChangePassWord()
{
	if(IsButtonCheckedBool(IDC_CHECK_PASSWORD_PsWord))
	{
		::ShowWindow(GetItem(IDC_EDIT_PsWord),SW_HIDE);
		::ShowWindow(GetItem(IDC_CIN_PsWord),SW_HIDE);
		::ShowWindow(GetItem(IDC_TEXT2_STATIC_PsWord),SW_HIDE);
		::ShowWindow(GetItem(IDC_CIN_PSWORD_NORMAL),SW_SHOW);
		_psWordin.GetText(_psWord);
		_psWordinnormal.SetText(_psWord);
	
	}
	else
	{
		::ShowWindow(GetItem(IDC_EDIT_PsWord),SW_SHOW);
		::ShowWindow(GetItem(IDC_CIN_PSWORD_NORMAL),SW_HIDE);
		::ShowWindow(GetItem(IDC_CIN_PsWord),SW_SHOW);
		::ShowWindow(GetItem(IDC_TEXT2_STATIC_PsWord),SW_SHOW);
		_psWordinnormal.GetText(_psWord);
		_psWordin.SetText(_psWord);
	}
}
bool CPsWordDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_CHECK_PASSWORD_PsWord:
		ChangePassWord();//œ‘ æ√‹¬Î°£
		return true;
	case IDC_BUTTON_ENSURE:
		if( EnterPsWord() )
		{
			OnOK();
		}
		return true;
	case ID_BUTTON_CANCEL:
		OnOK();
		return true;
	case ID_BUTTON_HELP:
		static LPCWSTR kFMHelpTopic = L"dialog/wen_jian.htm";
		ShowHelpWindow(NULL,kFMHelpTopic);
		return true;
			
	}

	return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
}

extern void SetupDefaultPassword(UString newpassword);
bool CPsWordDialog::EnterPsWord()
{
	UString commderline;
	if(IsButtonCheckedBool(IDC_CHECK_PASSWORD_PsWord))
	{
		
		_psWord.Empty();
		_psWordinnormal.GetText(_psWord);
		if(_psWord.IsEmpty())
		{
			commderline.Empty();
			SetupDefaultPassword(commderline);
			::MessageBoxW(HWND(*this),LangString(0x07000026),LangString(0x07000012),MB_ICONWARNING);
				return true;
		}
		
		commderline =L" -p";
		commderline +=_psWord;
		SetupDefaultPassword(commderline);
		return true;
	}
	else
	{
		UString judgeString;
		_psWordjudeg.GetText(judgeString);
		_psWord.Empty();
		_psWordin.GetText(_psWord);
		if(_psWord == judgeString
			&&!_psWord.IsEmpty()
		  )
		{
			
			commderline =L" -p";
			commderline +=_psWord;
			SetupDefaultPassword(commderline);
			return true;
		}
		if(_psWord.IsEmpty()
			&&judgeString.IsEmpty())
		{
			commderline.Empty();
			SetupDefaultPassword(commderline);
			::MessageBoxW(HWND(*this),LangString(0x07000026),LangString(0x07000012),MB_ICONWARNING);
				return true;
		}
		::MessageBoxW(HWND(*this),LangString(0x07000027),LangString(0x07000019),MB_ICONERROR);
		return false;
	}
}