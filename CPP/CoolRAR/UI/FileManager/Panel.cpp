// Panel.cpp

#include "StdAfx.h"

#include <Windowsx.h>
#include <tlhelp32.h> 
#include "HttpUpdat.h"
#include "Common/Defs.h"
#include "Common/IntToString.h"
#include "Common/StringConvert.h"
#include "Common/DynamicBuffer.h"

#include "Windows/Error.h"
#include "Windows/PropVariant.h"
#include "Windows/Thread.h"

#include "../../PropID.h"

#include "resource.h"
#include "../GUI/ExtractRes.h"

#include "../Common/ArchiveName.h"
#include "../Common/CompressCall.h"

#include "../Agent/IFolderArchive.h"

#include "App.h"
#include "ExtractCallback.h"
#include "FSFolder.h"
#include "FormatUtils.h"
#include "Panel.h"
#include "RootFolder.h"
#include "ViewSettings.h"

#include "../Common/ZipRegistry.h"
#include "../Explorer/ContextMenuFlags.h"
#include "../Explorer/RegistryContextMenu.h"

#include "Windows/FileFind.h"


#include "Windows/FileIO.h"

#include "FilePlugins.h"
#include "PluginLoader.h"
#include "../Agent/Agent.h"
#include "ProgramLocation.h"
#include "PsWordDialog.h"
#include "BrowseDialog.h"
#include "OptionsDlgFM.h"
#include "FMAdvancedPage.h"
#include "FMConventionalPage.h"
#include "ThemeDialog.h"




using namespace NWindows;
using namespace NControl;
using namespace NFile;
using namespace NFind;
using namespace NIO;
using namespace NContextMenuFlags;


#ifndef _UNICODE
extern bool g_IsNT;
#endif
extern HttpUpdat httpworkdat;
static const UINT_PTR kTimerID = 1;
static const UINT kTimerElapse = 1000;


static DWORD kStyles[4] = { LVS_ICON, LVS_SMALLICON, LVS_LIST, LVS_REPORT };
extern HINSTANCE g_hInstance;
extern DWORD g_ComCtl32Version;

void CPanel::Release()
{
  // It's for unloading COM dll's: don't change it.
  CloseOpenFolders();
  _sevenZipContextMenu.Release();
  _systemContextMenu.Release();
}


CPanel::~CPanel()
{
	
	httpworkdat.DllGetUpdat();
	httpworkdat.DestroyDll();

  CloseOpenFolders();
}

HWND CPanel::GetParent()
{
  HWND h = CWindow2::GetParent();
  return (h == 0) ? _mainWindow : h;
}

static LPCWSTR kClassName = L"7-Zip::Panel";


HRESULT CPanel::Create(HWND mainWindow, HWND parentWindow, UINT id,
    const UString &currentFolderPrefix, CPanelCallback *panelCallback, CAppState *appState,
    bool &archiveIsOpened, bool &encrypted)
{
  _mainWindow = mainWindow;
  _processTimer = true;
  _processNotify = true;

  _panelCallback = panelCallback;
  _appState = appState;
  _baseID = id;
  _comboBoxID = _baseID + 3;
  _statusBarID = _comboBoxID + 1;
  _TreeViewID =_baseID+5;

  UString cfp = currentFolderPrefix;

  if (!currentFolderPrefix.IsEmpty())
    if (currentFolderPrefix[0] == L'.')
      if (!NFile::NDirectory::MyGetFullPathName(currentFolderPrefix, cfp))
        cfp = currentFolderPrefix;
  if(BindToPath(cfp, archiveIsOpened, encrypted) != S_OK)
	BindToPath(L"桌面",archiveIsOpened,encrypted);

  if (!CreateEx(0, kClassName, 0, WS_CHILD | WS_VISIBLE,
      0, 0, _xSize, 260,
      parentWindow, (HMENU)(UINT_PTR)id, g_hInstance))
    return E_FAIL;
  return S_OK;
}

void CPanel::OnInvertTracker(RECT rect)
{

	HDC           pDC ;
	HBRUSH pBrush ;

	pDC = GetDC (_window) ;
	pBrush = CreateSolidBrush (
		RGB (156, 156,  156)) ;

	HBRUSH hOldBrush = NULL;
	if (pBrush != NULL)
		hOldBrush = (HBRUSH)SelectObject(pDC, pBrush);
	::PatBlt(pDC,rect.left, rect.top, 3, rect.bottom, PATINVERT);
	if (hOldBrush != NULL)
		SelectObject(pDC, hOldBrush);
	DWORD a = ::GetLastError();
	ReleaseDC (_window, pDC) ;
	DeleteObject (pBrush) ;

}

LRESULT CPanel::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_MOUSEMOVE:
	  {
 		 ::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
 		  if(treeMove)
 		  {
			  
			  short x = (short)GET_X_LPARAM(lParam);
			  short y = (short)GET_Y_LPARAM(lParam);
 
 			  RECT rect;
			  rect.left = x - 2;
			  rect.right =  rect.left + 5;
				rect.top = _treeviewTop;//树视图顶部
			  rect.bottom = _treeviewBottom;//树视图底部
			  if (oldRect.left == 0 && oldRect.right == 0)
 			  {				
		 
					OnInvertTracker(rect);
				  oldRect=rect;
			  }
 			  else
 			  {
				  OnInvertTracker(oldRect);
				  OnInvertTracker(rect);
				  	oldRect=rect;
			  } 
				
		  }
		  
		  return 0;
	  }
  case WM_LBUTTONDOWN:
	  {
		  treeMove = true;//激活树视图移动函数
		  short x = (short)GET_X_LPARAM(lParam);
		  short y = (short)GET_Y_LPARAM(lParam);
		  oldPison.x = x;
		  oldPison.y = y;//存入当前树视图坐标


		  RECT rect;
		  rect.left = x - 2;
		  rect.right =  rect.left + 5;
		  rect.top = _treeviewTop;//树视图顶部
		  rect.bottom = _treeviewBottom;//树视图底部

		  if (oldRect.left == 0 && oldRect.right == 0)
		  {				
			  OnInvertTracker(rect);
			  oldRect=rect;
		  }

		  ::SetCapture(_window);

		  return 0;
	  }
  case WM_LBUTTONUP:
	  {
	  short x = (short)GET_X_LPARAM(lParam);
	    OnInvertTracker(oldRect);
			oldRect.bottom  = 0;
		  			oldRect.left = 0;
		  			oldRect.right = 0;
		  			oldRect.top = 0;
		  			if(x < 6)
						x = 6;
					SaveTreeMoveSize(x);
		  			ChangTLSzie(x-oldPison.x);//传入坐标改变树视图位置
		  			treeMove = false;
		  			::ReleaseCapture();
		   			return 0; 
		  			
		  		
		  		
		  	
	}
    case kShiftSelectMessage:
      OnShiftSelectMessage();
      return 0;
    case kReLoadMessage:
      RefreshListCtrl(_selectedState);
      return 0;
    case kSetFocusToListView:
      _listView.SetFocus();
      return 0;
    case kOpenItemChanged:
      return OnOpenItemChanged(lParam);
    case kRefreshStatusBar:
      OnRefreshStatusBar();
      return 0;
    case kRefreshHeaderComboBox:
      LoadFullPathAndShow();
      return 0;
    case WM_TIMER:
      OnTimer();
      return 0;
    case WM_CONTEXTMENU:
      if (OnContextMenu(HANDLE(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
        return 0;
	   break;

  }
  return CWindow2::OnMessage(message, wParam, lParam);
}
static LRESULT APIENTRY TreeViewSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWindow tempDialog(hwnd);
	CMyTreeView *w = (CMyTreeView *)(tempDialog.GetUserDataLongPtr());
	if (w == NULL)
		return 0;
	return w->OnMessage(message, wParam, lParam);
}
static LRESULT APIENTRY	StatusBarSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWindow tempDialog(hwnd);
	CMyStatusBar *w = (CMyStatusBar *)(tempDialog.GetUserDataLongPtr());
	if (w == NULL)
		return 0;

	return w->OnMessage(message, wParam, lParam);
}

