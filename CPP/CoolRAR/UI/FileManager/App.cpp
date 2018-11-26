// App.cpp

#include "StdAfx.h"

#include "resource.h"
#include "OverwriteDialogRes.h"

#include "Common/IntToString.h"
#include "Common/StringConvert.h"

#include "Windows/COM.h"
#include "Windows/Error.h"
#include "Windows/FileDir.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"
#include "Windows/Thread.h"

#include "App.h"
#include "CopyDialog.h"
#include "ExtractCallback.h"
#include "FormatUtils.h"
#include "IFolder.h"
#include "LangUtils.h"
#include "RegistryUtils.h"
#include "ViewSettings.h"
#include "GuideDialog.h"
#include "BrowseDialog.h"
#include "PsWordDialog.h"
#include "../Common/CompressCall.h"
#include "FavoritesDialog.h"
#include "FavClearUpDialog.h"
#include "Windows/Registry.h"
#include "RegistryUtils.h"
#include "HttpUpdat.h"
#include "OptionsDlgFM.h"
#include "ThemeDialog.h"

#include "Common/DynamicBuffer.h"

#include "Windows/FileIO.h"



using namespace NWindows;
using namespace NFile;
using namespace NFind;

extern DWORD g_ComCtl32Version;
extern HINSTANCE g_hInstance;

static LPCWSTR kTempDirPrefix = L"7zE";

 UString ThemeValue;


void CPanelCallbackImp::OnTab()
{
  if (g_App.NumPanels != 1)
    _app->Panels[1 - _index].SetFocusToList();
  _app->RefreshTitle();
}

void CPanelCallbackImp::SetFocusToPath(int index)
{
  int newPanelIndex = index;
  if (g_App.NumPanels == 1)
    newPanelIndex = g_App.LastFocusedPanel;
  _app->RefreshTitle();
  _app->Panels[newPanelIndex]._headerComboBox.SetFocus();
  _app->Panels[newPanelIndex]._headerComboBox.ShowDropDown();
}


void CPanelCallbackImp::OnCopy(bool move, bool copyToSame) { _app->OnCopy(move, copyToSame, _index); }
void CPanelCallbackImp::OnSetSameFolder() { _app->OnSetSameFolder(_index); }
void CPanelCallbackImp::OnSetSubFolder()  { _app->OnSetSubFolder(_index); }
void CPanelCallbackImp::PanelWasFocused() { _app->SetFocusedPanel(_index); _app->RefreshTitle(_index); }
void CPanelCallbackImp::DragBegin() { _app->DragBegin(_index); }
void CPanelCallbackImp::DragEnd() { _app->DragEnd(); }
void CPanelCallbackImp::RefreshTitle(bool always) { _app->RefreshTitle(_index, always); }

void CApp::SetListSettings()
{
  bool showDots = ReadShowDots();
  bool showRealFileIcons = ReadShowRealFileIcons();

  DWORD extendedStyle = LVS_EX_HEADERDRAGDROP;
  if (ReadFullRow())
    extendedStyle |= LVS_EX_FULLROWSELECT;
  if (ReadShowGrid())
    extendedStyle |= LVS_EX_GRIDLINES;
  bool mySelectionMode = ReadAlternativeSelection();
  
  if (ReadSingleClick())
  {
    extendedStyle |= LVS_EX_ONECLICKACTIVATE | LVS_EX_TRACKSELECT;

  }

  for (int i = 0; i < kNumPanelsMax; i++)
  {
    CPanel &panel = Panels[i];
    panel._mySelectMode = mySelectionMode;
    panel._showDots = showDots;
    panel._showRealFileIcons = showRealFileIcons;
    panel._exStyle = extendedStyle;

    DWORD style = (DWORD)panel._listView.GetStyle();
    if (mySelectionMode)
      style |= LVS_SINGLESEL;
    else
      style &= ~LVS_SINGLESEL;
    panel._listView.SetStyle(style);
    panel.SetExtendedStyle();
  }
}

void CApp::SetShowSystemMenu()
{
  ShowSystemMenu = ReadShowSystemMenu();
}

#ifndef ILC_COLOR32
#define ILC_COLOR32 0x0020
#endif

HRESULT CApp::CreateOnePanel(int panelIndex, const UString &mainPath, bool &archiveIsOpened, bool &encrypted)
{
  if (PanelsCreated[panelIndex])
    return S_OK;
  m_PanelCallbackImp[panelIndex].Init(this, panelIndex);
  UString path;
  if (mainPath.IsEmpty())
  {
    if (!::ReadPanelPath(panelIndex, path))
	{
		path=L"桌面";
	}
      
  }
  else
    path = mainPath;
  int id = 1000 + 100 * panelIndex;
  RINOK(Panels[panelIndex].Create(_window, _window,
      id, path, &m_PanelCallbackImp[panelIndex], &AppState, archiveIsOpened, encrypted));
  PanelsCreated[panelIndex] = true;
  return S_OK;
}

static void CreateToolbar(HWND parent,
    NWindows::NControl::CImageList &imageList,
    NWindows::NControl::CToolBar &toolBar,
    bool largeButtons)
{
  toolBar.Attach(::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 0
      | WS_CHILD
      | WS_VISIBLE
      | TBSTYLE_FLAT
      | TBSTYLE_TOOLTIPS
      | TBSTYLE_WRAPABLE
	  | CCS_NODIVIDER

	  

   
      ,0,0,0,0, parent, NULL, g_hInstance, NULL));

  
  toolBar.ButtonStructSize();

  imageList.Create(
      largeButtons ? 48: 24,
      largeButtons ? 36: 24,
      ILC_MASK | ILC_COLOR32, 0, 0);
  toolBar.SetImageList(0, imageList);
  SIZE size;
  toolBar.GetMaxSize(&size);

}

struct CButtonInfo
{
  int CommandID;
  UINT BitmapResID;
  UINT Bitmap2ResID;
  UINT StringResID;
  UInt32 LangID;
  wchar_t* bmpName;
  UString GetText() const { return LangString(StringResID, LangID); }
};


static CButtonInfo g_StandardButtons[] =
{
	{ IDM_CHECKFILE_LAGEBUTTON, IDB_CHECK, IDB_CHECK2, IDS_BUTTON_CHECK, 0x03000108, L"Check.bmp"},
	{ IDM_DELETEFILE_LAGEBUTTON, IDB_DELETE, IDB_DELETE2, IDS_BUTTON_DELETE, 0x03020422, L"Delate.bmp"},
	{ IDM_SEARCHFILE_LAGEBUTTON, IDB_FIND, IDB_FIND, IDS_BUTTON_FIND, 0x03020424, L"Find.bmp"},
	{ IDM_GUIDE_LAGEBUTTON, IDB_GUIDER, IDB_GUIDER2, IDS_BUTTON_GUIDE, 0x03020436, L"xiangdao.bmp"} ,
	{ IDM_GETINFORMATION_LAGEBUTTON, IDB_INFO, IDB_INFO2, IDS_BUTTON_INFO, 0x03020423, L"Info.bmp"},
	{ IDM_REPAIRFILE_LAGEBUTTON, IDB_REPAIRFAST, IDB_REPAIR2, IDS_BUTTON_REPAIR ,0x03020435, L"xiufu.bmp"},
	{ IDM_ADDANNOTATION,IDB_ANNOTION,IDB_ANNOTION,IDS_BUTTON_ANNOTION,0x03020437, L"zhushi.bmp"}

};

