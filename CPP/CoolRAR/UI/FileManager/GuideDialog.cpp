// GuideDialog.cpp

#include "StdAfx.h"
#include "BrowseDialog.h"
#include "GuideDialog.h"
#include "../Common/CompressCall.h"
#include "Panel.h"
#include "resource.h"
#include "HttpUpdat.h"
#include "HelpUtils.h"
#include "LangUtils.h"
using namespace NWindows;


extern HttpUpdat httpworkdat;
extern HINSTANCE g_hInstance;
static LPCWSTR kFMHelpTopic = L"dialog/cha_kan.htm";
static CIDLangPair kIDLangPairs[] =
{
	{ IDC_TEXT1_STATIC_GUIDE                 , 0x04000378 },
	{ IDC_TEXT2_STATIC_GUIDE                 , 0x04000379 },
	{ IDC_TEXT3_STATIC_GUIDE                 , 0x04000380 },
	{ IDC_TEXT4_STATIC_GUIDE                 , 0x04000381 },
	{ IDC_TEXT5_STATIC_GUIDE                 , 0x04000382 },
	{ IDC_TEXT6_STATIC_GUIDE                 , 0x04000383 },
	{ IDC_GUIDE_REALEASE                     , 0x04000384 },
	{ IDC_GUIDE_NEWREALEASE                  , 0x04000385 },
	{ IDC_BUTTON_SEARCH                      , 0x04000386 },
	{ IDC_BUTTON_NEXT                        , 0x04000387 },
	{ IDC_BUTTON_FRONT                       , 0x04000388 },
	{ IDOK                                   , 0x05000002 },
	{ IDC_GUIDE_BUTTON_HELP                  , 0x05000003 }
	
};
bool CGuideDialog::OnInit()
{
	
	LangSetWindowText(HWND(*this), 0x04000389);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	_path.Attach(GetItem(IDC_COMBO_REALEASE));
	
	
	for (int i = 0; i < Strings.Size(); i++)
		_path.AddString(Strings[i]);
	_path.SetText(Value);
	::ShowWindow(GetItem(IDC_COMBO_REALEASE),SW_HIDE);

	stpeSign =FirstStpe;

	_pressname.Attach(GetItem(IDC_COMBO_PRESSNAME));
	Title.Empty();
	Title =L"向导";
	SetText(Title);
	for (int j = 0; j < PressString.Size(); j++)
		_pressname.AddString(PressString[j]);
	_pressname.SetText(Pressname);
	::ShowWindow(GetItem(IDC_COMBO_PRESSNAME),SW_HIDE);
	return CModalDialog::OnInit();
}