static LRESULT APIENTRY ListViewSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CWindow tempDialog(hwnd);
  CMyListView *w = (CMyListView *)(tempDialog.GetUserDataLongPtr());

if (w == NULL)
    return 0;



  
  return w->OnMessage(message, wParam, lParam);
}


LRESULT CMyTreeView::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{

	
#ifndef _UNICODE
	if (g_IsNT)
		return CallWindowProcW(_origWindowProc, *this, message, wParam, lParam);
	else
#endif
		return CallWindowProc(_origWindowProc, *this, message, wParam, lParam);
}

LRESULT CMyStatusBar::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{

	if (WM_LBUTTONDOWN == message)
	{
		short x = (short)GET_X_LPARAM(lParam);
		if (x < 19)
		{
			_panel->CreatMenuByStatusBar(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		else if( x < 45)
		{
			CPsWordDialog p_Word;
			p_Word.Create(g_HWND);
		}
	}
	if (kRefreshStatusBar == message)
	{
		_panel->RefreshStatusBar();
	}
#ifndef _UNICODE
	if (g_IsNT)
		return CallWindowProcW(_origWindowProc, *this, message, wParam, lParam);
	else
#endif
		return CallWindowProc(_origWindowProc, *this, message, wParam, lParam);

}


LRESULT CMyListView::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{

	
	if (message == WM_CHAR)
  {
    UINT scanCode = (UINT)((lParam >> 16) & 0xFF);
    bool extended = ((lParam & 0x1000000) != 0);
    UINT virtualKey = MapVirtualKey(scanCode, 1);
    if (virtualKey == VK_MULTIPLY || virtualKey == VK_ADD ||
        virtualKey == VK_SUBTRACT)
      return 0;
    if ((wParam == '/' && extended)
        || wParam == '\\' || wParam == '/')
    {
      _panel->OpenDrivesFolder();
      return 0;
    }
  }
  else if (message == WM_SYSCHAR)
  {
    // For Alt+Enter Beep disabling
    UINT scanCode = (UINT)(lParam >> 16) & 0xFF;
    UINT virtualKey = MapVirtualKey(scanCode, 1);
    if (virtualKey == VK_RETURN || virtualKey == VK_MULTIPLY ||
        virtualKey == VK_ADD || virtualKey == VK_SUBTRACT)
      return 0;
  }

  else if (message == WM_KEYDOWN)
  {
    bool alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
    bool ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
    bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
    switch(wParam)
    {
      case VK_NEXT:
      {
        if (ctrl && !alt && !shift)
        {
          _panel->OpenFocusedItemAsInternal();
          return 0;
        }
        break;
      }
      case VK_PRIOR:
      if (ctrl && !alt && !shift)
      {
        _panel->OpenParentFolder();
        return 0;
      }
    }
  }
  #ifdef UNDER_CE
  else if (message == WM_KEYUP)
  {
    if (wParam == VK_F2) // it's VK_TSOFT2
    {
      // Activate Menu
      ::PostMessage(g_HWND, WM_SYSCOMMAND, SC_KEYMENU, 0);
      return 0;
    }
  }
  #endif
  else if (message == WM_SETFOCUS)
  {
    _panel->_lastFocusedIsList = true;
    _panel->_panelCallback->PanelWasFocused();
  }
  #ifndef _UNICODE
  if (g_IsNT)
    return CallWindowProcW(_origWindowProc, *this, message, wParam, lParam);
  else
  #endif
    return CallWindowProc(_origWindowProc, *this, message, wParam, lParam);
}


static LRESULT APIENTRY ComboBoxEditSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CWindow tempDialog(hwnd);
  CMyComboBoxEdit *w = (CMyComboBoxEdit *)(tempDialog.GetUserDataLongPtr());
  if (w == NULL)
    return 0;
  return w->OnMessage(message, wParam, lParam);
}

LRESULT CMyComboBoxEdit::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  // See MSDN / Subclassing a Combo Box / Creating a Combo-box Toolbar
  switch (message)
  {
    case WM_SYSKEYDOWN:
      switch (wParam)
      {
        case VK_F1:
        case VK_F2:
        {
          // check ALT
          if ((lParam & (1<<29)) == 0)
            break;
          bool alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
          bool ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
          bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
          if (alt && !ctrl && !shift)
          {
            _panel->_panelCallback->SetFocusToPath(wParam == VK_F1 ? 0 : 1);
            return 0;
          }
          break;
        }
      }
      break;
    case WM_KEYDOWN:
      switch (wParam)
      {
        case VK_TAB:
          _panel->SetFocusToList();
          return 0;
        case VK_F9:
        {
          bool alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
          bool ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
          bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
          if (!alt && !ctrl && !shift)
          {
            g_App.SwitchOnOffOnePanel();;
            return 0;
          }
          break;
        }
      }
      break;
    case WM_CHAR:
      switch (wParam)
      {
        case VK_TAB:
        case VK_ESCAPE:
          return 0;
      }
  }
  #ifndef _UNICODE
  if (g_IsNT)
    return CallWindowProcW(_origWindowProc, *this, message, wParam, lParam);
  else
  #endif
    return CallWindowProc(_origWindowProc, *this, message, wParam, lParam);
}

extern UString RootFolder_GetName_Computer(int &iconIndex);
extern UString RootFolder_GetName_Network(int &iconIndex);
extern UString RootFolder_GetName_Documents(int &iconIndex);
extern UString RootFolder_GetName_Desktop(int &iconIndex);
extern UString GetMyDesktopPath();
extern UString GetMyDocumentsPath();


void CPanel::InitTree()
{
	 treePathVector.Clear();

	 DesktopPathVector.Clear();


	int iconIndex;
	int childNum;
	UString ItemName;
	UString Filepath;
	UStringVector childFileVector;

	TreePathStruct treePathStruct;
	//我的桌面
	ItemName = RootFolder_GetName_Desktop(iconIndex);
	Filepath = GetMyDesktopPath();
	CFindFile Findfile;
	childFileVector =Findfile.FindDirFile(Filepath);
	childNum=childFileVector.Size();
	if(childNum == 0)childNum =1;  //强制我的桌面存在子节点
	HTREEITEM DesktopRoot=_treeView.InsertItem(ItemName,iconIndex,childNum);
	treePathStruct.FilePath=Filepath;
	treePathStruct.Root=DesktopRoot;
	treePathVector.Add(treePathStruct);

	//我的文档
	ItemName = RootFolder_GetName_Documents(iconIndex);
	childFileVector =Findfile.FindDirFile(GetMyDocumentsPath());
	childNum=childFileVector.Size();
	HTREEITEM DocumentsRoot=_treeView.InsertItem(DesktopRoot,ItemName,iconIndex,childNum);
	treePathStruct.FilePath=GetMyDocumentsPath();
	treePathStruct.Root=DocumentsRoot;

	treePathVector.Add(treePathStruct);

	//我的电脑
	ItemName = RootFolder_GetName_Computer(iconIndex);
	UStringVector driveStrings;
	MyGetLogicalDriveStrings(driveStrings);
	childNum = driveStrings.Size();
	HTREEITEM ComputerRoot=_treeView.InsertItem(DesktopRoot,ItemName,iconIndex,childNum);
	treePathStruct.FilePath=L"我的电脑";
	treePathStruct.Root=ComputerRoot;

	treePathVector.Add(treePathStruct);

	for (int i = 0; i < driveStrings.Size(); i++)
	{
		UString s = driveStrings[i];
		UString driveName = s;
		bool needChild = GetDriveInfo(driveName);//我的电脑
		AddDriveTree(driveName,s,needChild,ComputerRoot);

	}
	changTreeSize = true;
	_treeView.Expand(ComputerRoot);//展开我的电脑

	AddTree(GetMyDesktopPath(),DesktopRoot,false,false);//我的桌面文件夹

	changTreeSize = true;
	_treeView.Expand(DesktopRoot);//展开我的桌面
	if (_TreeViewMode == 0)
	{
		_treeView.Show(SW_SHOW);
	}
	else
	{
		_treeView.Show(SW_HIDE);
	}

}