static CButtonInfo g_ArchiveButtons[] =
{
	{ kAddCommand, IDB_ADD, IDB_ADD2, IDS_ADD, 0x03020400, L"Add.bmp"},
	{ kExtractCommand, IDB_EXTRACT, IDB_EXTRACT2, IDS_EXTRACT, 0x03020401, L"Extract.bmp"},
	{ kTestCommand , IDB_TEST, IDB_TEST2, IDS_TEST, 0x03020402, L"Test.bmp"}
};

static bool SetButtonText(int commandID, CButtonInfo *buttons, int numButtons, UString &s)
{
  for (int i = 0; i < numButtons; i++)
  {
    const CButtonInfo &b = buttons[i];
    if (b.CommandID == commandID)
    {
      s = b.GetText();
      return true;
    }
  }
  return false;
}

static void SetButtonText(int commandID, UString &s)
{
  if (SetButtonText(commandID, g_StandardButtons,
      sizeof(g_StandardButtons) / sizeof(g_StandardButtons[0]), s))
    return;
  SetButtonText(commandID, g_ArchiveButtons,
      sizeof(g_ArchiveButtons) / sizeof(g_ArchiveButtons[0]), s);
}

static void AddButton(
    NControl::CImageList &imageList,
    NControl::CToolBar &toolBar,
    CButtonInfo &butInfo, bool showText, bool large)
{
  TBBUTTON but;
  but.iBitmap = 0;
  but.idCommand = butInfo.CommandID;
  but.fsState = TBSTATE_ENABLED;
  but.fsStyle = TBSTYLE_BUTTON;
  but.dwData = 0;

  UString s = butInfo.GetText();
  but.iString = 0;
  if (showText)
    but.iString = (INT_PTR)(LPCWSTR)s;

  but.iBitmap = imageList.GetImageCount();
  HBITMAP b = ::LoadBitmap(g_hInstance,
      large ?
      MAKEINTRESOURCE(butInfo.BitmapResID):
      MAKEINTRESOURCE(butInfo.Bitmap2ResID));
  if (b != 0)
  {
    imageList.AddMasked(b, RGB(212,208,199));
    ::DeleteObject(b);
  }
  #ifdef _UNICODE
  toolBar.AddButton(1, &but);
  #else
  toolBar.AddButtonW(1, &but);
  #endif
}

void CApp::ReloadToolbars()
{
	_buttonsImageList.Destroy();
	_toolBar.Destroy();

	CThemeDialog ThemeTitle;
	ThemeValue=ThemeTitle.ReadThemeReg();

	UString ThemesPath,BtnBmpPath;
	UString Pathtemp = ThemeTitle.GetAppDataPath();
	ThemesPath = Pathtemp+WSTRING_PATH_SEPARATOR+L"CoolRAR"+WSTRING_PATH_SEPARATOR+L"Themes";
	BtnBmpPath = ThemesPath+WSTRING_PATH_SEPARATOR+ThemeValue+WSTRING_PATH_SEPARATOR+L"toolbar";

	if (ThemeValue !=L"")
	{
		if (ShowArchiveToolbar || ShowStandardToolbar)
		{
			CreateToolbar(_window, _buttonsImageList, _toolBar, LargeButtons);
			int i;
			if (ShowArchiveToolbar)
				for (i = 0; i < sizeof(g_ArchiveButtons) / sizeof(g_ArchiveButtons[0]); i++)
				{
					AddButton(_buttonsImageList, _toolBar, g_ArchiveButtons[i], ShowButtonsLables, LargeButtons);
				} 
				if (ShowStandardToolbar)
					for (i = 0; i < sizeof(g_StandardButtons) / sizeof(g_StandardButtons[0]); i++)
					{
						if (i == 5)
						{

							TBBUTTON but;
							but.iBitmap = 0;
							but.idCommand = IDS_SEPARATOR;
							but.fsState = TBSTATE_INDETERMINATE;
							but.fsStyle = TBSTYLE_SEP;
							but.dwData = 0;
							but.iString = 0;
							_toolBar.AddButtonW(1,&but);
						}
						AddButton(_buttonsImageList, _toolBar, g_StandardButtons[i], ShowButtonsLables, LargeButtons);
					}

					_toolBar.AutoSize();
		}
		g_App.SaveThemeBmp(BtnBmpPath);
	}
	else
	{
		if (ShowArchiveToolbar || ShowStandardToolbar)
		{
			CreateToolbar(_window, _buttonsImageList, _toolBar, LargeButtons);
			int i;
			if (ShowArchiveToolbar)
				for (i = 0; i < sizeof(g_ArchiveButtons) / sizeof(g_ArchiveButtons[0]); i++)
				{
					AddButton(_buttonsImageList, _toolBar, g_ArchiveButtons[i], ShowButtonsLables, LargeButtons);
				} 
				if (ShowStandardToolbar)
					for (i = 0; i < sizeof(g_StandardButtons) / sizeof(g_StandardButtons[0]); i++)
					{
						if (i == 5)
						{

							TBBUTTON but;
							but.iBitmap = 0;
							but.idCommand = IDS_SEPARATOR;
							but.fsState = TBSTATE_INDETERMINATE;
							but.fsStyle = TBSTYLE_SEP;
							but.dwData = 0;
							but.iString = 0;
							_toolBar.AddButtonW(1,&but);
						}
						AddButton(_buttonsImageList, _toolBar, g_StandardButtons[i], ShowButtonsLables, LargeButtons);
					}

					_toolBar.AutoSize();
		}
	}
}

void CApp::ThemeDefault()
{
	_buttonsImageList.Destroy();
	_toolBar.Destroy();

	if (ShowArchiveToolbar || ShowStandardToolbar)
	{
		CreateToolbar(_window, _buttonsImageList, _toolBar, LargeButtons);
		int i;
		if (ShowArchiveToolbar)
			for (i = 0; i < sizeof(g_ArchiveButtons) / sizeof(g_ArchiveButtons[0]); i++)
			{
				AddButton(_buttonsImageList, _toolBar, g_ArchiveButtons[i], ShowButtonsLables, LargeButtons);
			} 
			if (ShowStandardToolbar)
				for (i = 0; i < sizeof(g_StandardButtons) / sizeof(g_StandardButtons[0]); i++)
				{
					if (i == 5)
			  {

				  TBBUTTON but;
				  but.iBitmap = 0;
				  but.idCommand = IDS_SEPARATOR;
				  but.fsState = TBSTATE_INDETERMINATE;
				  but.fsStyle = TBSTYLE_SEP;
				  but.dwData = 0;
				  but.iString = 0;
				  _toolBar.AddButtonW(1,&but);
			  }
					AddButton(_buttonsImageList, _toolBar, g_StandardButtons[i], ShowButtonsLables, LargeButtons);
				}
				_toolBar.AutoSize();
	}
}
//获取主题
void CApp::SaveThemeBmp(UString iconBagPath)
{
	iconBagPath += L"\\";
	_buttonsImageList.RemoveAll();

	if (ShowArchiveToolbar || ShowStandardToolbar)
	{
		int i;
		if (ShowArchiveToolbar)
		{
			for ( i = 0; i < sizeof(g_ArchiveButtons) / sizeof(g_ArchiveButtons[0]); i++)
			{
				UString iconPath = iconBagPath;
				iconPath += g_ArchiveButtons[i].bmpName;

				HBITMAP b = (HBITMAP)::LoadImageW(g_hInstance,iconPath,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE |LR_LOADFROMFILE);

				if (b != 0)
				{
					_buttonsImageList.AddMasked(b, RGB(212,208,199));
					::DeleteObject(b);
				}
			}
		}
		if (ShowStandardToolbar)
		{
			for ( i = 0; i < sizeof(g_StandardButtons) / sizeof(g_StandardButtons[0]); i++)
			{
				UString iconPath = iconBagPath;
				iconPath += g_StandardButtons[i].bmpName;
				HBITMAP b = (HBITMAP)::LoadImageW(g_hInstance,iconPath,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE |LR_LOADFROMFILE);
				if (b != 0)
				{
					_buttonsImageList.AddMasked(b, RGB(212,208,199));
					::DeleteObject(b);
				}
			}
		}
	}
	RECT rcCleint;
	_toolBar.GetClientRect(&rcCleint);//获取客户区.
	_toolBar.InvalidateRect(&rcCleint);
}

