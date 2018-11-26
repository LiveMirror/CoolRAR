// FindDialog.cpp

#include "StdAfx.h"

#include "FindDialog.h"

#include "Windows/FileFind.h"

#include "Common/MyString.h"
#include "Windows/FileSystem.h"
#include "HelpUtils.h"
#include "LangUtils.h"

using namespace NWindows;
using namespace NFile;
using namespace NFind;

static LPCWSTR kFMHelpTopic = L"dialog/cha_kan.htm";

static CIDLangPair kIDLangPairs[] =
{
	{ IDC_FIND_DLG_GROUP_FILENAME                 , 0x04000390 },
	{ IDC_FIND_DLG_TEXT_FILENAME                  , 0x04000391 },
	{ IDC_FIND_DLG_TEXT_USTRING                   , 0x04000392 },
	{ IDC_FIND_DLG_BOX_NAME_ONE                   , 0x04000393 },
	{ IDC_FIND_DLG_BOX_NAME_TWO                   , 0x04000394 },
	{ IDC_FIND_DLG_BOX_NAME_THREE                 , 0x04000395 },
	{ IDC_FIND_DLG_GROUP_LOCATION                 , 0x04000396 },
	{ IDC_FIND_DLG_TEXT_LOCATION                  , 0x04000397 },
	{ IDC_FIND_DLG_TEXT_TYPE                      , 0x04000398 },
	{ IDC_FIND_DLG_BOX_LOCATION_ONE               , 0x04000399 },
	{ IDC_FIND_DLG_BOX_LOCATION_TWO               , 0x04000400 },
	{ IDC_FIND_DLG_BOX_LOCATION_THREE               , 0x04000401 },
	{ IDC_FIND_DLG_BUTTON_OK                      , 0x05000001 },
	{ IDC_FIND_DLG_BUTTON_CANCLE                  , 0x05000002 },
	{ IDC_FIND_DLG_BUTTON_HELP                    , 0x05000003 },
	{ IDC_FIND_DLG_BUTTON_SAVE                    , 0x04000402 },

};
bool CFindDialog::OnInit()
{
	LangSetWindowText(HWND(*this), 0x01000100);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	_FileName.Attach(GetItem(IDC_FIND_DLG_COMBO_FILENAME));
	_Ustring.Attach(GetItem(IDC_FIND_DLG_COMBO_USTRING));
	_Location.Attach(GetItem(IDC_FIND_DLG_COMBO_LOCATION));
	_FileType.Attach(GetItem(IDC_FIND_DLG_COMBO_TYPE));

	
	_Location.AddString(L"<当前文件夹>");
	UString str=L"<本地硬盘驱动器> ";

	UStringVector driveStrings;
	MyGetLogicalDriveStrings(driveStrings);

	for (int i = 0; i < driveStrings.Size(); i++)
	{
		UString s = driveStrings[i];
	  if (s.Length()!= 0 && s.Right(1) == L"\\")
	  {
		  s=s.Left(s.Length()-1);
		  _Location.AddString(s);

		if (NFile::NSystem::MyGetDriveType(s) == DRIVE_FIXED)str += s+L"  ";
	  }

	}

	_Location.AddString(str);

	_Location.SetCurSel(0);


  
  return CModalDialog::OnInit();
}




bool CFindDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{

	switch(buttonID)
	{
	case IDC_FIND_DLG_BUTTON_OK:
		OnButtonOk();
		return true;
	case IDC_FIND_DLG_BUTTON_CANCLE:
		OnButtonCancle();
		return true;
	case IDC_FIND_DLG_BUTTON_SAVE:
		OnButtonSave();
		return true;
	case IDC_FIND_DLG_BUTTON_HELP:
		OnButtonHelp();
		return true;
	default:
		return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
	}
	return true;
}

void CFindDialog::OnButtonOk()
{


	_FileName.GetText(*FileName);
	_Ustring.GetText(*UstringName);
	_Location.GetText(*Location);

  if (FileName->Length() ==0 && UstringName->Length() ==0)
  {
	  MessageBoxW(_window,LangString(0x07000016),LangString(0x07000002),MB_ICONWARNING);
	  return;
  }

  if( !UstringName->IsEmpty() )				//优先以字符串查找
  {
	  FileName->Empty();
	  _Ustring.GetText(*FileName);					
  }
  else
  {
	  if(  FileName->Find(L".") )
	  {
		MessageBoxW(_window,LangString(0x07000017),LangString(0x07000002),MB_ICONWARNING);
		return;
	  }

  }
  *IsSize=IsButtonCheckedBool(IDC_FIND_DLG_BOX_NAME_ONE);
  *Ischild=IsButtonCheckedBool(IDC_FIND_DLG_BOX_LOCATION_ONE);
  *IsFile=IsButtonCheckedBool(IDC_FIND_DLG_BOX_LOCATION_TWO);
  CModalDialog::OnOK();
}

void CFindDialog::OnButtonCancle()
{
	OnCancel();
}

void CFindDialog::OnButtonSave()
{

}

void CFindDialog::OnButtonHelp()
{
	ShowHelpWindow(NULL, kFMHelpTopic);
}