void CPanel::InitTreeList()
{

	treeMove = false;

	DWORD style =TVS_HASBUTTONS | TVS_HASLINES  | TVS_EDITLABELS | WS_BORDER |WS_CHILD|WS_VISIBLE;
	DWORD exStyle = WS_EX_CLIENTEDGE;
	if (!_treeView.CreateEx(exStyle, style, 0, 30, 116, 260,
		HWND(*this), (HMENU)(UINT_PTR)(_TreeViewID), g_hInstance, NULL))
		return ;

#ifndef UNDER_CE
	_treeView.SetUnicodeFormat(true);
#endif
	_treeView.SetUserDataLongPtr(LONG_PTR(&_treeView));
	_treeView._panel = this;

#ifndef _UNICODE
	if(g_IsNT)
		_treeView._origWindowProc =
		(WNDPROC)_treeView.SetLongPtrW(GWLP_WNDPROC, LONG_PTR(TreeViewSubclassProc));
	else
#endif
		_treeView._origWindowProc =
		(WNDPROC)_treeView.SetLongPtr(GWLP_WNDPROC, LONG_PTR(TreeViewSubclassProc));

	_treeView.SetImageList(GetSysImageList(true), LVSIL_SMALL);
	_treeView.SetImageList(GetSysImageList(true), LVSIL_NORMAL);

	InitTree();
}



bool CPanel::AddDriveTree(UString driveName,UString path,bool needchild,HTREEITEM  ParentRoot)
{

	CFindFile Findfile;
	int iconIndex ;//系统图标
	UString SavePath = path;
	UString strPath= SavePath+L"*.*";
	TreePathStruct treePathStruct;

	if (!needchild)
	{
		CFileInfoW info;
		DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
		GetRealIconIndex(path, attrib,iconIndex);
		HTREEITEM Root=_treeView.InsertItem(ParentRoot,driveName,iconIndex);
		treePathStruct.Root=Root;
		treePathStruct.FilePath=path;
		treePathVector.Add(treePathStruct);
	}
	else
	{
		int childNum;//子文件个数
		UStringVector childFileVector;//子文件名
		childFileVector =Findfile.FindDirFile(path);
		childNum=childFileVector.Size();
		CFileInfoW info;
		DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
		if (info.Find(path))attrib = info.Attrib;
		GetRealIconIndex(path, attrib,iconIndex);
		HTREEITEM OneRoot =_treeView.InsertItem(ParentRoot,driveName, iconIndex,childNum);
		treePathStruct.FilePath=path;
		treePathStruct.Root=OneRoot;
		treePathVector.Add(treePathStruct);
	
	}
	return true;
}
bool CPanel::AddTree(UString Filepath,HTREEITEM  ParentRoot,bool Layers,bool PartBool)
{
	CFindFile Findfile;
	int iconIndex ;//系统图标
	int childNum;//子文件个数
	UStringVector childFileVector;//子文件名
	UString ItemName;//文件名称
	UString SavePath = Filepath;
	UString strPath= SavePath+L"*.*";
	TreePathStruct treePathStruct;

	childFileVector =Findfile.FindDirFile(SavePath);
	childNum=childFileVector.Size();

	if (SavePath.Right(1) == L"\\")
	{
		SavePath = SavePath.Left(SavePath.Length()-1);
	}
	while(SavePath.Right(1) != L"\\" && SavePath.Length() != 0)
	{
		ItemName = SavePath.Right(1)+ItemName;
		SavePath = SavePath.Left(SavePath.Length()-1);
	}

	CFileInfoW info;
	DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
	if (info.Find(Filepath))attrib = info.Attrib;
	GetRealIconIndex(Filepath, attrib,iconIndex);


	HTREEITEM OneRoot;
	if(ItemName == L"桌面" || PartBool)
	{
		OneRoot = ParentRoot;
	}
	else
	{
		OneRoot =_treeView.InsertItem(ParentRoot,ItemName, iconIndex,childNum);
		treePathStruct.FilePath=Filepath;
		treePathStruct.Root=OneRoot;
		treePathVector.Add(treePathStruct);
	}

	CFileInfoW fileinfo;
	bool  bWork = Findfile.FindFirst(strPath,fileinfo);

	while(bWork)
	{
		if( (fileinfo.Name != L".") &&(fileinfo.Name != L"..") && bWork && !fileinfo.IsHidden())
		{
			SavePath = Filepath + fileinfo.Name + L"\\";
			if(fileinfo.IsDir())
			{
				childFileVector =Findfile.FindDirFile(SavePath);
				childNum=childFileVector.Size();
				if (info.Find(SavePath))attrib = info.Attrib;
				GetRealIconIndex(SavePath, attrib,iconIndex);
				if(childNum != 0)
				{
					if(Layers)
					{
						AddTree(SavePath,OneRoot,false,false);
					}
					else
					{
						HTREEITEM root = _treeView.InsertItem(OneRoot,fileinfo.Name,iconIndex,childNum);
						treePathStruct.FilePath=SavePath;
						treePathStruct.Root=root;

						treePathVector.Add(treePathStruct);
					}
				}
				else 
				{
					HTREEITEM root = _treeView.InsertItem(OneRoot,fileinfo.Name,iconIndex);
					treePathStruct.FilePath=SavePath;
					treePathStruct.Root=root;
					treePathVector.Add(treePathStruct);
				}
			}
		}

		bWork = Findfile.FindNext(fileinfo);
	}
	return true;
}

#ifndef UNDER_CE
STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);
#endif
extern bool _needReopen;
extern bool WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize);
void CreatMenu()
{

	UString str;
	ReadContextMenu(str);
	if (str == L"true")return;
	if(_needReopen)
	{
		UString path;
		GetProgramFolderPath(path);
		path =  path + L"CoolRAR.exe";
		TCHAR szBuffer[4096];
		memset(szBuffer,0,4096*sizeof(TCHAR));
		WCharToMByte(path.GetBuffer(path.Length()),szBuffer,sizeof(szBuffer)/sizeof(szBuffer[4096]));
		ShellExecute(NULL, "RunAs",szBuffer, "dd", NULL, SW_SHOWNORMAL);
		PostMessage(g_HWND, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)g_HWND);
		PostMessage (g_HWND, WM_CLOSE, 0, 0);
		return;
	}
	str = L"true";
	SaveContextMenu(str);
	CContextMenuInfo ci;
	ci.Load();
#ifndef UNDER_CE
	DllRegisterServer();
	NZipRootRegistry::AddContextMenuHandler();
//	NZipRootRegistry::AddDropHandler();
#endif
	ci.Cascaded = false;
	ci.Flags = 0;
	ci.Flags = 61735;
	ci.Save();
}