void CApp::SaveToolbarChanges()
{
  SaveToolbar();
  ReloadToolbars();
  MoveSubWindows();
}

void MyLoadMenu();

extern void SetupDefaultPassword(UString newpassword);
HRESULT CApp::Create(HWND hwnd, const UString &mainPath, int xSizes[2], bool &archiveIsOpened, bool &encrypted)
{
  _window.Attach(hwnd);
  #ifdef UNDER_CE
  _commandBar.Create(g_hInstance, hwnd, 1);
  #endif
  MyLoadMenu();
  #ifdef UNDER_CE
  _commandBar.AutoSize();
  #endif

  ReadToolbar();
  ReloadToolbars();

  int i;
  for (i = 0; i < kNumPanelsMax; i++)
    PanelsCreated[i] = false;

  AppState.Read();
  SetListSettings();
  SetShowSystemMenu();
  if (LastFocusedPanel >= kNumPanelsMax)
    LastFocusedPanel = 0;


  CTreeMode treeMode;
  ReadTreeMode(treeMode);

  CListMode listMode;
  ReadListMode(listMode);
  for (i = 0; i < kNumPanelsMax; i++)
  {
    CPanel &panel = Panels[i];
    panel._ListViewMode = listMode.Panels[i];
	panel._TreeViewMode = treeMode.Panels[i];
    panel._xSize = xSizes[i];
     panel._flatModeForArc = ReadFlatView(i);
  }
  for (i = 0; i < kNumPanelsMax; i++)
    if (NumPanels > 1 || i == LastFocusedPanel)
    {
      if (NumPanels == 1)
        Panels[i]._xSize = xSizes[0] + xSizes[1];
      bool archiveIsOpened2 = false;
      bool encrypted2 = false;
      bool mainPanel = (i == LastFocusedPanel);
      RINOK(CreateOnePanel(i, mainPanel ? mainPath : L"", archiveIsOpened2, encrypted2));
      if (mainPanel)
      {
        archiveIsOpened = archiveIsOpened2;
        encrypted = encrypted2;
      }
    }
  SetFocusedPanel(LastFocusedPanel);
  Panels[LastFocusedPanel].SetFocusToList();
  UString newPassword;
  if(ReadPassWord(newPassword))
  SetupDefaultPassword(newPassword);//读取默认密码
  return S_OK;
}

HRESULT CApp::SwitchOnOffOnePanel()
{
  if (NumPanels == 1)
  {
    NumPanels++;
    bool archiveIsOpened, encrypted;
    RINOK(CreateOnePanel(1 - LastFocusedPanel, UString(), archiveIsOpened, encrypted));
    Panels[1 - LastFocusedPanel].Enable(true);
    Panels[1 - LastFocusedPanel].Show(SW_SHOWNORMAL);
  }
  else
  {
    NumPanels--;
    Panels[1 - LastFocusedPanel].Enable(false);
    Panels[1 - LastFocusedPanel].Show(SW_HIDE);
  }
  MoveSubWindows();
  return S_OK;
}

extern void SaveDefaultPassword(UString &savepassword);
void CApp::Save()
{
  AppState.Save();
  CListMode listMode;
  CTreeMode treeMode;
  for (int i = 0; i < kNumPanelsMax; i++)
  {
    const CPanel &panel = Panels[i];
    UString path;
    if (panel._parentFolders.IsEmpty())
      path = panel._currentFolderPrefix;
    else
      path = GetFolderPath(panel._parentFolders[0].ParentFolder);
    SavePanelPath(i, path);
    listMode.Panels[i] = panel.GetListViewMode();
	treeMode.Panels[i] = panel.GetTreeViewMode();
    SaveFlatView(i, panel._flatModeForArc);
  }
  SaveListMode(listMode);



  SaveTreeMode(treeMode);
  UString savedefaultPsWord;
  SaveDefaultPassword(savedefaultPsWord);
  SavePassWord(savedefaultPsWord);   //保存当前设置的默认密码
}

void CApp::Release()
{
  // It's for unloading COM dll's: don't change it.
  for (int i = 0; i < kNumPanelsMax; i++)
    Panels[i].Release();
}

// reduces path to part that exists on disk
static void ReducePathToRealFileSystemPath(UString &path)
{
  while (!path.IsEmpty())
  {
    if (NFind::DoesDirExist(path))
    {
      NName::NormalizeDirPathPrefix(path);
      break;
    }
    int pos = path.ReverseFind(WCHAR_PATH_SEPARATOR);
    if (pos < 0)
      path.Empty();
    else
    {
      path = path.Left(pos + 1);
      if (path.Length() == 3 && path[1] == L':')
        break;
      if (path.Length() > 2 && path[0] == '\\' && path[1] == '\\')
      {
        int nextPos = path.Find(WCHAR_PATH_SEPARATOR, 2); // pos after \\COMPNAME
        if (nextPos > 0 && path.Find(WCHAR_PATH_SEPARATOR, nextPos + 1) == pos)
          break;
      }
      path = path.Left(pos);
    }
  }
}

// return true for dir\, if dir exist
static bool CheckFolderPath(const UString &path)
{
  UString pathReduced = path;
  ReducePathToRealFileSystemPath(pathReduced);
  return (pathReduced == path);
}

static bool IsPathAbsolute(const UString &path)
{
  if (path.Length() >= 1 && path[0] == WCHAR_PATH_SEPARATOR)
    return true;
  #ifdef _WIN32
  if (path.Length() >= 3 && path[1] == L':' && path[2] == L'\\')
    return true;
  #endif
  return false;
}

extern UString ConvertSizeToString(UInt64 value);

static UString AddSizeValue(UInt64 size)
{
  return MyFormatNew(IDS_FILE_SIZE, 0x02000982, ConvertSizeToString(size));
}