bool CGuideDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	
	switch(buttonID)
	{
	 case IDC_BUTTON_NEXT:

		 if(CheckRadioButton(IDC_GUIDE_REALEASE) 
			 && stpeSign ==FirstStpe)                         //选解压缩文件并按下下一步按钮时

		 {
			NextRealeaseOne();
			break;
		 }	

		 if(CheckRadioButton(IDC_GUIDE_REALEASE) 
			&& stpeSign ==RealeaseOneStpe
			)                                                    //选解压缩文件并进行到第二步时

		 {	
			 _path.GetText(Value);
			 if( !Value.IsEmpty() )
			 {
				 UStringVector paths;
				 paths.Add(Value);
				 while(Value.Find(L".") >0)
				 {
					 Value.DeleteBack();
				 }
				 Value += WCHAR_PATH_SEPARATOR;
				 ::ExtractArchives(paths, Value, true);
				 OnOK();

				 break;
			 }	
			 ::MessageBoxW(HWND(*this),LangString(0x07000022),LangString(0x07000012),MB_ICONWARNING);
			 break;
		 }
	    if(CheckRadioButton(IDC_GUIDE_REALEASE) 
			 && stpeSign ==RealeaseOneStpe
			 && Value.IsEmpty()
			 )
			 
		 {
			 ::MessageBoxW(HWND(*this),LangString(0x07000022),LangString(0x07000012),MB_ICONWARNING);
			 break;
		 }


		 if(CheckRadioButton(IDC_GUIDE_NEWREALEASE)
			&& stpeSign ==FirstStpe)                            //选择创建新的压缩文件第一步
		 {
			 NewPressOne();
			 break;
		 }
		
		 if(CheckRadioButton(IDC_GUIDE_NEWREALEASE)
			 && stpeSign ==PressThrStpe
			 && !Pressname.IsEmpty())							//创建新的压缩文件最后一步
		 {
			 NewPressThree();
			 break;
		 }
		 else if(CheckRadioButton(IDC_GUIDE_NEWREALEASE)
			 && stpeSign ==PressThrStpe
			 && Pressname.IsEmpty())
		 {
			 ::MessageBoxW(HWND(*this),LangString(0x07000023),LangString(0x07000019),MB_ICONWARNING);
			 break;
		 }

		 if(CheckRadioButton(IDC_GUIDE_ADDREALEASE)
			 && stpeSign == FirstStpe)
		 {
			 AddRealeaseOne();
			 break;
		 }
		 if(CheckRadioButton(IDC_GUIDE_ADDREALEASE)
			 && stpeSign ==AddOneStpe )
		 {
			AddRealeaseTwo();
			break;
		 }
			 ::MessageBoxW(HWND(*this),LangString(0x07000024),LangString(0x07000012),MB_ICONWARNING);
		break;

	 case IDC_GUIDE_BUTTON_HELP:
		 ShowHelpWindow(NULL,kFMHelpTopic);
		 break;

	 case IDC_BUTTON_FRONT:
		 if( stpeSign == RealeaseOneStpe )
		 {
			 FrontRealeaseTwo();
		 }
		 if( stpeSign == PressThrStpe)
		 {
			PressTwoFront();
		 }
		 if( stpeSign == AddOneStpe)
		 {
			AddRealeaseFront();
		 }
		 break;
	 case IDC_BUTTON_SEARCH:
		//浏览文件	
		 if( stpeSign == RealeaseOneStpe)//选择解压缩时
		 {
			SearchFile();
		 }
		 if( stpeSign == PressThrStpe)//选择创建压缩文件时
		 {
			 SearchPressFile();
		 }
		 if( stpeSign == AddOneStpe)
		 {
			SearchFile();
		 }
		
		 break;

	 default:
		 return CModalDialog::OnButtonClicked(buttonID, buttonHWND);

	}
	return false;
}
bool CGuideDialog::CheckRadioButton(int buttonID)
{
	return CGuideDialog::IsButtonCheckedBool(buttonID);

}
//解压缩文件第一步
void CGuideDialog::NextRealeaseOne()
{
	::ShowWindow(GetItem(IDC_GUIDE_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_GUIDE_NEWREALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_GUIDE_ADDREALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT3_STATIC_GUIDE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT4_STATIC_GUIDE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_COMBO_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_SHOWNORMAL);
	EnableItem(IDC_BUTTON_FRONT,true);
	stpeSign =RealeaseOneStpe;
}
//从解压缩文件第一步返回
void CGuideDialog::FrontRealeaseTwo()
{
	::ShowWindow(GetItem(IDC_GUIDE_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_GUIDE_NEWREALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_GUIDE_ADDREALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT3_STATIC_GUIDE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT4_STATIC_GUIDE),SW_HIDE);
	::ShowWindow(GetItem(IDC_COMBO_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_HIDE);
	EnableItem(IDC_BUTTON_FRONT,false);
	stpeSign =FirstStpe;
}

extern UString CreateArchiveName(const UString &srcName, bool fromPrev, bool keepName);
//创建新的压缩文件第一步
void CGuideDialog::NewPressOne()
{
	UString title =L"请选择要添加的文件";
	UString resultPath;
	UString currentPath=L"";
	if (MyBrowseForFolder(HWND(*this), title, currentPath, resultPath))
	{
		fullPath =resultPath;
	    Pressname = CreateArchiveName(resultPath, resultPath.IsEmpty(), false);
	    ReflashVector(PressString,Pressname,_pressname);
		NewPressTwo();	
	}


}
//创建新的压缩文件第二步
void CGuideDialog::NewPressTwo()
{
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_COMBO_PRESSNAME),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT5_STATIC_GUIDE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT1_STATIC_GUIDE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT2_STATIC_GUIDE),SW_HIDE);
	::ShowWindow(GetItem(IDC_GUIDE_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_GUIDE_NEWREALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_GUIDE_ADDREALEASE),SW_HIDE);
	EnableItem(IDC_BUTTON_FRONT,true);
	stpeSign = PressThrStpe;
}