static THREAD_FUNC_DECL CoderThread(void *param)
{
	
	Sleep(1000);
	HttpUpdat httpupdat;
	httpupdat.DllUpdatExe(false);
	httpupdat.DestroyDll();
	return 0;

}
UString GetProgramCommandIOCN()
{
	UString path = L"\"";
	UString folder;
	if (GetProgramFolderPath(folder))
		path += folder;
	path += L"CoolRAR.exe\" \"%1\"";
	return path;

}
extern void SplitString(const UString &srcString, UStringVector &destStrings);
static UString GetIconPath(const UString &filePath,
						   const CLSID &clsID, const UString &extension, Int32 &iconIndex)
{
	CPluginLibrary library;
	CMyComPtr<IFolderManager> folderManager;
	CMyComPtr<IFolderFolder> folder;
	if (filePath.IsEmpty())
		folderManager = new CArchiveFolderManager;
	else if (library.LoadAndCreateManager(filePath, clsID, &folderManager) != S_OK)
		return UString();
	CMyComBSTR extBSTR;
	if (folderManager->GetExtensions(&extBSTR) != S_OK)
		return UString();
	const UString ext2 = (const wchar_t *)extBSTR;
	UStringVector exts;
	SplitString(ext2, exts);
	for (int i = 0; i < exts.Size(); i++)
	{
		const UString &plugExt = exts[i];
		if (extension.CompareNoCase((const wchar_t *)plugExt) == 0)
		{
			CMyComBSTR iconPathTemp;
			if (folderManager->GetIconPath(plugExt, &iconPathTemp, &iconIndex) != S_OK)
				break;
			if (iconPathTemp != 0)
				return (const wchar_t *)iconPathTemp;
		}
	}
	return UString();
}

bool JudgeIconPath()
{
	UString LastIconPath,NewIconPath;
	if(!ReadIconPath(LastIconPath))
		return false;

	GetProgramFolderPath(NewIconPath);
	NewIconPath += L"CoolRAR.dll";
	if(NewIconPath != LastIconPath)
		return false;

	return true;
}
void CoolIcon()
{
	CExtDatabase _extDatabase;
	bool notify = false;
	_extDatabase.Read();
	bool isIconPathRight = JudgeIconPath();
	for (int i = 0; i < _extDatabase.ExtBigItems.Size(); i++)
	{
		const CExtInfoBig &extInfo = _extDatabase.ExtBigItems[i];

		UString title = extInfo.Ext + UString(L" Archive");
		UString command = GetProgramCommandIOCN();
		UString iconPath;
		Int32 iconIndex = -1;
		if (!extInfo.PluginsPairs.IsEmpty())
		{
			const CPluginInfo &plugin = _extDatabase.Plugins[extInfo.PluginsPairs[0].Index];
			iconPath = GetIconPath(plugin.FilePath, plugin.ClassID, extInfo.Ext, iconIndex);
		}
		UString iconPathcheck;
		bool loadicon = NRegistryAssociations::CheckShellExtensionInfo(GetSystemString(extInfo.Ext),iconPathcheck,iconIndex);
		if (loadicon && !NWindows::NFile::NFind::DoesFileExist(iconPath))
		{
			loadicon = false;		
		}
		if (!loadicon || !isIconPathRight)
		{
			if(_needReopen)
			{
				UString path;
				GetProgramFolderPath(path);
				path =  path + L"CoolRAR.exe";
				TCHAR szBuffer[4096];
				memset(szBuffer,0,4096*sizeof(TCHAR));
				WCharToMByte(path.GetBuffer(path.Length()),szBuffer,sizeof(szBuffer)/sizeof(szBuffer[4096]));
				ShellExecute(NULL, "RunAs",szBuffer, "dd", NULL, SW_SHOWNORMAL);
				PostMessage(g_HWND, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)g_HWND);
				PostMessage(g_HWND, WM_CLOSE, 0, 0);
				return;
			}
			notify = true; 
			NRegistryAssociations::AddShellExtensionInfo(GetSystemString(extInfo.Ext),
				title, command, iconPath, iconIndex, NULL, 0);
			SaveIconPath(iconPath);
		}
			
	}
#ifndef UNDER_CE
	if(notify)
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
#endif

}