static void AddValuePair1(UINT resourceID, UInt32 langID, UInt64 size, UString &s)
{
  s += LangString(resourceID, langID);
  s += L" ";
  s += AddSizeValue(size);
  s += L"\n";
}

void AddValuePair2(UINT resourceID, UInt32 langID, UInt64 num, UInt64 size, UString &s)
{
  if (num == 0)
    return;
  s += LangString(resourceID, langID);
  s += L" ";
  s += ConvertSizeToString(num);

  if (size != (UInt64)(Int64)-1)
  {
    s += L"    ( ";
    s += AddSizeValue(size);
    s += L" )";
  }
  s += L"\n";
}

static void AddPropValueToSum(IFolderFolder *folder, int index, PROPID propID, UInt64 &sum)
{
  if (sum == (UInt64)(Int64)-1)
    return;
  NCOM::CPropVariant prop;
  folder->GetProperty(index, propID, &prop);
  switch(prop.vt)
  {
    case VT_UI4:
    case VT_UI8:
      sum += ConvertPropVariantToUInt64(prop);
      break;
    default:
      sum = (UInt64)(Int64)-1;
  }
}

UString CPanel::GetItemsInfoString(const CRecordVector<UInt32> &indices)
{
  UString info;
  UInt64 numDirs, numFiles, filesSize, foldersSize;
  numDirs = numFiles = filesSize = foldersSize = 0;
  int i;
  for (i = 0; i < indices.Size(); i++)
  {
    int index = indices[i];
    if (IsItemFolder(index))
    {
      AddPropValueToSum(_folder, index, kpidSize, foldersSize);
      numDirs++;
    }
    else
    {
      AddPropValueToSum(_folder, index, kpidSize, filesSize);
      numFiles++;
    }
  }

  AddValuePair2(IDS_FOLDERS_COLON, 0x02000321, numDirs, foldersSize, info);
  AddValuePair2(IDS_FILES_COLON, 0x02000320, numFiles, filesSize, info);
  int numDefined = ((foldersSize != (UInt64)(Int64)-1) && foldersSize != 0) ? 1: 0;
  numDefined += ((filesSize != (UInt64)(Int64)-1) && filesSize != 0) ? 1: 0;
  if (numDefined == 2)
    AddValuePair1(IDS_SIZE_COLON, 0x02000322, filesSize + foldersSize, info);
  
  info += L"\n";
  info += _currentFolderPrefix;
  
  for (i = 0; i < indices.Size() && i < kCopyDialog_NumInfoLines - 6; i++)
  {
    info += L"\n  ";
    int index = indices[i];
    info += GetItemRelPath(index);
    if (IsItemFolder(index))
      info += WCHAR_PATH_SEPARATOR;
  }
  if (i != indices.Size())
    info += L"\n  ...";
  return info;
}


void CApp::PressFileSave(int srcPanelIndex,UString savePath)
{
	bool move=false;
	bool copyToSame =false;
	int destPanelIndex = (NumPanels <= 1) ? srcPanelIndex : (1 - srcPanelIndex);
	CPanel &srcPanel = Panels[srcPanelIndex];
	CPanel &destPanel = Panels[destPanelIndex];

	CPanel::CDisableTimerProcessing disableTimerProcessing1(destPanel);
	CPanel::CDisableTimerProcessing disableTimerProcessing2(srcPanel);

	if (!srcPanel.DoesItSupportOperations())
	{
		srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
		return;
	}

	CRecordVector<UInt32> indices;
	bool useDestPanel = false;

	
		if (copyToSame)
		{
			int focusedItem = srcPanel._listView.GetFocusedItem();
			if (focusedItem < 0)
				return;
			int realIndex = srcPanel.GetRealItemIndex(focusedItem);
			if (realIndex == kParentIndex)
				return;
			indices.Add(realIndex);
		}
		else
		{
			srcPanel.GetOperatedIndicesSmart(indices);
			if (indices.Size() == 0)
				return;
			
		}

		
#ifndef UNDER_CE
		if (savePath.Length() > 0 && savePath[0] == '\\')
			if (savePath.Length() == 1 || savePath[1] != '\\')
			{
				srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
				return;
			}
#endif

			if (indices.Size() > 1 || (!savePath.IsEmpty() && savePath.Back() == WCHAR_PATH_SEPARATOR) ||
				NFind::DoesDirExist(savePath))
			{
				NDirectory::CreateComplexDirectory(savePath);
				NName::NormalizeDirPathPrefix(savePath);
				if (!CheckFolderPath(savePath))
				{
					if (NumPanels < 2 || savePath != destPanel._currentFolderPrefix || !destPanel.DoesItSupportOperations())
					{
						srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
						return;
					}
					useDestPanel = true;
				}
			}
			else
			{
				int pos = savePath.ReverseFind(WCHAR_PATH_SEPARATOR);
				if (pos >= 0)
				{
					UString prefix = savePath.Left(pos + 1);
					NDirectory::CreateComplexDirectory(prefix);
					if (!CheckFolderPath(prefix))
					{
						srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
						return;
					}
				}
			}


	bool useSrcPanel = (!useDestPanel || !srcPanel.IsFsOrDrivesFolder() || destPanel.IsFSFolder());
	bool useTemp = useSrcPanel && useDestPanel;
	NFile::NDirectory::CTempDirectoryW tempDirectory;
	UString tempDirPrefix;
	if (useTemp)
	{
		tempDirectory.Create(kTempDirPrefix);
		tempDirPrefix = tempDirectory.GetPath();
		NFile::NName::NormalizeDirPathPrefix(tempDirPrefix);
	}

	CSelectedState srcSelState;
	CSelectedState destSelState;
	srcPanel.SaveSelectedState(srcSelState);
	destPanel.SaveSelectedState(destSelState);

	HRESULT result;
	if (useSrcPanel)
	{
		UString folder = useTemp ? tempDirPrefix : savePath;
	
		result = srcPanel.CopyTo(indices, folder, move, true, 0);

		if (result != S_OK)
		{
			disableTimerProcessing1.Restore();
			disableTimerProcessing2.Restore();
			// For Password:
			srcPanel.SetFocusToList();
			if (result != E_ABORT)
				srcPanel.MessageBoxError(result, L"Error");
			return;
		}
	}

	if (useDestPanel)
	{
		UStringVector filePaths;
		UString folderPrefix;
		if (useTemp)
			folderPrefix = tempDirPrefix;
		else
			folderPrefix = srcPanel._currentFolderPrefix;
		filePaths.Reserve(indices.Size());
		for (int i = 0; i < indices.Size(); i++)
			filePaths.Add(srcPanel.GetItemRelPath(indices[i]));

		result = destPanel.CopyFrom(folderPrefix, filePaths, true, 0);

		if (result != S_OK)
		{
			disableTimerProcessing1.Restore();
			disableTimerProcessing2.Restore();
			// For Password:
			srcPanel.SetFocusToList();
			if (result != E_ABORT)
				srcPanel.MessageBoxError(result, L"Error");
			return;
		}
	}

	RefreshTitleAlways();
	if (copyToSame || move)
	{
		srcPanel.RefreshListCtrl(srcSelState);
	}
	if (!copyToSame)
	{
		destPanel.RefreshListCtrl(destSelState);
		srcPanel.KillSelection();
	}
	disableTimerProcessing1.Restore();
	disableTimerProcessing2.Restore();
	srcPanel.SetFocusToList();
}

