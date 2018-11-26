// FindDialog.cpp

#include "StdAfx.h"
#include "FindProcessDialog.h"
#include "Windows/FileIO.h"
#include "Windows/FileFind.h"
#include "LangUtils.h"
#include "App.h"
#include "Panel.h"
#include "Windows/Process.h"
#include "Common/StringConvert.h"
#include "../Common/CompressCall.h"
#include "HelpUtils.h"

using namespace NWindows;
using namespace NFile;
using namespace NFind;
using namespace NIO;
using namespace NSynchronization;
using namespace NDirectory;

#define UM_SHOW_FILEINFO WM_USER+1

static LPCWSTR kFMHelpTopic = L"dialog/start.htm";
static CIDLangPair kIDLangPairs[] =
{
	{ IDC_FINDPROCESS_BUTTON_BACK                   , 0x04000403 },
	{ IDC_FINDPROCESS_BUTTON_PAUSE                  , 0x04000404 },
	{ IDC_FINDPROCESS_BUTTON_CLOSE                  , 0x04000405 },
	{ IDC_FINDPROCESS_BUTTON_HELP                   , 0x05000003 },
	{ IDC_FINDPROCESS_BUTTON_EXTRACT                , 0x04000406 },
	{ IDC_FINDPROCESS_BUTTON_CHECK                  , 0x04000407 },
	{ IDC_FINDPROCESS_BUTTON_SEARCH                 , 0x04000408 },
	{ IDC_FINDPROCESS_TEXT_PATH                     , 0x04000409 },


};

extern CApp g_App;
extern bool g_IsNT;
static HRESULT StartApplication(const UString &dir, const UString &path, HWND window, CProcess &process)
{
	UINT32 result;
#ifndef _UNICODE
	if (g_IsNT)
	{
		SHELLEXECUTEINFOW execInfo;
		execInfo.cbSize = sizeof(execInfo);
		execInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
		execInfo.hwnd = NULL;
		execInfo.lpVerb = NULL;
		execInfo.lpFile = path;
		execInfo.lpParameters = NULL;
		execInfo.lpDirectory = dir.IsEmpty() ? NULL : (LPCWSTR)dir;
		execInfo.nShow = SW_SHOWNORMAL;
		execInfo.hProcess = 0;
		ShellExecuteExWP shellExecuteExW = (ShellExecuteExWP)
			::GetProcAddress(::GetModuleHandleW(L"shell32.dll"), "ShellExecuteExW");
		if (shellExecuteExW == 0)
			return 0;
		shellExecuteExW(&execInfo);
		result = (UINT32)(UINT_PTR)execInfo.hInstApp;
		process.Attach(execInfo.hProcess);
	}
	else
#endif
	{
		SHELLEXECUTEINFO execInfo;
		execInfo.cbSize = sizeof(execInfo);
		execInfo.fMask = SEE_MASK_NOCLOSEPROCESS
#ifndef UNDER_CE
			| SEE_MASK_FLAG_DDEWAIT
#endif
			;
		execInfo.hwnd = NULL;
		execInfo.lpVerb = NULL;
		const CSysString sysPath = GetSystemString(path);
		const CSysString sysDir = GetSystemString(dir);
		execInfo.lpFile = sysPath;
		execInfo.lpParameters = NULL;
		execInfo.lpDirectory =
#ifdef UNDER_CE
			NULL
#else
			sysDir.IsEmpty() ? NULL : (LPCTSTR)sysDir
#endif
			;
		execInfo.nShow = SW_SHOWNORMAL;
		execInfo.hProcess = 0;
		::ShellExecuteEx(&execInfo);
		result = (UINT32)(UINT_PTR)execInfo.hInstApp;
		process.Attach(execInfo.hProcess);
	}

	return S_OK;
}

bool CFindProcessDialog::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000410);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	SetTimer(UM_SHOW_FILEINFO,1);
	ListviewOnInit();
  return CModalDialog::OnInit();
}

void CFindProcessDialog::ListviewOnInit()
{
	_filelistview.Attach(GetItem(IDC_FINDPROCESS_LIST));
	LV_COLUMNW column;
	column.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.cx = 150;
	column.pszText = L"文件名";
	_filelistview.InsertColumn(1, &column);
	column.pszText = L"位置";
	_filelistview.InsertColumn(2, &column);
	column.pszText = L"上下文";
	_filelistview.InsertColumn(3, &column);

}

void CFindProcessDialog::FileOk(UString FileP)
{
	FileNameStr filenamestr;
	CFileInfoW fileinfo;
	if (FileP.Right(1) != L"\\")
	{
		FileP+=L"\\";
	}
	UString strPath= FileP+L"*.*";
	CFindFile findfile;
	bool  bWork = findfile.FindFirst(strPath,fileinfo);
	
	while(bWork)
	{
		UString name =fileinfo.Name;
		
		SetItemText(IDC_FINDPROCESS_TEXT_TITLE,FileP + name);
		if (!IsSize)
		{
			name.MakeLower();
		}
		int i =name.Find(FileName);
		if (i!= -1)
		{
			filenamestr.FileName=fileinfo.Name;
			filenamestr.FilePath=FileP;
			filenamestrVect.Add(filenamestr);
			InsertFileInfo(fileinfo.Name,FileP,filenamestrVect.Size());
		}
		if ( fileinfo.Name != L".." && fileinfo.Name != L".")
		{
			if(fileinfo.IsDir() &&Ischild)
			{
				FileOk(FileP+fileinfo.Name);
			}
		}
 		
		bWork = findfile.FindNext(fileinfo);
		
	}
	
	SetItemText(IDC_FINDPROCESS_TEXT_TITLE,"");
	SetItemText(IDC_FINDPROCESS_TEXT_PATH,L"处理完毕");
}