bool CPanel::OnCreate(CREATESTRUCT * /* createStruct */)
{
	
	
	NWindows::CThread  UpdatThread;

	UpdatThread.Create(CoderThread, this);
	UpdatThread.Detach();
	//添加资源管理器视图
	InitTreeList();
	CoolIcon();
	CreatMenu();


  _sortID = kpidName;
  _ascending = true;
  _lastFocusedIsList = true;

  DWORD style = WS_CHILD | WS_VISIBLE; 

  style |= LVS_SHAREIMAGELISTS;
  style |= WS_CLIPCHILDREN;
  style |= WS_CLIPSIBLINGS;
  style |= WS_BORDER;
  const UInt32 kNumListModes = sizeof(kStyles) / sizeof(kStyles[0]);
  if (_ListViewMode >= kNumListModes)
    _ListViewMode = kNumListModes - 1;

  style |= kStyles[_ListViewMode]
    | WS_TABSTOP
    | LVS_EDITLABELS;
  if (_mySelectMode)
    style |= LVS_SINGLESEL;


  DWORD exStyle;
  exStyle = WS_EX_CLIENTEDGE;
  

  if (!_listView.CreateEx(exStyle, style, 40, 0, 116, 260,
      HWND(*this), (HMENU)(UINT_PTR)(_baseID + 1), g_hInstance, NULL))
    return false;

  #ifndef UNDER_CE
  _listView.SetUnicodeFormat(true);
  #endif

  _listView.SetUserDataLongPtr(LONG_PTR(&_listView));
  _listView._panel = this;

   #ifndef _UNICODE
   if(g_IsNT)
     _listView._origWindowProc =
      (WNDPROC)_listView.SetLongPtrW(GWLP_WNDPROC, LONG_PTR(ListViewSubclassProc));
   else
   #endif
     _listView._origWindowProc =
      (WNDPROC)_listView.SetLongPtr(GWLP_WNDPROC, LONG_PTR(ListViewSubclassProc));

   HIMAGELIST listImageList =GetSysImageList(true);
   CThemeDialog themeTitle;
   UString ThemeValue;
   ThemeValue = themeTitle.ReadThemeReg();
   //加载排列顺序箭头
   UString iconpath;
   if (ThemeValue !=L"")
   {
	   UString tempPath;
	   tempPath = themeTitle.GetAppDataPath()+L"\\CoolRAR\\Themes\\";
	   iconpath=tempPath+WSTRING_PATH_SEPARATOR+ThemeValue;
	   HICON upico =(HICON)LoadImageW(g_hInstance,iconpath+L"\\arrowheadup.ico",
		   IMAGE_ICON,
		   0,	
		   0,	
		   LR_LOADFROMFILE|LR_CREATEDIBSECTION
		   );
	   HICON downico =(HICON)LoadImageW(g_hInstance,iconpath+L"\\arrowheaddown.ico",
		   IMAGE_ICON,
		   0,	
		   0,	
		   LR_LOADFROMFILE|LR_CREATEDIBSECTION
		   );
	   _upArrangeIconIndex = ImageList_AddIcon(listImageList,upico);
	   _downArrangeIconIndex = ImageList_AddIcon(listImageList,downico);
   }
   else
   {
	   GetProgramFolderPath(iconpath);
	   HICON upico =(HICON)LoadImageW(g_hInstance,iconpath+L"icon\\arrowheadup.ico",
		   IMAGE_ICON,
		   0,	
		   0,	
		   LR_LOADFROMFILE|LR_CREATEDIBSECTION
		   );
	   HICON downico =(HICON)LoadImageW(g_hInstance,iconpath+L"icon\\arrowheaddown.ico",
		   IMAGE_ICON,
		   0,	
		   0,	
		   LR_LOADFROMFILE|LR_CREATEDIBSECTION
		   );
	   _upArrangeIconIndex = ImageList_AddIcon(listImageList,upico);
	   _downArrangeIconIndex = ImageList_AddIcon(listImageList,downico);
   }

   ImageList_SetBkColor(listImageList,CLR_NONE);
  _listView.SetImageList(listImageList, LVSIL_SMALL);
  _listView.SetImageList(GetSysImageList(false), LVSIL_NORMAL);
  SetExtendedStyle();

  _listView.Show(SW_SHOW);
  _listView.InvalidateRect(NULL, true);
  _listView.Update();
  
  
  // Ensure that the common control DLL is loaded.
  INITCOMMONCONTROLSEX icex;

  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC  = ICC_BAR_CLASSES;
  InitCommonControlsEx(&icex);

  TBBUTTON tbb [ ] =
  {
    
    {VIEW_PARENTFOLDER, kParentFolderID, TBSTATE_ENABLED, BTNS_BUTTON, 0L, 0},
   
  };

  #ifndef UNDER_CE
  if (g_ComCtl32Version >= MAKELONG(71, 4))
  #endif
  {
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);
    
    _headerReBar.Attach(::CreateWindowEx(WS_EX_TOOLWINDOW,
      REBARCLASSNAME,
      NULL, WS_VISIBLE | WS_BORDER | WS_CHILD |
      WS_CLIPCHILDREN | WS_CLIPSIBLINGS
      | CCS_NODIVIDER
      | CCS_TOP
      | RBS_VARHEIGHT
      | RBS_BANDBORDERS
      ,0,0,0,0, HWND(*this), NULL, g_hInstance, NULL));
  }

  DWORD toolbarStyle =  WS_CHILD | WS_VISIBLE ;
  if (_headerReBar)
  {
    toolbarStyle |= 0
      

      | TBSTYLE_TOOLTIPS
      | CCS_NODIVIDER
      | CCS_NORESIZE
      | TBSTYLE_FLAT
      ;
  }

  _headerToolBar.Attach(::CreateToolbarEx ((*this), toolbarStyle,
      _baseID + 2, 11,
      (HINSTANCE)HINST_COMMCTRL,
      IDB_VIEW_SMALL_COLOR,
      (LPCTBBUTTON)&tbb, sizeof(tbb) / sizeof(tbb[0]),
      0, 0, 0, 0, sizeof (TBBUTTON)));

  #ifndef UNDER_CE
  
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC = ICC_USEREX_CLASSES;
  InitCommonControlsEx(&icex);
  #endif
  
  _headerComboBox.CreateEx(0,
      #ifdef UNDER_CE
      WC_COMBOBOXW
      #else
      WC_COMBOBOXEXW
      #endif
      , NULL,
    WS_BORDER | WS_VISIBLE |WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL,
      0, 0, 100, 220,
      ((_headerReBar == 0) ? HWND(*this) : _headerToolBar),
      (HMENU)(UINT_PTR)(_comboBoxID),
      g_hInstance, NULL);
  #ifndef UNDER_CE
  _headerComboBox.SetUnicodeFormat(true);

  _headerComboBox.SetImageList(GetSysImageList(true));

  _headerComboBox.SetExtendedStyle(CBES_EX_PATHWORDBREAKPROC, CBES_EX_PATHWORDBREAKPROC);

 
  _comboBoxEdit.Attach(_headerComboBox.GetEditControl());

  

  _comboBoxEdit.SetUserDataLongPtr(LONG_PTR(&_comboBoxEdit));
  _comboBoxEdit._panel = this;
   #ifndef _UNICODE
   if(g_IsNT)
     _comboBoxEdit._origWindowProc =
      (WNDPROC)_comboBoxEdit.SetLongPtrW(GWLP_WNDPROC, LONG_PTR(ComboBoxEditSubclassProc));
   else
   #endif
     _comboBoxEdit._origWindowProc =
      (WNDPROC)_comboBoxEdit.SetLongPtr(GWLP_WNDPROC, LONG_PTR(ComboBoxEditSubclassProc));

  #endif

  if (_headerReBar)
  {
    REBARINFO     rbi;
    rbi.cbSize = sizeof(REBARINFO);  // Required when using this struct.
    rbi.fMask  = 0;
    rbi.himl   = (HIMAGELIST)NULL;
    _headerReBar.SetBarInfo(&rbi);
    
    // Send the TB_BUTTONSTRUCTSIZE message, which is required for
    // backward compatibility.
    
    SIZE size;
    _headerToolBar.GetMaxSize(&size);

    
    REBARBANDINFO rbBand;
    rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
    rbBand.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
	rbBand.fStyle = RBBS_FIXEDBMP|RBBS_CHILDEDGE;
    rbBand.cxMinChild = size.cx;
    rbBand.cyMinChild = size.cy;
    rbBand.cyChild = size.cy;
    rbBand.cx = size.cx;
    rbBand.hwndChild  = _headerToolBar;
    _headerReBar.InsertBand(-1, &rbBand);

    RECT rc;
    ::GetWindowRect(_headerComboBox, &rc);
    rbBand.cxMinChild = 30;
    rbBand.cyMinChild = rc.bottom - rc.top;
    rbBand.cx = 1000;
    rbBand.hwndChild  = _headerComboBox;
    _headerReBar.InsertBand(-1, &rbBand);
   
  }

  //状态栏开始初始化
  _statusBar.Create(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, L"Status", (*this), _statusBarID);
  int sizes[] = {19 ,42 , 450,  -1};
  _statusBar.SetParts(4, sizes);
  _statusBar._panel =this;
  _statusBar.SetUserDataLongPtr(LONG_PTR(&_statusBar));

#ifndef _UNICODE
  if(g_IsNT)
	  _statusBar._origWindowProc =
	  (WNDPROC)_statusBar.SetLongPtrW(GWLP_WNDPROC, LONG_PTR(StatusBarSubclassProc));
  else
#endif
	  _statusBar._origWindowProc =
	  (WNDPROC)_statusBar.SetLongPtr(GWLP_WNDPROC, LONG_PTR(StatusBarSubclassProc));


 

  SetTimer(kTimerID, kTimerElapse);


  RefreshListCtrl();
  RefreshStatusBar();
  
  return true;
}

void CPanel::OnDestroy()
{
  SaveListViewInfo();
  CWindow2::OnDestroy();
}

void CPanel::ChangTLSzie(int x)
{
	if ((HWND)*this == 0)
		return;
	RECT ListRect;
	RECT TreeRect;
	_treeView.GetWindowRect(&TreeRect);
	_listView.GetWindowRect(&ListRect);

	int kHeaderSize;
	RECT rect;

	if (_headerReBar)
		_headerReBar.GetWindowRect(&rect);
	else
		_headerToolBar.GetWindowRect(&rect);

	kHeaderSize = rect.bottom - rect.top;
	_treeView.Move(0,kHeaderSize,TreeRect.right-TreeRect.left+x,TreeRect.bottom-TreeRect.top,true);
	_listView.Move(TreeRect.right-TreeRect.left+x+3,kHeaderSize,ListRect.right-ListRect.left-x,TreeRect.bottom-TreeRect.top,true);

	
}