void CApp::OnCopyDirectly(bool move, bool copyToSame, UString destPath,int srcPanelIndex,NExtract::NOverwriteMode::EEnum overwritemode)
{
  UString directpath = destPath;
  int destPanelIndex = (NumPanels <= 1) ? srcPanelIndex : (1 - srcPanelIndex);
  CPanel &srcPanel = Panels[srcPanelIndex];
  CPanel &destPanel = Panels[destPanelIndex];

  CPanel::CDisableTimerProcessing disableTimerProcessing1(destPanel);
  CPanel::CDisableTimerProcessing disableTimerProcessing2(srcPanel);

  if (!srcPanel.DoesItSupportOperations())
  {
    srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
    return;
  }

  CRecordVector<UInt32> indices;
  
  bool useDestPanel = false;

  {
    if (copyToSame)
    {
		int focusedItem = srcPanel._listView.GetFocusedItem();
		if (focusedItem < 0)
			return;
		int realIndex = srcPanel.GetRealItemIndex(focusedItem);
		if (realIndex == kParentIndex)//如果用户在压缩包内未选择单个文件解压
			srcPanel.GetAllItemIndices(indices);
		else
			indices.Add(realIndex);
    }
    else
    {
      srcPanel.GetOperatedIndicesSmart(indices);
      if (indices.Size() == 0)
        return;
    }
	if ( destPath.Back() != WCHAR_PATH_SEPARATOR)
		destPath += WCHAR_PATH_SEPARATOR;
    if (destPath.IsEmpty())
    {
      srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
      return;
    }
	UStringVector copyFolders;
	ReadCopyHistory(copyFolders);
    if (!IsPathAbsolute(destPath))
    {
      if (!srcPanel.IsFSFolder())
      {
        srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
        return;
      }
      destPath = srcPanel._currentFolderPrefix + destPath;
    }

    #ifndef UNDER_CE
    if (destPath.Length() > 0 && destPath[0] == '\\')
      if (destPath.Length() == 1 || destPath[1] != '\\')
      {
        srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
        return;
      }
    #endif

    if (indices.Size() > 1 || (!destPath.IsEmpty() && destPath.Back() == WCHAR_PATH_SEPARATOR) ||
        NFind::DoesDirExist(destPath))
    {
      NDirectory::CreateComplexDirectory(destPath);
      NName::NormalizeDirPathPrefix(destPath);
      if (!CheckFolderPath(destPath))
      {
        if (NumPanels < 2 || destPath != destPanel._currentFolderPrefix || !destPanel.DoesItSupportOperations())
        {
          srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
          return;
        }
        useDestPanel = true;
      }
    }
    else
    {
      int pos = destPath.ReverseFind(WCHAR_PATH_SEPARATOR);
      if (pos >= 0)
      {
        UString prefix = destPath.Left(pos + 1);
        NDirectory::CreateComplexDirectory(prefix);
        if (!CheckFolderPath(prefix))
        {
          srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
          return;
        }
      }
    }
	
    AddUniqueStringToHeadOfList(copyFolders, destPath);
    while (copyFolders.Size() > 20)
      copyFolders.DeleteBack();
    SaveCopyHistory(copyFolders);
  }

  bool useSrcPanel = (!useDestPanel || !srcPanel.IsFsOrDrivesFolder() || destPanel.IsFSFolder());
  bool useTemp = useSrcPanel && useDestPanel;
  NFile::NDirectory::CTempDirectoryW tempDirectory;
  UString tempDirPrefix;
  if (useTemp)
  {
    tempDirectory.Create(kTempDirPrefix);
    tempDirPrefix = tempDirectory.GetPath();
    NFile::NName::NormalizeDirPathPrefix(tempDirPrefix);
  }

  CSelectedState srcSelState;
  CSelectedState destSelState;
  srcPanel.SaveSelectedState(srcSelState);
  destPanel.SaveSelectedState(destSelState);

  HRESULT result;
  if (useSrcPanel)
  {
    UString folder = useTemp ? tempDirPrefix : destPath;
	UInt32 s;
	for(int i=0;i<=indices.Size();i++)
	{ 
		s =indices.Back();}
    result = srcPanel.CopyTo(indices, folder, move, true, 0,overwritemode);
	
    if (result != S_OK)
    {
      disableTimerProcessing1.Restore();
      disableTimerProcessing2.Restore();
      // For Password:
      srcPanel.SetFocusToList();
      if (result != E_ABORT)
        srcPanel.MessageBoxError(result, L"Error");
      return;
    }
  }
  
  if (useDestPanel)
  {
    UStringVector filePaths;
    UString folderPrefix;
    if (useTemp)
      folderPrefix = tempDirPrefix;
    else
      folderPrefix = srcPanel._currentFolderPrefix;
    filePaths.Reserve(indices.Size());
    for (int i = 0; i < indices.Size(); i++)
      filePaths.Add(srcPanel.GetItemRelPath(indices[i]));

    result = destPanel.CopyFrom(folderPrefix, filePaths, true, 0);

    if (result != S_OK)
    {
      disableTimerProcessing1.Restore();
      disableTimerProcessing2.Restore();
      // For Password:
      srcPanel.SetFocusToList();
      if (result != E_ABORT)
        srcPanel.MessageBoxError(result, L"Error");
      return;
    }
  }

  RefreshTitleAlways();
  if (copyToSame || move)
  {
    srcPanel.RefreshListCtrl(srcSelState);
  }
  if (!copyToSame)
  {
    destPanel.RefreshListCtrl(destSelState);
    srcPanel.KillSelection();
  }
  disableTimerProcessing1.Restore();
  disableTimerProcessing2.Restore();
  srcPanel.SetFocusToList();
}
void CApp::OnCopy(bool move, bool copyToSame, int srcPanelIndex)
{
	int destPanelIndex = (NumPanels <= 1) ? srcPanelIndex : (1 - srcPanelIndex);
	CPanel &srcPanel = Panels[srcPanelIndex];
	CPanel &destPanel = Panels[destPanelIndex];

	CPanel::CDisableTimerProcessing disableTimerProcessing1(destPanel);
	CPanel::CDisableTimerProcessing disableTimerProcessing2(srcPanel);

	if (!srcPanel.DoesItSupportOperations())
	{
		srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
		return;
	}

	CRecordVector<UInt32> indices;
	UString destPath;
	bool useDestPanel = false;

	{
		if (copyToSame)
		{
			int focusedItem = srcPanel._listView.GetFocusedItem();
			if (focusedItem < 0)
				return;
			int realIndex = srcPanel.GetRealItemIndex(focusedItem);
			if (realIndex == kParentIndex)
				return;
			indices.Add(realIndex);
			destPath = srcPanel.GetItemName(realIndex);
		}
		else
		{
			srcPanel.GetOperatedIndicesSmart(indices);
			if (indices.Size() == 0)
				return;
			destPath = destPanel._currentFolderPrefix;
			if (NumPanels == 1)
				ReducePathToRealFileSystemPath(destPath);
		}

		CCopyDialog copyDialog;
		UStringVector copyFolders;
		ReadCopyHistory(copyFolders);

		copyDialog.Strings = copyFolders;
		copyDialog.Value = destPath;

		copyDialog.Title = move ?
			LangString(IDS_MOVE, 0x03020202):
		LangString(IDS_COPY, 0x03020201);
		copyDialog.Static = move ?
			LangString(IDS_MOVE_TO, 0x03020204):
		LangString(IDS_COPY_TO, 0x03020203);

		copyDialog.Info = srcPanel.GetItemsInfoString(indices);

		if (copyDialog.Create(srcPanel.GetParent()) == IDCANCEL)
			return;

		destPath = copyDialog.Value;

		if (destPath.IsEmpty())
		{
			srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
			return;
		}

		if (!IsPathAbsolute(destPath))
		{
			if (!srcPanel.IsFSFolder())
			{
				srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
				return;
			}
			destPath = srcPanel._currentFolderPrefix + destPath;
		}

#ifndef UNDER_CE
		if (destPath.Length() > 0 && destPath[0] == '\\')
			if (destPath.Length() == 1 || destPath[1] != '\\')
			{
				srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
				return;
			}
#endif

			if (indices.Size() > 1 || (!destPath.IsEmpty() && destPath.Back() == WCHAR_PATH_SEPARATOR) ||
				NFind::DoesDirExist(destPath))
			{
				NDirectory::CreateComplexDirectory(destPath);
				NName::NormalizeDirPathPrefix(destPath);
				if (!CheckFolderPath(destPath))
				{
					if (NumPanels < 2 || destPath != destPanel._currentFolderPrefix || !destPanel.DoesItSupportOperations())
					{
						srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
						return;
					}
					useDestPanel = true;
				}
			}
			else
			{
				int pos = destPath.ReverseFind(WCHAR_PATH_SEPARATOR);
				if (pos >= 0)
				{
					UString prefix = destPath.Left(pos + 1);
					NDirectory::CreateComplexDirectory(prefix);
					if (!CheckFolderPath(prefix))
					{
						srcPanel.MessageBoxErrorLang(IDS_OPERATION_IS_NOT_SUPPORTED, 0x03020208);
						return;
					}
				}
			}

			AddUniqueStringToHeadOfList(copyFolders, destPath);
			while (copyFolders.Size() > 20)
				copyFolders.DeleteBack();
			SaveCopyHistory(copyFolders);
	}

	bool useSrcPanel = (!useDestPanel || !srcPanel.IsFsOrDrivesFolder() || destPanel.IsFSFolder());
	bool useTemp = useSrcPanel && useDestPanel;
	NFile::NDirectory::CTempDirectoryW tempDirectory;
	UString tempDirPrefix;
	if (useTemp)
	{
		tempDirectory.Create(kTempDirPrefix);
		tempDirPrefix = tempDirectory.GetPath();
		NFile::NName::NormalizeDirPathPrefix(tempDirPrefix);
	}

	CSelectedState srcSelState;
	CSelectedState destSelState;
	srcPanel.SaveSelectedState(srcSelState);
	destPanel.SaveSelectedState(destSelState);

	HRESULT result;
	if (useSrcPanel)
	{
		UString folder = useTemp ? tempDirPrefix : destPath;
		UInt32 s;
		for(int i=0;i<=indices.Size();i++)
		{ 
			s =indices.Back();}
		result = srcPanel.CopyTo(indices, folder, move, true, 0);

		if (result != S_OK)
		{
			disableTimerProcessing1.Restore();
			disableTimerProcessing2.Restore();
			// For Password:
			srcPanel.SetFocusToList();
			if (result != E_ABORT)
				srcPanel.MessageBoxError(result, L"Error");
			return;
		}
	}

	if (useDestPanel)
	{
		UStringVector filePaths;
		UString folderPrefix;
		if (useTemp)
			folderPrefix = tempDirPrefix;
		else
			folderPrefix = srcPanel._currentFolderPrefix;
		filePaths.Reserve(indices.Size());
		for (int i = 0; i < indices.Size(); i++)
			filePaths.Add(srcPanel.GetItemRelPath(indices[i]));

		result = destPanel.CopyFrom(folderPrefix, filePaths, true, 0);

		if (result != S_OK)
		{
			disableTimerProcessing1.Restore();
			disableTimerProcessing2.Restore();
			// For Password:
			srcPanel.SetFocusToList();
			if (result != E_ABORT)
				srcPanel.MessageBoxError(result, L"Error");
			return;
		}
	}

	RefreshTitleAlways();
	if (copyToSame || move)
	{
		srcPanel.RefreshListCtrl(srcSelState);
	}
	if (!copyToSame)
	{
		destPanel.RefreshListCtrl(destSelState);
		srcPanel.KillSelection();
	}
	disableTimerProcessing1.Restore();
	disableTimerProcessing2.Restore();
	srcPanel.SetFocusToList();
}
void CApp::OnSetSameFolder(int srcPanelIndex)
{
  if (NumPanels <= 1)
    return;
  const CPanel &srcPanel = Panels[srcPanelIndex];
  CPanel &destPanel = Panels[1 - srcPanelIndex];
  destPanel.BindToPathAndRefresh(srcPanel._currentFolderPrefix);
}