//创建新的压缩文件第三步
void CGuideDialog::NewPressThree()
{
	Pressname =PressString.Back();
	
	
	UStringVector names;
	names.Add(fullPath);
	while(fullPath.Back() != WCHAR_PATH_SEPARATOR)
		fullPath.DeleteBack();
	CompressFiles(fullPath, Pressname, L"", names, false, true, false);
	OnOK();
}

//从创建新的压缩文件第二步返回
void CGuideDialog::PressTwoFront()
{
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_HIDE);
	::ShowWindow(GetItem(IDC_COMBO_PRESSNAME),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT5_STATIC_GUIDE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT1_STATIC_GUIDE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT2_STATIC_GUIDE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_GUIDE_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_GUIDE_NEWREALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_GUIDE_ADDREALEASE),SW_SHOWNORMAL);
	EnableItem(IDC_BUTTON_FRONT,false);
	fullPath.Empty();
	stpeSign =FirstStpe;
}

void CGuideDialog::AddRealeaseOne()
{
	UString title =L"请选择要添加的文件";
	UString resultPath;
	UString currentPath=L"";
	if (MyBrowseForFolder(HWND(*this), title, currentPath, resultPath))
	{
		fullPath =resultPath;
		Pressname = CreateArchiveName(resultPath, resultPath.IsEmpty(), false);		

	}
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_COMBO_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT6_STATIC_GUIDE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT1_STATIC_GUIDE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT2_STATIC_GUIDE),SW_HIDE);
	::ShowWindow(GetItem(IDC_GUIDE_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_GUIDE_NEWREALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_GUIDE_ADDREALEASE),SW_HIDE);
	stpeSign =AddOneStpe;

}

void CGuideDialog::AddRealeaseTwo()
{
	
	UStringVector names;
	names.Add(fullPath);
	
	CompressFiles(Value+L"\\", Value, L"zip", names, false, false, false);
}

void CGuideDialog::AddRealeaseFront()
{
	stpeSign =FirstStpe;
	Value.Empty();
	Pressname.Empty();
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_HIDE);
	::ShowWindow(GetItem(IDC_COMBO_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT6_STATIC_GUIDE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT1_STATIC_GUIDE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT2_STATIC_GUIDE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_GUIDE_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_GUIDE_NEWREALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_GUIDE_ADDREALEASE),SW_SHOWNORMAL);	
}
//选择压缩文件时浏览文件
void CGuideDialog::SearchPressFile()
{
	UString fileName=L"";
	UString title = L"查找文件";
	UString s = L"所有文件";
	UString resPath;
	if (MyBrowseForFile(HWND(*this),  title, fileName, s, resPath))
	{
		Pressname =resPath;
		ReflashVector(PressString,Pressname,_pressname);
	}

}
//选择解压缩文件时浏览文件
void CGuideDialog::SearchFile()
{
	UString fileName=L"";
	UString title = L"查找压缩文件";
	UString s = L"所有文件";
	UString resPath;
	if (MyBrowseForFile(HWND(*this),  title, fileName, s, resPath))
	{
		Value =resPath;
		Strings.Add(Value);
		for (int i = 0; i < Strings.Size(); i++)
		{
			_path.AddString(Strings[i]);
		}
		_path.SetCurSel(-1);
		_path.SetText(Value);

	}

}

//更新迭代器中的内容并将内容显示到下拉列表中
void CGuideDialog::ReflashVector(UStringVector &VString,UString string,NWindows::NControl::CComboBox &Object)
{
	VString.Add(string);
	for (int i = 0; i < VString.Size(); i++)
	{
		_path.AddString(VString[i]);
	}
	Object.SetCurSel(-1);
	Object.SetText(string);

}