void CPanel::ChangeWindowSize(int xSize, int ySize)
{
  if ((HWND)*this == 0)
    return;
  int kHeaderSize;
  int kStatusBarSize;


 
  RECT rect;
  if (_headerReBar)
    _headerReBar.GetWindowRect(&rect);
  else
    _headerToolBar.GetWindowRect(&rect);

  kHeaderSize = rect.bottom - rect.top;

  _statusBar.GetWindowRect(&rect);
  kStatusBarSize = rect.bottom - rect.top;

  int yListViewSize = MyMax(ySize - kHeaderSize - kStatusBarSize, 0);
    _treeviewBottom = yListViewSize;
  const int kStartXPos = 32;
  if (_headerReBar)
  {
  }
  else
  {
    _headerToolBar.Move(0, 0, xSize, 0);
    _headerComboBox.Move(kStartXPos, 2,
        MyMax(xSize - kStartXPos - 10, kStartXPos), 0);
  }




  if ( _TreeViewMode == 0)
  {
	  _treeView.Move(0, kHeaderSize, 250, yListViewSize);
	  _listView.Move(253, kHeaderSize, xSize - 250, yListViewSize);
	  UInt32 x;
	  if(ReadTreeMoveSize(x))            //如果用户有改变树视图大小
	  {
			  if( (int)x < 6)            //如果树视图左边界位置超过可控制的范围
			  {
				  x = 6;                 //将位置设为左边界值
				SaveTreeMoveSize(x);
			  }
			  if ( (int)x > (xSize - 10))//如果树视图右边界位置超过可控制的范围
			  {
				  x = xSize - 10;
				  SaveTreeMoveSize(x);
			  }
		      int pisonX = x - 253;      //253为默认列表视图左边界
			  ChangTLSzie(pisonX);
	  }

  }
  else
  {
	 
	  _listView.Move(0, kHeaderSize, xSize, yListViewSize);

  }
  _statusBar.Move(0, kHeaderSize + yListViewSize, xSize, kStatusBarSize);

  _treeviewTop = kHeaderSize;
  
  

}

bool CPanel::OnSize(WPARAM /* wParam */, int xSize, int ySize)
{
  if ((HWND)*this == 0)
    return true;
  if (_headerReBar)
    _headerReBar.Move(0, 0, xSize, 0);
  ChangeWindowSize(xSize, ySize);
  return true;
}

bool CPanel::OnNotifyReBar(LPNMHDR header, LRESULT & /* result */)
{
  switch(header->code)
  {
    case RBN_HEIGHTCHANGE:
    {
      RECT rect;
      GetWindowRect(&rect);
      ChangeWindowSize(rect.right - rect.left, rect.bottom - rect.top);
      return false;
    }
  }
  return false;
}

bool CPanel::OnNotifyTree(LPNMHDR header)
{


	HTREEITEM _currentRoot = _treeView.GetSelectendItem();
	static HTREEITEM ParentRoot =NULL;
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(header);

	if (pNMTreeView != NULL )
	{
	if ( 2 == pNMTreeView->action)
	{
		if ( ParentRoot != pNMTreeView->itemNew.hItem)
		{
			ParentRoot = pNMTreeView->itemNew.hItem;
			UString path = GetTreeItem(ParentRoot);

			if (path != L"" && changTreeSize == false)
			{
				return	OnTreeClick(ParentRoot);
			}
			else changTreeSize =false;
		}
		else changTreeSize =false;
	}
	else if ( 1 == pNMTreeView->action)
		{
			changTreeSize =false;
		}
	

	}
	switch(header->code)
	{
	case NM_DBLCLK:
		{
			OnTreeClick();
			return true;
		}
	}
	return false;
}

bool CPanel::OnTreeClick(HTREEITEM hItem)
{
	bool BSetText =false;
	HTREEITEM ParentRoot;
	if (hItem == NULL)
	{

		BSetText =true;
		ParentRoot = _treeView.GetSelectendItem();
	}
	else ParentRoot = hItem;

	if(ParentRoot == NULL)return false;
	HTREEITEM ChildRoot = _treeView.GetChildItem(ParentRoot);

	int i;
	for( i=0 ;i<treePathVector.Size();i++)
	{
		TreePathStruct treePathStruct=treePathVector[i];
		if(treePathStruct.Root == ParentRoot)
		{
			break;
		}
	}
	if (i>=3 || i== 1 || i ==0)
	{
		while(ChildRoot != NULL)
		{
			for(int i=0 ;i<treePathVector.Size();i++)
			{
				TreePathStruct treePathStruct=treePathVector[i];
				if(treePathStruct.Root == ChildRoot)
				{
					treePathVector.Delete(i);
				}
			}
			_treeView.DeleteItem(ChildRoot);
			ChildRoot = _treeView.GetChildItem(ParentRoot);
		}

		if (i == 0)
		{

			_treeView.DeleteItem(ParentRoot);
			InitTree();
		}
		else
		{
			UString path = GetTreeItem(ParentRoot);
			AddTree(path,ParentRoot,false,true);	
		}

		
	}

	changTreeSize = true;
	_treeView.Expand(ParentRoot);

if (BSetText)
{
	UString currentFolderPrefix =GetTreeItem(ParentRoot);

	if (currentFolderPrefix == L"")
	{
		return true;
	}
	currentFolderPrefix.DeleteBack();
	_comboBoxEdit.SetText(currentFolderPrefix);
	
	if (BindToPathAndRefresh(GetTreeItem(ParentRoot)) == S_OK)
	{
		PostMessage(kSetFocusToListView);
#ifdef UNDER_CE
		PostMessage(kRefreshHeaderComboBox);
#endif
		return true;
	}
}
	return true;
}

UString CPanel::GetTreeItem(HTREEITEM hItem)
{
	TreePathStruct treePathStruct;
	if (hItem != NULL)
	{
		for(int i=0 ;i<treePathVector.Size();i++)
		{
			treePathStruct=treePathVector[i];
			if(treePathStruct.Root == hItem)return treePathStruct.FilePath;
		}
	}
	return L"";

}


bool CPanel::OnNotify(UINT /* controlID */, LPNMHDR header, LRESULT &result)
{
  if (!_processNotify)
    return false;
  if (header->hwndFrom == _headerComboBox)
    return OnNotifyComboBox(header, result);
  else if (header->hwndFrom == _headerReBar)
    return OnNotifyReBar(header, result);
  else if (header->hwndFrom == _listView)
    return OnNotifyList(header, result);
  else if(header->hwndFrom == _treeView)
	  return OnNotifyTree(header);
  else if (::GetParent(header->hwndFrom) == _listView &&
      header->code == NM_RCLICK)
    return OnRightClick((MY_NMLISTVIEW_NMITEMACTIVATE *)header, result);
  return false;
}

bool CPanel::OnCommand(int code, int itemID, LPARAM lParam, LRESULT &result)
{

	
	 
  if (itemID == kParentFolderID)
  {
    OpenParentFolder();
    result = 0;
    return true;
  }
 
  if (itemID == _comboBoxID)
  {
    if (OnComboBoxCommand(code, lParam, result))
      return true;
  }
  return CWindow2::OnCommand(code, itemID, lParam, result);
}

void CPanel::MessageBoxInfo(LPCWSTR message, LPCWSTR caption)
  { ::MessageBoxW(HWND(*this), message, caption, MB_OK); }
void CPanel::MessageBox(LPCWSTR message, LPCWSTR caption)
  { ::MessageBoxW(HWND(*this), message, caption, MB_OK | MB_ICONSTOP); }
void CPanel::MessageBox(LPCWSTR message)
  { MessageBox(message, L"CoolRAR"); }
void CPanel::MessageBoxMyError(LPCWSTR message)
  { MessageBox(message, L"Error"); }


void CPanel::MessageBoxError(HRESULT errorCode, LPCWSTR caption)
{
  MessageBox(HResultToMessage(errorCode), caption);
}

void CPanel::MessageBoxError(HRESULT errorCode)
  { MessageBoxError(errorCode, L"CoolRAR"); }
void CPanel::MessageBoxLastError(LPCWSTR caption)
  { MessageBoxError(::GetLastError(), caption); }
void CPanel::MessageBoxLastError()
  { MessageBoxLastError(L"Error"); }

void CPanel::MessageBoxErrorLang(UINT resourceID, UInt32 langID)
  { MessageBox(LangString(resourceID, langID)); }