void CApp::OnSetSubFolder(int srcPanelIndex)
{
  if (NumPanels <= 1)
    return;
  const CPanel &srcPanel = Panels[srcPanelIndex];
  CPanel &destPanel = Panels[1 - srcPanelIndex];

  int focusedItem = srcPanel._listView.GetFocusedItem();
  if (focusedItem < 0)
    return;
  int realIndex = srcPanel.GetRealItemIndex(focusedItem);
  if (!srcPanel.IsItemFolder(realIndex))
    return;

  CMyComPtr<IFolderFolder> newFolder;
  if (realIndex == kParentIndex)
  {
    if (srcPanel._folder->BindToParentFolder(&newFolder) != S_OK)
      return;
  }
  else
  {
    if (srcPanel._folder->BindToFolder(realIndex, &newFolder) != S_OK)
      return;
  }
  destPanel.CloseOpenFolders();
  destPanel._folder = newFolder;
  destPanel.RefreshListCtrl();
}



static UString g_ToolTipBuffer;
static CSysString g_ToolTipBufferSys;

void CApp::OnNotify(int /* ctrlID */, LPNMHDR pnmh)
{
  {
    if (pnmh->code == TTN_GETDISPINFO)
    {
      LPNMTTDISPINFO info = (LPNMTTDISPINFO)pnmh;
      info->hinst = 0;
      g_ToolTipBuffer.Empty();
      SetButtonText((int)info->hdr.idFrom, g_ToolTipBuffer);
      g_ToolTipBufferSys = GetSystemString(g_ToolTipBuffer);
      info->lpszText = (LPTSTR)(LPCTSTR)g_ToolTipBufferSys;
      return;
    }
    #ifndef _UNICODE
    if (pnmh->code == TTN_GETDISPINFOW)
    {
      LPNMTTDISPINFOW info = (LPNMTTDISPINFOW)pnmh;
      info->hinst = 0;
      g_ToolTipBuffer.Empty();
      SetButtonText((int)info->hdr.idFrom, g_ToolTipBuffer);
      info->lpszText = (LPWSTR)(LPCWSTR)g_ToolTipBuffer;
      return;
    }
    #endif
  }
}