bool CFindProcessDialog::OnTimer(WPARAM , LPARAM )
{
	KillTimer(UM_SHOW_FILEINFO);
	if (!IsSize)
	{
		FileName.MakeLower();
	}
	if (FileName.Length() != 0)
	{
		if (FileName.Length()!= 0 && FileName.Right(1) == L"*")
		{
			FileName = FileName.Left(FileName.Length()-2);
		}
		FileOk(Location);
	}
	else if(UstringName.Length() != 0)
	{
	}

	for (int i=0;i<filenamestrVect.Size();i++)
	{
		FileNameStr filenamestr=filenamestrVect[i];
	}
	return true;
}
bool CFindProcessDialog::OnCommand(int code, int itemID, LPARAM lParam)
{
	if (code == WM_TIMER)
	{
		OnTimer((WPARAM)itemID,lParam);
		
		return true;
	}
	
	return CModalDialog::OnCommand(code,itemID,lParam);
}

void CFindProcessDialog::InsertFileInfo(UString filename,UString filepath,int totalnum)
{	
		int i =totalnum -1;   //从0项开始插入，totalnum为文件信息的数量
		_filelistview.InsertItem(i,filename);
		_filelistview.SetSubItem(i,1,filepath);
}



bool CFindProcessDialog::OpenFouceItem()
{
	int index=_filelistview.GetFocusedItem();
	if(index < 0)
	{
		MessageBoxW(HWND(*this),LangString(0x07000018),LangString(0x07000019),MB_ICONERROR);
		return true;
	}
	UString name =filenamestrVect.operator [](index).FileName;
	UString path =filenamestrVect.operator [](index).FilePath;
	UString fullpath =path+name;

	if (fullpath.Right(1) != L"\\")
	{
		fullpath+=L"\\";
	}

	fullpath+=L"*.*";
	CFindFile findfile;
	CFileInfoW fileinfo;
	findfile.FindFirst(fullpath,fileinfo);

	if(!name.IsEmpty())
	{
		if(fileinfo.IsDir())		//如果选中的文件为文件夹
		{
			g_App.OpenFileByPath(path + name);
			return true;
		}	
		bool encrypted;
		encrypted =true;
		if(g_App.FileTypeJudge(path + name))//如果选中的文件为压缩文件
		{
			g_App.OpenItemAsArchiveByPath(name,path,encrypted);
			g_App.GetFocusedPanel().RefreshListCtrl();
			return true;
		}

		CProcess process;
		if(StartApplication(path ,path + name,HWND(this),process) ==S_OK)//如果为可执行的外部文件
			return true;

	}

	return false;
	
}
void CFindProcessDialog::DoExtract()
{
	int index=_filelistview.GetFocusedItem();
	if(index < 0)
	{
		MessageBoxW(HWND(*this),LangString(0x07000018),LangString(0x07000019),MB_ICONERROR);
		return;
	}
	UString name =filenamestrVect.operator [](index).FileName;
	UString path =filenamestrVect.operator [](index).FilePath;
	UStringVector paths;
	paths.Add(path+ name);
	if( !g_App.FileTypeJudge(path+ name) )
	{
		::MessageBoxW(HWND(*this),LangString(0x07000020),LangString(0x07000019),MB_ICONERROR);
		return;
	}
	while(name.Find(L".") >0)
	{
		name.DeleteBack();
	}
	::ExtractArchives(paths, path + name + WCHAR_PATH_SEPARATOR, true);
}

void CFindProcessDialog::DoOpenParentFloder()
{
	int index=_filelistview.GetFocusedItem();
	if(index < 0)
	{
		MessageBoxW(HWND(*this),LangString(0x07000018),LangString(0x07000019),MB_ICONERROR);
		return;
	}
	UString name =filenamestrVect.operator [](index).FileName;
	UString path =filenamestrVect.operator [](index).FilePath;
	g_App.OpenFileByPath(path);
	g_App.GoToSearchFile(name);

	
}
bool CFindProcessDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_FINDPROCESS_BUTTON_CHECK:	
		if(!OpenFouceItem())
			::MessageBoxW(HWND(*this),LangString(0x07000019),LangString(0x07000021),15);
		break;

	case IDC_FINDPROCESS_BUTTON_CLOSE:
		OnOK();
		break;
	case IDC_FINDPROCESS_BUTTON_EXTRACT:
		DoExtract();
		break;
	case IDC_FINDPROCESS_BUTTON_SEARCH:
		DoOpenParentFloder();
		break;
	case IDC_FINDPROCESS_BUTTON_HELP:
		ShowHelpWindow(NULL,kFMHelpTopic);
		break;
		
	default:
		return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
	}
	return false;
}