void CPanel::SetFocusToList()
{
  _listView.SetFocus();
  
}

void CPanel::SetFocusToLastRememberedItem()
{
  if (_lastFocusedIsList)
    SetFocusToList();
  else
    _headerComboBox.SetFocus();
}

UString CPanel::GetFolderTypeID() const
{
  NCOM::CPropVariant prop;
  if (_folder->GetFolderProperty(kpidType, &prop) == S_OK)
    if (prop.vt == VT_BSTR)
      return (const wchar_t *)prop.bstrVal;
  return L"";
}

bool CPanel::IsRootFolder() const
{
  return (GetFolderTypeID() == L"RootFolder");
}

bool CPanel::IsFSFolder() const
{
  return (GetFolderTypeID() == L"FSFolder");
}

bool CPanel::IsFSDrivesFolder() const
{
  return (GetFolderTypeID() == L"FSDrives");
}

UString CPanel::GetFsPath() const
{
  if (IsFSDrivesFolder() && !IsDeviceDrivesPrefix())
    return UString();
  return _currentFolderPrefix;
}

UString CPanel::GetDriveOrNetworkPrefix() const
{
  if (!IsFSFolder())
    return UString();
  UString drive = GetFsPath();
  if (drive.Length() < 3)
    return UString();
  if (drive[0] == L'\\' && drive[1] == L'\\')
  {
    // if network
    int pos = drive.Find(L'\\', 2);
    if (pos < 0)
      return UString();
    pos = drive.Find(L'\\', pos + 1);
    if (pos < 0)
      return UString();
    return drive.Left(pos + 1);
  }
  if (drive[1] != L':' || drive[2] != L'\\')
    return UString();
  return drive.Left(3);
}

bool CPanel::DoesItSupportOperations() const
{
  CMyComPtr<IFolderOperations> folderOperations;
  return _folder.QueryInterface(IID_IFolderOperations, &folderOperations) == S_OK;
}

bool CPanel::ListViewModeJudge()
{
	if(_ListViewMode == 2)
	{
		return true;
	}
	return false;
}

bool CPanel::TreeViewModeJudge()
{
	if(_TreeViewMode == 0)
	{
		return true;
	}
	return false;
}

void CPanel::ChangTreeViewMode()
{
	RECT rect;
	_listView.GetWindowRect(&rect);
	RECT comrect;
	_headerComboBox.GetWindowRect(&comrect);
   if(_TreeViewMode == 0)
   {
	   UInt32 x;
	   ReadTreeMoveSize(x);
	   if ( x < 100)//关闭时如果当前树视图过小
	   {
		   SaveTreeMoveSize(100);//设置宽为100
	   }
	   _TreeViewMode =1;
	   _treeView.Show(SW_HIDE);
	   
   }
   else
   {
	   _TreeViewMode =0;
	   _treeView.Show(SW_SHOW);
   }
   GetWindowRect(&rect);
   ChangeWindowSize(rect.right - rect.left, rect.bottom - rect.top);
}


void CPanel::SetListViewMode(UInt32 index)
{
  if (index >= 4)
    return;
  _ListViewMode = index;
  DWORD oldStyle = (DWORD)_listView.GetStyle();
  DWORD newStyle = kStyles[index];
  if ((oldStyle & LVS_TYPEMASK) != newStyle)
    _listView.SetStyle((oldStyle & ~LVS_TYPEMASK) | newStyle);
  
}

void CPanel::ChangeFlatMode()
{
  _flatMode = !_flatMode;
  if (_parentFolders.Size() > 0)
    _flatModeForArc = _flatMode;
  else
    _flatModeForDisk = _flatMode;
  RefreshListCtrlSaveFocused();
}


void CPanel::RefreshStatusBar()
{
  PostMessage(kRefreshStatusBar);
}

void CPanel::AddToArchive()
{
	if (_isInThePressFile)
	{
		UString title =L"请选择要添加的文件或文件夹";
		UString resultPath;
		UString currentPath=L"";
		if (MyBrowseForFolder(HWND(*this), title, currentPath, resultPath))
		{
			UStringVector filepath;
			filepath.Add(resultPath);
			CopyFromNoAsk(filepath);
			return;
		}
		return;
	}
  CRecordVector<UInt32> indices;
  GetOperatedItemIndices(indices);
  if (!IsFsOrDrivesFolder())
  {
    MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
    return;
  }
  if (indices.Size() == 0)
  {
	 
	  ::MessageBoxW(g_HWND,LangString(IDS_SELECT_FILES, 0x03020A03),LangString(0x07000002),MB_ICONWARNING);
    return;
  }
  UStringVector names;

  UString curPrefix = _currentFolderPrefix;
  UString destCurDirPrefix = _currentFolderPrefix;
  if (IsFSDrivesFolder())
  {
    destCurDirPrefix = ROOT_FS_FOLDER;
    if (!IsDeviceDrivesPrefix())
      curPrefix.Empty();
  }

  for (int i = 0; i < indices.Size(); i++)
    names.Add(curPrefix + GetItemRelPath(indices[i]));
  const UString archiveName = CreateArchiveName(names.Front(), (names.Size() > 1), false);
  HRESULT res = CompressFiles(destCurDirPrefix, archiveName, L"", names, false, true, false);
  if (res != S_OK)
  {
    if (destCurDirPrefix.Length() >= MAX_PATH)
      MessageBoxErrorLang(IDS_MESSAGE_UNSUPPORTED_OPERATION_FOR_LONG_PATH_FOLDER, 0x03020A01);
  }
  
}

static UString GetSubFolderNameForExtract(const UString &archiveName)
{
  int slashPos = archiveName.ReverseFind(WCHAR_PATH_SEPARATOR);
  int dotPos = archiveName.ReverseFind(L'.');
  if (dotPos < 0 || slashPos > dotPos)
    return archiveName + UString(L"~");
  UString res = archiveName.Left(dotPos);
  res.TrimRight();
  return res;
}

void CPanel::GetFilePaths(const CRecordVector<UInt32> &indices, UStringVector &paths)
{
  for (int i = 0; i < indices.Size(); i++)
  {
    int index = indices[i];
    if (IsItemFolder(index))
    {
      paths.Clear();
      break;
    }
    paths.Add(GetItemFullPath(index));
  }
  if (paths.Size() == 0)
  {
	::MessageBoxW(NULL,LangString(0x07000043),LangString(0x07000044),MB_ICONWARNING); 
  
    return;
  }
}

extern  void ExtractDricet(const UStringVector &arcPaths, const UString &outFolder, bool showDialog);
//直接解压缩！
void CPanel::ExtractDirectly()
{
	

	if (_parentFolders.Size() > 0 )//是否在压缩文件子路径下
	{
		UString judgeString = _currentFolderPrefix;
		judgeString.DeleteBack();
		int judge;
		while(judgeString.Back() != WCHAR_PATH_SEPARATOR)
		{
			judgeString.DeleteBack();
		}
		judge = judgeString.Length();
		if(_currentFolderPrefix.Find(L".") >judge)//判断是否在压缩文件第一层路径(根据父路径.符号后有没\符号来判断)
		{
			UStringVector pathsAll;
			UString fullpath;
			fullpath =_currentFolderPrefix;
			fullpath.DeleteBack();
			pathsAll.Add(fullpath);
			while(fullpath.Find(L".") >0)
			{
				fullpath.DeleteBack();
			}
			fullpath += WCHAR_PATH_SEPARATOR;
			::ExtractArchives(pathsAll, fullpath , false);
			return;
		}
		while(judgeString.Find(L".") >0)
		{
			judgeString.DeleteBack();
		}
		judgeString += WCHAR_PATH_SEPARATOR;           //处理路径，释放路径设为压缩文件所在路径
		g_App.OnCopyDirectly(false,false,judgeString,0,NExtract::NOverwriteMode::kAskBefore);//如果在压缩文件第二或三层调用
		return;

	}
	CRecordVector<UInt32> indices;
	GetOperatedItemIndices(indices);
	UStringVector paths;
	GetFilePaths(indices, paths);
	if (paths.IsEmpty())
	{
		return;
	}
	UString folderName;
	if (indices.Size() == 1)
		folderName = GetSubFolderNameForExtract(GetItemRelPath(indices[0]));
	else
		folderName = L"*";
	UString path1;
	path1=paths.Front();
	ExtractDricet(paths,_currentFolderPrefix + folderName, false);
}