void CApp::RefreshTitle(bool always)
{
  UString path = GetFocusedPanel()._currentFolderPrefix;
  UString replacestring;//用以替换
  if(!path.IsEmpty())//路径不为空时获取文件名
  {
	 path.DeleteBack();
	 
	 while (path.Back() != WCHAR_PATH_SEPARATOR && !path.IsEmpty())
	 {
		replacestring +=path.Back();
		path.DeleteBack();
	 }
	 path.Empty();
	 int lengh = replacestring.Length();//记录字符长度
	 for (int i =0;i < lengh; i++)
	 {
		 path +=replacestring.Back();
		 replacestring.DeleteBack();
	 }
	 
  }
  if(path.IsEmpty())
	  return;
  path +=LangString(IDB_TITLE_LOGO,0x04080000);
  if (!always && path == PrevTitle)
    return;
 
  
  PrevTitle = path;
  NWindows::MySetWindowText(_window, path);
}

void CApp::RefreshTitle(int panelIndex, bool always)
{
  if (panelIndex != GetFocusedPanelIndex())
    return;
  RefreshTitle(always);
}
//WCHAR转化为TCHAR
extern bool WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize);

extern bool MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize);



//复制剪切文件
void CutOrCopyFiles(char *lpBuffer,UINT uBufLen,BOOL bCopy)
{
	UINT uDropEffect;
	DROPFILES dropFiles;
	UINT uGblLen,uDropFilesLen;
	HGLOBAL hGblFiles,hGblEffect;
	char *szData,*szFileList;
	DWORD *dwDropEffect;
	uDropEffect=RegisterClipboardFormat("Preferred DropEffect");
	hGblEffect=GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE,sizeof(DWORD));
	dwDropEffect=(DWORD*)GlobalLock(hGblEffect);
	if(bCopy)
		*dwDropEffect=DROPEFFECT_COPY;
	else 
		*dwDropEffect=DROPEFFECT_MOVE;
	GlobalUnlock(hGblEffect);

	uDropFilesLen=sizeof(DROPFILES);
	dropFiles.pFiles =uDropFilesLen;
	dropFiles.pt.x=0;
	dropFiles.pt.y=0;
	dropFiles.fNC =FALSE;
	dropFiles.fWide =TRUE;

	uGblLen=uDropFilesLen+uBufLen*2+8;
	hGblFiles= GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, uGblLen);
	szData=(char*)GlobalLock(hGblFiles);
	memcpy(szData,(LPVOID)(&dropFiles),uDropFilesLen);
	szFileList=szData+uDropFilesLen;

	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,
		lpBuffer,uBufLen,(WCHAR *)szFileList,uBufLen);

	GlobalUnlock(hGblFiles);
	if( OpenClipboard(NULL) )
	{
		EmptyClipboard();
		SetClipboardData( CF_HDROP, hGblFiles );
		CloseClipboard();
	}
}

void CApp::OnTest()
{
	int srcPanelIndex =GetFocusedPanelIndex();
	int destPanelIndex = (NumPanels <= 1) ? srcPanelIndex : (1 - srcPanelIndex);
	CPanel &srcPanel = Panels[srcPanelIndex];
	CPanel &destPanel = Panels[destPanelIndex];
	CPanel::CDisableTimerProcessing disableTimerProcessing1(destPanel);
	CPanel::CDisableTimerProcessing disableTimerProcessing2(srcPanel);
	CRecordVector<UInt32> indices;
	UString destPath;
	bool useDestPanel = false;
	srcPanel.GetOperatedIndicesSmart(indices);
	if (indices.Size() == 0)
		return;
	destPath = destPanel._currentFolderPrefix;
	UString ClipFileName;
	for (int i = 0; i < indices.Size(); i++)
	{
		int index = indices[i];

		ClipFileName+= destPath+srcPanel.GetItemRelPath(index)+L"\r\n";

	}

	TCHAR szBuffer[4096];

	memset(szBuffer,0,4096*sizeof(TCHAR));

	WCharToMByte(ClipFileName.GetBuffer(ClipFileName.Length()),szBuffer,sizeof(szBuffer)/sizeof(szBuffer[4096]));
	TCHAR *lp,*lpStart;
	TCHAR szFiles[4096],*lpFileStart;
	UINT uLen,uTotalLen=0;

	if(*szBuffer!=0) {
		//把文本筐中的文件名整理成要求的格式:"file1\0File2\0"
		memset(szFiles,0,4096*sizeof(TCHAR));
		lpFileStart=szFiles; //下个文件名存放位置指针
		lpStart=szBuffer;    //当前正在整理的文件名首地址
		lp=szBuffer;
		while(*lp!=0) {
			if(*lp=='\r') { //到达文件名末尾
				*lp=0;  //将回车符号换成0
				lstrcat(lpFileStart,lpStart);
				uLen=lstrlen(lpStart)+1;
				lpFileStart+=uLen;
				uTotalLen+=uLen;
				lp++;//跳过'\n'
				lpStart=lp+1;
			}
			lp++;
		}



		CutOrCopyFiles(szFiles,uTotalLen,TRUE);

	}

}



void CApp::SaveFileCopy()
{
	
	CRecordVector<UInt32> indices;
	GetFocusedPanel().GetOperatedItemIndices(indices);
	if(indices.Size() > 1)
		return;
	UString title =L"保存压缩文件副本";
	UString resultPath;
	UString currentPath=GetFocusedPanel().GetItemRelPath(indices[0]);
	UString filetype =L"所有文件";
	filetype +=L" (*.*)";
	if (MyBrowseSaveFilePath(g_HWND, title, currentPath,filetype, resultPath))
	{
		PressFileSave(GetFocusedPanelIndex(),resultPath);
	}

}
void CApp::PasswordSet()
{
	CPsWordDialog p_Word;
		p_Word.Create(g_HWND);
}
void CApp::CopyFiletoClipBoard()
{
	GetFocusedPanel().CopyFiles();
}
void CApp::MoveFromClipBoard()
{

	GetFocusedPanel().PlasterFiles();
}
void CApp::DeSelectRow()
{

	SelectSpec(false);
	RefreshStatusBar();
}
void CApp::SelectRow()
{
	SelectSpec(true);
	RefreshStatusBar();
}
void CApp::AddFile()
{
	GetFocusedPanel().AddToArchive();//添加文件到压缩文件中。
}
void CApp::ToFlord()
{
	GetFocusedPanel().ExtractArchives();//解压到指定文件夹。


}
void CApp::Test()
{
	GetFocusedPanel().TestArchives();//测试压缩的文件。

}


void CApp::SetFileName()
{
	Rename();

}
void CApp::PrintFile()
{
	GetFocusedPanel().PrintFile();
}

void CApp::AddAnnotation()
{
	GetFocusedPanel().ChangeComment();
}

//
//
//Tool
void CApp::Guide()
{
	CGuideDialog g_Guide;
	g_Guide.Create();

}

