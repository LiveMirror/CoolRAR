// wcDialog.cpp

#include "StdAfx.h"
#include "BrowseDialog.h"
#include "wcDialog.h"
#include "../Common/CompressCall.h"
#include "Panel.h"
#include "resource.h"
using namespace NWindows;


extern HINSTANCE g_hInstance;

bool CwcDialog::OnInit()
{
	

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
	for (int i = 0; i < PressString.Size(); i++)
		_pressname.AddString(PressString[i]);
	_pressname.SetText(Pressname);
	::ShowWindow(GetItem(IDC_COMBO_PRESSNAME),SW_HIDE);
	return CModalDialog::OnInit();
}


bool CwcDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	
	switch(buttonID)
	{
	 case IDC_BUTTON_NEXT:

		 if(CheckRadioButton(IDC_WC_REALEASE) 
			 && stpeSign ==FirstStpe)                         //选解压缩文件并按下下一步按钮时

		 {
			NextRealeaseOne();
			break;
		 }	

		 if(CheckRadioButton(IDC_WC_REALEASE) 
			&& stpeSign ==RealeaseOneStpe
			&& !Value.IsEmpty()
			)                                                    //选解压缩文件并进行到第二步时

		 {	
			 UString name =Value;

	    	 _path.GetText(Value);
			 if( !Value.IsEmpty() )
			 {
				 UStringVector paths;
	    	     paths.Add(Value);
		         ::ExtractArchives(paths, Value+UString(WCHAR_PATH_SEPARATOR), true);
			     break;
			 }
			 ::MessageBox(HWND(*this),"请选择一个压缩文件再进行下一步","提示",MB_ICONWARNING);
			 break;
		 }
	    if(CheckRadioButton(IDC_WC_REALEASE) 
			 && stpeSign ==RealeaseOneStpe
			 && Value.IsEmpty()
			 )
			 
		 {
			 ::MessageBox(HWND(*this),"请选择一个压缩文件再进行下一步","提示",MB_ICONWARNING);
			 break;
		 }


		 if(CheckRadioButton(IDC_WC_NEWREALEASE)
			&& stpeSign ==FirstStpe)                            //选择创建新的压缩文件第一步
		 {
			 NewPressOne();
			 break;
		 }
		
		 if(CheckRadioButton(IDC_WC_NEWREALEASE)
			 && stpeSign ==PressThrStpe
			 && !Pressname.IsEmpty())							//创建新的压缩文件最后一步
		 {
			 NewPressThree();
			 break;
		 }
		 else if(CheckRadioButton(IDC_WC_NEWREALEASE)
			 && stpeSign ==PressThrStpe
			 && Pressname.IsEmpty())
		 {
			 ::MessageBox(HWND(*this),"压缩文件名不能为空","错误",20);
			 break;
		 }

		 if(CheckRadioButton(IDC_WC_ADDREALEASE)
			 && stpeSign == FirstStpe)
		 {
			 AddRealeaseOne();
			 break;
		 }
		 if(CheckRadioButton(IDC_WC_ADDREALEASE)
			 && stpeSign ==AddOneStpe )
		 {
			AddRealeaseTwo();
			break;
		 }
			 ::MessageBox(HWND(*this),"请选择你要进行的操作","提示",MB_ICONWARNING);
		break;

	 case IDC_BUTTON_HELP:
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
bool CwcDialog::CheckRadioButton(int buttonID)
{
	return CwcDialog::IsButtonCheckedBool(buttonID);

}
//解压缩文件第一步
void CwcDialog::NextRealeaseOne()
{
	::ShowWindow(GetItem(IDC_WC_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_WC_NEWREALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_WC_ADDREALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT3_STATIC_WC),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT4_STATIC_WC),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_COMBO_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_SHOWNORMAL);
	stpeSign =RealeaseOneStpe;
}
//从解压缩文件第一步返回
void CwcDialog::FrontRealeaseTwo()
{
	::ShowWindow(GetItem(IDC_WC_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_WC_NEWREALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_WC_ADDREALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT3_STATIC_WC),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT4_STATIC_WC),SW_HIDE);
	::ShowWindow(GetItem(IDC_COMBO_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_HIDE);
	stpeSign =FirstStpe;
}

extern UString CreateArchiveName(const UString &srcName, bool fromPrev, bool keepName);
//创建新的压缩文件第一步
void CwcDialog::NewPressOne()
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
	
		return;
}
//创建新的压缩文件第二步
void CwcDialog::NewPressTwo()
{
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_COMBO_PRESSNAME),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT5_STATIC_WC),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT1_STATIC_WC),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT2_STATIC_WC),SW_HIDE);
	::ShowWindow(GetItem(IDC_WC_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_WC_NEWREALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_WC_ADDREALEASE),SW_HIDE);
	stpeSign = PressThrStpe;
	
}

//创建新的压缩文件第三步
void CwcDialog::NewPressThree()
{
	Pressname =PressString.Back();
	int drive =3;
	UString drivename =fullPath.Left(drive);
	UStringVector names;
	names.Add(fullPath);
	
	CompressFiles(drivename, Pressname, L"", names, false, true, false);
}

//从创建新的压缩文件第二步返回
void CwcDialog::PressTwoFront()
{
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_HIDE);
	::ShowWindow(GetItem(IDC_COMBO_PRESSNAME),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT5_STATIC_WC),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT1_STATIC_WC),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT2_STATIC_WC),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_WC_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_WC_NEWREALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_WC_ADDREALEASE),SW_SHOWNORMAL);
	fullPath.Empty();
	stpeSign =FirstStpe;
}

void CwcDialog::AddRealeaseOne()
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
	::ShowWindow(GetItem(IDC_TEXT6_STATIC_WC),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT1_STATIC_WC),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT2_STATIC_WC),SW_HIDE);
	::ShowWindow(GetItem(IDC_WC_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_WC_NEWREALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_WC_ADDREALEASE),SW_HIDE);
	stpeSign =AddOneStpe;

}

void CwcDialog::AddRealeaseTwo()
{
	
	UStringVector names;
	names.Add(fullPath);
	
	CompressFiles(Value+L"\\", Value, L"zip", names, false, false, false);
}

void CwcDialog::AddRealeaseFront()
{
	stpeSign =FirstStpe;
	Value.Empty();
	Pressname.Empty();
	::ShowWindow(GetItem(IDC_BUTTON_SEARCH),SW_HIDE);
	::ShowWindow(GetItem(IDC_COMBO_REALEASE),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT6_STATIC_WC),SW_HIDE);
	::ShowWindow(GetItem(IDC_TEXT1_STATIC_WC),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_TEXT2_STATIC_WC),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_WC_REALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_WC_NEWREALEASE),SW_SHOWNORMAL);
	::ShowWindow(GetItem(IDC_WC_ADDREALEASE),SW_SHOWNORMAL);	
}
//选择压缩文件时浏览文件
void CwcDialog::SearchPressFile()
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
void CwcDialog::SearchFile()
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
void CwcDialog::ReflashVector(UStringVector &VString,UString string,NWindows::NControl::CComboBox &Object)
{
	VString.Add(string);
	for (int i = 0; i < VString.Size(); i++)
	{
		_path.AddString(VString[i]);
	}
	Object.SetCurSel(-1);
	Object.SetText(string);

}