void CPanel::ExtractTheme(UStringVector paths,UString PathName)
{

	if (paths.IsEmpty())
	{
		return;
	}
	ExtractDricet(paths,PathName, false);
}

void CPanel::ExtractArchives()
{
	if (_parentFolders.Size() > 0 )//是否在压缩文件子路径下
	{
		ExtractArchivesFileByChoose();
		return;
	}
	CRecordVector<UInt32> indices;
	GetOperatedItemIndices(indices);
	UStringVector paths;
	GetFilePaths(indices, paths);

	if (paths.IsEmpty())
		return;
	UString folderName;
	UString filename;
	if (indices.Size() == 1)
	{
		folderName = GetSubFolderNameForExtract(GetItemRelPath(indices[0]));
		filename =GetItemRelPath(indices[0]);
	}
	else
		folderName = L"*";
	if( JudgeFileType(  _currentFolderPrefix + filename) || indices.Size() > 1 )
	{
		::ExtractArchives(paths, _currentFolderPrefix + folderName + UString(WCHAR_PATH_SEPARATOR), true);
	}
	else 
	{
		:: MessageBoxW(g_HWND,LangString(0x07000045),L"CoolRAR",MB_ICONERROR);
	}

}

extern INT_PTR OptionsDialog(HWND hwndOwner, 
							 HINSTANCE /* hInstance */,
							 CConventionalPage pagewc,
							 CAdvancedPage AdvPage,
							 NExtract::NPathMode::EEnum      &PathMode,
							 NExtract::NOverwriteMode::EEnum &OverwriteMode,
							 UString &Path);
void CPanel::ExtractArchivesFileByChoose()
{
	CConventionalPage pageEXInfo;
	CAdvancedPage     pageAdv;
	NExtract::NPathMode::EEnum PathMode;
	NExtract::NOverwriteMode::EEnum OverwriteMode;
	UString path;

	path = _currentFolderPrefix;
	int dotPos = path.ReverseFind(L'.');
	path = path.Left(dotPos);
	path += WCHAR_PATH_SEPARATOR;
	pageEXInfo.DirectoryPath = path;
	INT_PTR res =OptionsDialog(g_HWND,g_hInstance,pageEXInfo,pageAdv,PathMode,OverwriteMode,path);
	if(res !=1 )
		return;
	g_App.OnCopyDirectly(false,false,path,0,OverwriteMode);

}
static void AddValuePair(UINT resourceID, UInt32 langID, UInt64 value, UString &s)
{
  wchar_t sz[32];
  s += LangString(resourceID, langID);
  s += L' ';
  ConvertUInt64ToString(value, sz);
  s += sz;
  s += L'\n';
}

class CThreadTest: public CProgressThreadVirt
{
  HRESULT ProcessVirt();
public:
  CRecordVector<UInt32> Indices;
  CExtractCallbackImp *ExtractCallbackSpec;
  CMyComPtr<IFolderArchiveExtractCallback> ExtractCallback;
  CMyComPtr<IArchiveFolder> ArchiveFolder;
};

HRESULT CThreadTest::ProcessVirt()
{
  RINOK(ArchiveFolder->Extract(&Indices[0], Indices.Size(),
      NExtract::NPathMode::kFullPathnames, NExtract::NOverwriteMode::kAskBefore,
      NULL, BoolToInt(true), ExtractCallback));
  if (ExtractCallbackSpec->IsOK())
  {
    UString s;
    AddValuePair(IDS_FOLDERS_COLON, 0x02000321, ExtractCallbackSpec->NumFolders, s);
    AddValuePair(IDS_FILES_COLON, 0x02000320, ExtractCallbackSpec->NumFiles, s);
    s += L'\n';
    s += LangString(IDS_MESSAGE_NO_ERRORS, 0x02000608);
    OkMessage = s;
  }
  return S_OK;
};



void CPanel::TestArchives()
{
  CRecordVector<UInt32> indices;
  GetOperatedIndicesSmart(indices);
  CMyComPtr<IArchiveFolder> archiveFolder;
  _folder.QueryInterface(IID_IArchiveFolder, &archiveFolder);
  if (archiveFolder)
  {
    {
    CThreadTest extracter;

    extracter.ArchiveFolder = archiveFolder;
    extracter.ExtractCallbackSpec = new CExtractCallbackImp;
    extracter.ExtractCallback = extracter.ExtractCallbackSpec;
    extracter.ExtractCallbackSpec->ProgressDialog = &extracter.ProgressDialog;

    if (indices.IsEmpty())
      return;

    extracter.Indices = indices;
    
    UString title = LangString(IDS_PROGRESS_TESTING, 0x02000F90);
    UString progressWindowTitle = LangString(IDS_APP_TITLE, 0x03000000);
    
    extracter.ProgressDialog.CompressingMode = false;
    extracter.ProgressDialog.MainWindow = GetParent();
    extracter.ProgressDialog.MainTitle = progressWindowTitle;
    extracter.ProgressDialog.MainAddTitle = title + L" ";
    
    extracter.ExtractCallbackSpec->OverwriteMode = NExtract::NOverwriteMode::kAskBefore;
    extracter.ExtractCallbackSpec->Init();
    
    if (extracter.Create(title, GetParent()) != S_OK)
      return;
    
    }
    RefreshTitleAlways();
    return;
  }

  if (!IsFSFolder())
  {
    MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
    return;
  }
  UStringVector paths;
  GetFilePaths(indices, paths);
  if (paths.IsEmpty())
    return;
  ::TestArchives(paths);
}

void CPanel::GetItemMaxCount(int &index)
{
	index= _listView.GetItemCount();
}

void CPanel::UpdateThemeIco(UString ImagePath,int Fouse)
{
	HIMAGELIST listImageList =GetSysImageList(true);
	HICON upico,downico;
	//加载排列顺序箭头
	UString iconpath;
	if (Fouse > 0)
	{
		iconpath=ImagePath;

		upico =(HICON)LoadImageW(g_hInstance,iconpath+L"\\arrowheadup.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		downico =(HICON)LoadImageW(g_hInstance,iconpath+L"\\arrowheaddown.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		_upArrangeIconIndex = ImageList_AddIcon(listImageList,upico);
		_downArrangeIconIndex = ImageList_AddIcon(listImageList,downico);
	}
	else
	{
		GetProgramFolderPath(ImagePath);
		iconpath=ImagePath;

		upico =(HICON)LoadImageW(g_hInstance,iconpath+L"\\icon\\arrowheadup.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		downico =(HICON)LoadImageW(g_hInstance,iconpath+L"\\icon\\arrowheaddown.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		_upArrangeIconIndex = ImageList_AddIcon(listImageList,upico);
		_downArrangeIconIndex = ImageList_AddIcon(listImageList,downico);
	}

	ImageList_SetBkColor(listImageList,CLR_NONE);
	_listView.SetImageList(listImageList, LVSIL_SMALL);
	_listView.SetImageList(GetSysImageList(false), LVSIL_NORMAL);

	RefreshListCtrl(UString(), -1, true, UStringVector());
}