void CApp::FormChange()
{
	GetFocusedPanel().FromChange();
}
void CApp::RepairFile()
{
	::MessageBoxW(g_HWND,LangString(0x07000007),LangString(0x07000002),MB_ICONWARNING);
}
void CApp::UnPressItself()
{
	GetFocusedPanel().PressItself();
}

void CApp::GetInformation()
{
	Properties();
}
void CApp::GetReport()
{
	MessageBox(NULL,NULL,NULL,NULL);
}
void CApp::Favorites()
{
	UString fileParentpath;
	GetFocusedPanel().GetNowPath(fileParentpath);
	CFavoritesDialog _favorites;
	_favorites.GetFloderPath(fileParentpath);
	_favorites.Create(0);
}

void CApp::ArrangeFavorite()
{
	CFavClearUpDialog favClearUp;
	favClearUp.Create(g_HWND);
}


void CApp::Filein()
{
	UString title=L"加载 CoolRAR 设置自";
	UString fileName;
	WCHAR s[MAX_PATH]=L"*.reg\0*.reg\0所有文件\0*.*\0";
	UString resPath;
	if (MyBrowseForAppointFile(g_HWND,  title, fileName, s, resPath)) 
	{
		NWindows::NFile::NIO::CInFile infile;
		infile.Open(resPath);
		UString strItem=L"";
		::ShellExecuteW(0,L"open",resPath,strItem, NULL ,SW_SHOWNORMAL);
	}
}
void CApp::FileOut()
{
	UString title=L"保存 CoolRAR 设置为";
	UString fileName=L"Settings.reg";
	UString s = L"*.reg";
	UString resPath;
	if (MyGetSaveFilePath(g_HWND,title,fileName,s,resPath))
	{
		GetFocusedPanel().PanelInfoOut(resPath);
	}

}
void CApp::CheckDaily()
{
	MessageBoxW(g_HWND,LangString(0x07000003),LangString(0x07000004), MB_ICONINFORMATION);
}
void CApp::DeleteDaily()
{
	MessageBoxW(g_HWND,LangString(0x07000005),LangString(0x07000006),MB_YESNO|MB_ICONQUESTION);
}
void CApp::ManageTheme()
{
	CThemeDialog ThemeDlg;
	ThemeDlg.Create();
}
void CApp::MyDeleteFile()
{
	CRecordVector<UInt32> indices;
	GetFocusedPanel().GetOperatedItemIndices(indices);
	if(indices.Size() == 0)
	{
		::MessageBoxW(g_HWND,LangString(0x07000001),LangString(0x07000002),MB_ICONWARNING);
		return;
	}
	bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
	Delete(!shift);

	
}
bool CApp::ChangeToolBarState()
{
	bool isInPressfile = false;
	GetFocusedPanel()._deleteEnable = true;
	_toolBar.EnableButton(kAddCommand,true);
	_toolBar.EnableButton(IDM_ADDANNOTATION,true);
	_toolBar.EnableButton(IDM_DELETEFILE_LAGEBUTTON,true);
	if(GetFocusedPanel().NowPathJudge())
	{
		isInPressfile =true;
		_toolBar.EnableButton(IDM_ADDANNOTATION,false);
		if(GetFocusedPanel().NowAcrhvieJudge())
		{
			GetFocusedPanel()._deleteEnable = false;//禁用删除快捷按钮
			_toolBar.EnableButton(IDM_DELETEFILE_LAGEBUTTON,false);//RAR文件中删除按钮不可点击
			_toolBar.EnableButton(kAddCommand,false);//在压缩路径下添加按钮不可点击
		}
	}

	return isInPressfile;

}

void CApp::GetMD5()
{
	UString title=L"需要获取MD5信息的文件";
	UString fileName=L"";
	UString s=L"*.*";
	UString filepath;
	if (MyBrowseForFile(g_HWND,  title, fileName, s, filepath)) 
	{
		HttpUpdat ForMD5;
		ForMD5.GetMD5Info(filepath);
	}
}

void CApp::SetAdvancedValue()
{
	OptionsDlg odlg;
	CCodecs *codecs = new CCodecs;
	CMyComPtr<IUnknown> compressCodecsInfo = codecs;
	HRESULT result = codecs->Load();
	bool oneFile = false;
	UString name=L"*";
	NOptinsAddDlg::CInfo &di = odlg.Info;
	odlg.ArcFormats = &codecs->Formats;

	for (int i = 0; i < codecs->Formats.Size(); i++)
	{
		const CArcInfoEx &ai = codecs->Formats[i];
		if (ai.Name.CompareNoCase(L"swfc") == 0)
			if (!oneFile || name.Right(4).CompareNoCase(L".swf") != 0)
				continue;
		if (ai.UpdateEnabled && (oneFile || !ai.KeepName))
			odlg.ArcIndices.Add(i);
	}
	odlg.OptionsAddDialog(g_HWND, g_hInstance);
}

//通过文件完整路径删除文件。。。
#ifndef _UNICODE
typedef int (WINAPI * SHFileOperationWP)(LPSHFILEOPSTRUCTW lpFileOp);
#endif
void CApp::MyDeleteFile(const UString path,bool toRecycleBin,HWND parentHwnd,bool toAsk)
{
	CDynamicBuffer<WCHAR> buffer;
	size_t size = 0;
	int maxLen = 0;

	// L"\\\\?\\") doesn't work here.
	if (path.Length() > maxLen)
		maxLen = path.Length();
	buffer.EnsureCapacity(size + path.Length() + 1);
	memmove(((WCHAR *)buffer) + size, (const WCHAR *)path, (path.Length() + 1) * sizeof(WCHAR));
	size += path.Length() + 1;

	buffer.EnsureCapacity(size + 1);
	((WCHAR *)buffer)[size] = 0;
	if (maxLen >= MAX_PATH)
	{
		if (toRecycleBin)
		{
			return;
		}

	}
	else
	{
		SHFILEOPSTRUCTW fo;
		fo.hwnd = parentHwnd;
		fo.wFunc = FO_DELETE;
		fo.pFrom = (const WCHAR *)buffer;
		fo.pTo = 0;
		fo.fFlags = 0;
		if (toAsk)
			fo.fFlags |= FOF_ALLOWUNDO;
		else
			fo.fFlags |= FOF_NOCONFIRMATION;
		fo.fAnyOperationsAborted = FALSE;
		fo.hNameMappings = 0;
		fo.lpszProgressTitle = 0;
		int res;
#ifdef _UNICODE
		res = ::SHFileOperationW(&fo);
#else
		SHFileOperationWP shFileOperationW = (SHFileOperationWP)
			::GetProcAddress(::GetModuleHandleW(L"shell32.dll"), "SHFileOperationW");
		if (shFileOperationW == 0)
			return;
		res = shFileOperationW(&fo);
#endif
	}
}
int CApp::ThemeIsSelect()
{
	UStringVector ThemeFolderName;
	CThemeDialog Theme;
	UString ThemeSelect = Theme.ReadThemeReg();
	GetThemeFolderName(ThemeFolderName);
 	int i = ThemeFolderName.FindInSorted(ThemeSelect);
	if (i != -1)
	{
		return i+1;
	}
	else
	{
		return 0;
	}
}