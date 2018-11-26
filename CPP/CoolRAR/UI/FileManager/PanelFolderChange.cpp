// PanelFolderChange.cpp

#include "StdAfx.h"

#include "Common/StringConvert.h"
#include "Common/Wildcard.h"
#include "Windows/FileSystem.h"
#include "Common/DynamicBuffer.h"
#include "Windows/PropVariant.h"

#include "../../PropID.h"

#ifdef UNDER_CE
#include "FSFolder.h"
#else
#include "FSDrives.h"
#endif
#include "LangUtils.h"
#include "ListViewDialog.h"
#include "Panel.h"
#include "RootFolder.h"
#include "ViewSettings.h"
#include "BrowseDialog.h"
#include "../Common/CompressCall.h"
#include "resource.h"
#include "RootFolder.h"
#include "App.h"

using namespace NWindows;
using namespace NFile;
using namespace NFind;




CListViewDialog listViewDialogA;
static wchar_t *kTempDirPrefix = L"Coolrartemp";
static UString archivename;   //源压缩文件名
static UString archivepath;   //源压缩文件所在路径
static UString from;          //格式
static UString tempFilePath;  //临时文件所在的绝对路径
static UString tempDir;       //临时文件夹

void CPanel::SetToRootFolder()
{
  _folder.Release();
  _library.Free();
  CRootFolder *rootFolderSpec = new CRootFolder;
  _folder = rootFolderSpec;
  rootFolderSpec->Init();
}

HRESULT CPanel::ParseDesktopPath(UString & DesktopfilePath)
{
	for (int i=0;i<DesktopPathVector.Size();i++)
	{
		if(DesktopPathVector[i].FileName == DesktopfilePath)
		{
			DesktopfilePath =DesktopPathVector[i].FilePath;
			return S_OK;
		}
	}
	return S_FALSE;
}

bool WCharToTChar(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,NULL,0,NULL,FALSE);
	if(dwSize < dwMinSize)
	{
		return false;
	}
	WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,lpszStr,dwSize,NULL,FALSE);
	return true;
}


HRESULT CPanel::BindToPath(const UString &fullPath, bool &archiveIsOpened, bool &encrypted)
{
  UString  DesktopFilePath = fullPath;
  archiveIsOpened = false;
  encrypted = false;
  UString pathmemory;//记录当前路径用以打开文件失败时返回之前的路径
  pathmemory = _currentFolderPrefix;
  CDisableTimerProcessing disableTimerProcessing1(*this);

  if (_parentFolders.Size() > 0)
  {
    const UString &virtPath = _parentFolders.Back().VirtualPath;
    if (fullPath.Left(virtPath.Length()) == virtPath)
    {
      for (;;)
      {
        CMyComPtr<IFolderFolder> newFolder;
        HRESULT res = _folder->BindToParentFolder(&newFolder);
        if (!newFolder || res != S_OK)
          break;
        _folder = newFolder;
      }
      UStringVector parts;
      SplitPathToParts(fullPath.Mid(virtPath.Length()), parts);
      for (int i = 0; i < parts.Size(); i++)
      {
        const UString &s = parts[i];
        if ((i == 0 || i == parts.Size() - 1) && s.IsEmpty())
          continue;
        CMyComPtr<IFolderFolder> newFolder;
        HRESULT res = _folder->BindToFolder(s, &newFolder);
        if (!newFolder || res != S_OK)
          break;
        _folder = newFolder;
      }
      return S_OK;
    }
  }

  CloseOpenFolders();
  UString sysPath = fullPath;
  CFileInfoW fileInfo;
  UStringVector reducedParts;
  while (!sysPath.IsEmpty())
  {
    if (fileInfo.Find(sysPath))
      break;
    int pos = sysPath.ReverseFind(WCHAR_PATH_SEPARATOR);
    if (pos < 0)
      sysPath.Empty();
    else
    {
      if (reducedParts.Size() > 0 || pos < sysPath.Length() - 1)
        reducedParts.Add(sysPath.Mid(pos + 1));
      sysPath = sysPath.Left(pos);
    }
  }
  SetToRootFolder();
  CMyComPtr<IFolderFolder> newFolder;
  if (sysPath.IsEmpty())
  {
    if (_folder->BindToFolder(fullPath, &newFolder) == S_OK)
	{
		_folder = newFolder;
	}
	else if(ParseDesktopPath(DesktopFilePath)  == S_OK)
	{
		while (!DesktopFilePath.IsEmpty())
		{
			if (fileInfo.Find(DesktopFilePath))
				break;
			int pos = DesktopFilePath.ReverseFind(WCHAR_PATH_SEPARATOR);
			if (pos < 0)
				DesktopFilePath.Empty();
			else
			{
				if (reducedParts.Size() > 0 || pos < DesktopFilePath.Length() - 1)
					reducedParts.Add(sysPath.Mid(pos + 1));
				DesktopFilePath = DesktopFilePath.Left(pos);
			}
		}

	if (fileInfo.IsDir())
	{
		NName::NormalizeDirPathPrefix(DesktopFilePath);
		if (_folder->BindToFolder(DesktopFilePath, &newFolder) == S_OK)
			_folder = newFolder;
	}
	else
	{
		UString dirPrefix;
		if (!NDirectory::GetOnlyDirPrefix(DesktopFilePath, dirPrefix))
			dirPrefix.Empty();
		if (_folder->BindToFolder(dirPrefix, &newFolder) == S_OK)
		{
			_folder = newFolder;
			LoadFullPath();
			UString fileName;
			if (NDirectory::GetOnlyName(DesktopFilePath, fileName))
			{
				HRESULT res = OpenItemAsArchive(fileName, encrypted);
				if (res != S_FALSE)
				{
					RINOK(res);
				}
				
				if (res == S_OK)
				{
					archiveIsOpened = true;
					for (int i = reducedParts.Size() - 1; i >= 0; i--)
					{
						CMyComPtr<IFolderFolder> newFolder;
						_folder->BindToFolder(reducedParts[i], &newFolder);
						if (!newFolder)
							break;
						_folder = newFolder;
					}
				}
			}
		}
	}

	}
	else
	{
		if(pathmemory.IsEmpty())//如果初始化时上次打开的路径此次无法打开且没有最后一次成功打开的路径信息
		{
			return S_FALSE;//，返回错误改为打开桌面路径
		}
		TCHAR lfullpath[4096];
		memset(lfullpath,0,4096*sizeof(TCHAR));
		UString wfullpath;
		wfullpath =fullPath;
		wfullpath +=L"  无法访问";
		WCharToTChar(wfullpath.GetBuffer(wfullpath.Length()),lfullpath,sizeof(lfullpath)/sizeof(lfullpath[4096]));
		::MessageBox(HWND(*this),lfullpath,"CoolRAR",MB_ICONWARNING);
		BindToPath(pathmemory,archiveIsOpened,encrypted);//打开最后一次成功打开的路径信息
	}
  }
  else if (fileInfo.IsDir())
  {
    NName::NormalizeDirPathPrefix(sysPath);
    if (_folder->BindToFolder(sysPath, &newFolder) == S_OK)
      _folder = newFolder;
	
	else
	{
		TCHAR lfullpath[4096];
		memset(lfullpath,0,4096*sizeof(TCHAR));
		UString wfullpath;
		wfullpath =fullPath;
		wfullpath +=L"  无法访问";
		WCharToTChar(wfullpath.GetBuffer(wfullpath.Length()),lfullpath,sizeof(lfullpath)/sizeof(lfullpath[4096]));
		::MessageBox(HWND(*this),lfullpath,"CoolRAR",MB_ICONWARNING);
		if(pathmemory.IsEmpty())
		{
			BindToPathAndRefresh(L"桌面");	
		}
		else
		{
			BindToPathAndRefresh(pathmemory);
		}
	}
  }
  else
  {
    UString dirPrefix;
    if (!NDirectory::GetOnlyDirPrefix(sysPath, dirPrefix))
      dirPrefix.Empty();
    if (_folder->BindToFolder(dirPrefix, &newFolder) == S_OK)
    {
      _folder = newFolder;
      LoadFullPath();
      UString fileName;
      if (NDirectory::GetOnlyName(sysPath, fileName))
      {
        HRESULT res = OpenItemAsArchive(fileName, encrypted);
        if (res != S_FALSE)
        {
          RINOK(res);
        }
       
        if (res == S_OK)
        {
          archiveIsOpened = true;
          for (int i = reducedParts.Size() - 1; i >= 0; i--)
          {
            CMyComPtr<IFolderFolder> newFolder;
            _folder->BindToFolder(reducedParts[i], &newFolder);
            if (!newFolder)
              break;
            _folder = newFolder;
          }
        }
      }
    }
  }
   return S_OK;
}

HRESULT CPanel::BindToPathAndRefresh(const UString &path)
{
  CDisableTimerProcessing disableTimerProcessing1(*this);
  bool archiveIsOpened, encrypted;
  RINOK(BindToPath(path, archiveIsOpened, encrypted));
  RefreshListCtrl(UString(), -1, true, UStringVector());
  return S_OK;
}

void CPanel::GoToItem(UString name)
{
	CRecordVector<UInt32> indices;
	int count = _listView.GetItemCount();//获取当前路径下文件或文件夹总数
	for(int i =0; i < count; i++)
	{
		int index =GetRealItemIndex(i);
		UString realname;
		realname =GetItemName(index);
		if(realname == name)
		{
			_listView.SetItemState_FocusedSelected(i);

			return;
		}
		realname.Empty();
	}

}

void CPanel::SetBookmark(int index)
{
	_appState->FastFolders.SetString(index, _currentFolderPrefix);
}
void CPanel::SetBookmark(UString string)
{
	_appState->FastFolders.SetString(string);
}
void CPanel::SetBookmark(const UStringVector &setstring)
{
	_appState->FastFolders.SetString(setstring);
}
void CPanel::GetBookmark(UStringVector &getstring)
{
	_appState->FastFolders.GetString(getstring);
}

void CPanel::SetSign(UString string,UString sign)
{
	_appState->FastFolders.SetSign(string,sign);
}
void CPanel::SetSign(const UStringVector &setSign)
{
	_appState->FastFolders.SetSign(setSign);
}
void CPanel::GetSign(UStringVector &getSign)
{
	_appState->FastFolders.GetSign(getSign);
}

void CPanel::OpenBookmark(int index)
{
  BindToPathAndRefresh(_appState->FastFolders.GetString(index));
}

//主题
void CPanel::SetTheme(int index)
{
	_appState->ThemeTitle.SetString(index, _currentFolderPrefix);
}
void CPanel::SetTheme(const UStringVector &setstring)
{
	_appState->ThemeTitle.SetString(setstring);
}
void CPanel::SetTheme(const UString &string)
{
	_appState->ThemeTitle.SetString(string);
}
void CPanel::GetTheme(UStringVector &getstring)
{
	_appState->ThemeTitle.GetString(getstring);
}
void CPanel::OpenTheme(int index)
{
	BindToPathAndRefresh(_appState->ThemeTitle.GetString(index));
}
void CPanel::SetThemeFolderName(UString Name)
{
	_appState->ThemeTitle.SetFolderName(Name);
}
void CPanel::SetThemeFolderName(const UStringVector &FolderName)
{
	_appState->ThemeTitle.SetFolderName(FolderName);
}
void CPanel::GetThemeFolderName(UStringVector &getFolderName)
{
	_appState->ThemeTitle.GetFolderName(getFolderName);
}

UString GetFolderPath(IFolderFolder *folder)
{
  NCOM::CPropVariant prop;
  if (folder->GetFolderProperty(kpidPath, &prop) == S_OK)
    if (prop.vt == VT_BSTR)
      return (wchar_t *)prop.bstrVal;
  return UString();
}

void CPanel::LoadFullPath()
{
  _currentFolderPrefix.Empty();
  for (int i = 0; i < _parentFolders.Size(); i++)
  {
    const CFolderLink &folderLink = _parentFolders[i];
    _currentFolderPrefix += GetFolderPath(folderLink.ParentFolder);
    _currentFolderPrefix += folderLink.ItemName;
    _currentFolderPrefix += WCHAR_PATH_SEPARATOR;
  }
  if (_folder)
    _currentFolderPrefix += GetFolderPath(_folder);
  ShowTreeFile();
}

static int GetRealIconIndex(LPCWSTR path, DWORD attributes)
{
  int index = -1;
  if (GetRealIconIndex(path, attributes, index) != 0)
    return index;
  return -1;
}

 void CPanel::GetLastFileName(UString &filestring)
{
 	filestring = _currentFolderPrefix;
}
 bool CPanel::FileCopyJudge() 
 {

	 CRecordVector<UInt32> indices;
	 GetOperatedItemIndices(indices);
	 UString filename;
	 if(indices.Size() >1 || indices.Size()<1 )
	 {
		 
		 return false;
	 }
	 int focusedItem = _listView.GetFocusedItem();
	 if (focusedItem >= 0)
	 {
		 int realIndex = GetRealItemIndex(focusedItem);
		 filename =GetItemRelPath(realIndex);
	 }
    
	if(JudgeFileType(_currentFolderPrefix+filename))
	{
		return true;
	}
	return false;
 }
void CPanel::LoadFullPathAndShow()
{
  LoadFullPath();
   if(JudgeFileType(_currentFolderPrefix))
 {
	_appState->FolderHistory.AddString(_currentFolderPrefix);
 }
   UString currentFloderPrefix;
   currentFloderPrefix =_currentFolderPrefix;
   currentFloderPrefix.DeleteBack();
  _headerComboBox.SetText(currentFloderPrefix);

  #ifndef UNDER_CE
  COMBOBOXEXITEM item;
  item.mask = 0;

  UString path = _currentFolderPrefix;
  if (path.Length() >
      #ifdef _WIN32
      3
      #else
      1
      #endif
      && path[path.Length() - 1] == WCHAR_PATH_SEPARATOR)
    path.Delete(path.Length() - 1);

  CFileInfoW info;
  DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
  if (info.Find(path))
    attrib = info.Attrib;
  
  item.iImage = GetRealIconIndex(path, attrib);

  if (item.iImage >= 0)
  {
    item.iSelectedImage = item.iImage;
    item.mask |= (CBEIF_IMAGE | CBEIF_SELECTEDIMAGE);
  }
  item.iItem = -1;
  _headerComboBox.SetItem(&item);
  #endif

  RefreshTitle();
}

#ifndef UNDER_CE
LRESULT CPanel::OnNotifyComboBoxEnter(const UString &s)
{
  if (BindToPathAndRefresh(GetUnicodeString(s)) == S_OK)
  {
    PostMessage(kSetFocusToListView);
    return TRUE;
  }
  return FALSE;
}

bool CPanel::OnNotifyComboBoxEndEdit(PNMCBEENDEDITW info, LRESULT &result)
{
  if (info->iWhy == CBENF_ESCAPE)
  {
	UString currentFolderPrefix;
	currentFolderPrefix =_currentFolderPrefix;
	currentFolderPrefix.DeleteBack();
    _headerComboBox.SetText(currentFolderPrefix);
    PostMessage(kSetFocusToListView);
    result = FALSE;
    return true;
  }

  

  if (info->iWhy == CBENF_RETURN)
  {
    // When we use Edit control and press Enter.
    UString s;
    _headerComboBox.GetText(s);
	if(s.Back() != WCHAR_PATH_SEPARATOR)
	s += WCHAR_PATH_SEPARATOR;
    result = OnNotifyComboBoxEnter(s);
    return true;
  }
  return false;
}
#endif

#ifndef _UNICODE
bool CPanel::OnNotifyComboBoxEndEdit(PNMCBEENDEDIT info, LRESULT &result)
{
  if (info->iWhy == CBENF_ESCAPE)
  {
	  UString currentFloderPrefix;
	  currentFloderPrefix =_currentFolderPrefix;
	  currentFloderPrefix.DeleteBack();
    _headerComboBox.SetText(currentFloderPrefix);
    PostMessage(kSetFocusToListView);
    result = FALSE;
    return true;
  }
  

  if (info->iWhy == CBENF_RETURN)
  {
    UString s;
    _headerComboBox.GetText(s);
	if(s.Back() != WCHAR_PATH_SEPARATOR)
    s += WCHAR_PATH_SEPARATOR;
    result = OnNotifyComboBoxEnter(s);
    return true;
  }
  return false;
}
#endif

void CPanel::AddComboBoxItem(const UString &name, int iconIndex, int indent, bool addToList)
{
  #ifdef UNDER_CE

  UString s;
  iconIndex = iconIndex;
  for (int i = 0; i < indent; i++)
    s += L"  ";
  _headerComboBox.AddString(s + name);
  
  #else
  
  COMBOBOXEXITEMW item;
  item.mask = CBEIF_TEXT | CBEIF_INDENT;
  item.iSelectedImage = item.iImage = iconIndex;
  if (iconIndex >= 0)
    item.mask |= (CBEIF_IMAGE | CBEIF_SELECTEDIMAGE);
  item.iItem = -1;
  item.iIndent = indent;
  item.pszText = (LPWSTR)(LPCWSTR)name;
  _headerComboBox.InsertItem(&item);
  
  #endif

  if (addToList)
    ComboBoxPaths.Add(name);
}

extern UString RootFolder_GetName_Computer(int &iconIndex);
extern UString RootFolder_GetName_Network(int &iconIndex);
extern UString RootFolder_GetName_Documents(int &iconIndex);
extern UString RootFolder_GetName_Desktop(int &iconIndex);
extern UString GetMyDesktopPath();


bool CPanel::OnComboBoxCommand(UINT code, LPARAM /* param */, LRESULT &result)
{
  result = FALSE;
  switch(code)
  {
    case CBN_DROPDOWN:
    {
      ComboBoxPaths.Clear();
      _headerComboBox.ResetContent();
      
      int i;
      UStringVector pathParts;
      
      SplitPathToParts(_currentFolderPrefix, pathParts);
      UString sumPass;
      if (!pathParts.IsEmpty())
        pathParts.DeleteBack();

      #ifndef UNDER_CE

      int iconIndex;
      UString name;

	  //我的桌面
	  name = RootFolder_GetName_Desktop(iconIndex);
	 AddComboBoxItem(name, iconIndex, 0, true);
    
      name = RootFolder_GetName_Computer(iconIndex);
      AddComboBoxItem(name, iconIndex, 1, true);
        
	  UStringVector driveStrings;
	  MyGetLogicalDriveStrings(driveStrings);
	  for (i = 0; i < driveStrings.Size(); i++)
	  {
		  UString s = driveStrings[i];
		  
		  ComboBoxPaths.Add(s);
		  int iconIndex = GetRealIconIndex(s, 0);
		  if (s.Length() > 0 && s[s.Length() - 1] == WCHAR_PATH_SEPARATOR)
			  s.Delete(s.Length() - 1);

		  UString volumeName;
		  if (NFile::NSystem::MyGetDriveType(s) == DRIVE_FIXED)
		  {
			  UString fileSystemName;
			  if (volumeName.Length() == 0)
			  {
				  volumeName = L"本地磁盘";
			  }

		  }
		  else 
			  if(NFile::NSystem::MyGetDriveType(s) == DRIVE_CDROM )
			  {
				  UString fileSystemName;
				  if (volumeName.Length() == 0)
				  {
					  volumeName = L"DVD_RAM 驱动器";
				  }
			  }
			  else if (NFile::NSystem::MyGetDriveType(s) == DRIVE_REMOVABLE)
			  {
				  UString fileSystemName;
				  if (volumeName.Length() == 0)
				  {
					  volumeName = L"可移动磁盘";
				  }
			  }
			  else 
			  {
				  UString fileSystemName;
				  if (volumeName.Length() == 0)
				  {
					  volumeName = L"其它";
				  }
			  }


		 AddComboBoxItem(volumeName+L"("+s+L")", iconIndex, 2, false);

		  UString strNew=s.Left(2);
		  strNew.MakeLower();
		  UString strOld=pathParts[0];
		  strOld=strOld.Left(2);
		  strOld.MakeLower();
		  if(strOld == strNew)
		  {
			  for (int j = 1; j < pathParts.Size(); j++)
			  {
				  UString name = pathParts[j];
				  if ( j == 1)//如果是磁盘内的顶层文件夹则加入驱动器路径，其子文件夹则跳过直接加入文件夹名称
				  {
					  sumPass +=strNew;
					  sumPass +=WCHAR_PATH_SEPARATOR;
				  }
				  sumPass += name;
				  sumPass += WCHAR_PATH_SEPARATOR;
				  CFileInfoW info;
				  DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
				  if (info.Find(sumPass))
					  attrib = info.Attrib;
				  ComboBoxPaths.Add(sumPass);	
				  AddComboBoxItem(name.IsEmpty() ? L"\\" : name, GetRealIconIndex(sumPass, attrib), j+2, false);

			  }
		  }

	  }

	  name = RootFolder_GetName_Documents(iconIndex);
	  AddComboBoxItem(name, iconIndex, 1, true);

      name = RootFolder_GetName_Network(iconIndex);
      AddComboBoxItem(name, iconIndex, 1, true);

	  CFindFile filefilnd;
	  UStringVector  VectorName=filefilnd.FindDirFile(GetMyDesktopPath());
	  DesktopPathStruct DesktopPathStr;
	  {
		  for(int i=0;i<VectorName.Size(); i++)
		  {
			  DesktopPathStr.FileName=VectorName[i];
			DesktopPathStr.FilePath=GetMyDesktopPath()+VectorName[i]+L"\\";
			DesktopPathVector.Add(DesktopPathStr);

			  AddComboBoxItem(VectorName[i], GetRealIconIndex(VectorName[i], FILE_ATTRIBUTE_DIRECTORY), 1, true);
		  }
	  }

      #endif
    
      return false;
    }

    case CBN_SELENDOK:
    {
      code = code;
      int index = _headerComboBox.GetCurSel();
      if (index >= 0)
      {
        UString pass = ComboBoxPaths[index];
        _headerComboBox.SetCurSel(-1);
		if(pass.Find(L"\\",0) >=0)
		pass.DeleteBack();
        if (BindToPathAndRefresh(pass) == S_OK)
        {
          PostMessage(kSetFocusToListView);
          #ifdef UNDER_CE
          PostMessage(kRefreshHeaderComboBox);
          #endif

          return true;
        }
      }
      return false;
    }
  }
  return false;
}

bool CPanel::OnNotifyComboBox(LPNMHDR header, LRESULT &result)
{
  #ifndef UNDER_CE
  switch(header->code)
  {
    case CBEN_BEGINEDIT:
    {
      _lastFocusedIsList = false;
      _panelCallback->PanelWasFocused();
      break;
    }
    #ifndef _UNICODE
    case CBEN_ENDEDIT:
    {
      return OnNotifyComboBoxEndEdit((PNMCBEENDEDIT)header, result);
    }
    #endif
    case CBEN_ENDEDITW:
    {
      return OnNotifyComboBoxEndEdit((PNMCBEENDEDITW)header, result);
    }
  }
  #endif
  return false;
}

//获取浏览记录
void CPanel::GetOldFoldersHistory(int &index,UString &foldername)
{

	//判断当前获取的浏览记录条目数是否大于浏览记录总条目数量
	if(   index >= _appState->FolderHistory.GetSize()  ) 
	{
		return;
	}
	//如果第一获取则加载浏览记录条目
	if(index ==0)
	{
		listViewDialogA.DeleteIsAllowed = true;
		_appState->FolderHistory.GetList(listViewDialogA.Strings);
		
	}
	//获取浏览记录
	for(;;index++)
	{
		listViewDialogA.FocusedItemIndex = index;
		foldername = listViewDialogA.Strings[listViewDialogA.FocusedItemIndex];
		
				++index;//记录下一条应该读取的浏览记录
				return;//返回当前浏览的记录
	}
}
//打开所选的历史浏览条目
void CPanel::OpenHistoryFile(int index)
{	
	UString selectString;
	_appState->FolderHistory.AddString(listViewDialogA.Strings[index-1]);
	selectString = listViewDialogA.Strings[index-1];
    BindToPathAndRefresh(selectString);

}
void CPanel::OpenDirDriver(UString selectdrive)
{
	BindToPathAndRefresh(selectdrive);

}
void CPanel::FoldersHistory()
{
  CListViewDialog listViewDialog;
  listViewDialog.DeleteIsAllowed = true;
  listViewDialog.Title = LangString(IDS_FOLDERS_HISTORY, 0x03020260);
  _appState->FolderHistory.GetList(listViewDialog.Strings);
  if (listViewDialog.Create(GetParent()) == IDCANCEL)
    return;
  UString selectString;
  if (listViewDialog.StringsWereChanged)
  {
    _appState->FolderHistory.RemoveAll();
    for (int i = listViewDialog.Strings.Size() - 1; i >= 0; i--)
		if(JudgeFileType(listViewDialog.Strings[i]))
		{
          _appState->FolderHistory.AddString(listViewDialog.Strings[i]);
		}
    if (listViewDialog.FocusedItemIndex >= 0)
      selectString = listViewDialog.Strings[listViewDialog.FocusedItemIndex];
  }
  else
  {
    if (listViewDialog.FocusedItemIndex >= 0)
      selectString = listViewDialog.Strings[listViewDialog.FocusedItemIndex];
  }
  if (listViewDialog.FocusedItemIndex >= 0)
    BindToPathAndRefresh(selectString);
}



//转换压缩格式
void CPanel::FromChange()
{
	UString archivepath,archivename,form;
	if (_parentFolders.Size() > 0)//在压缩文件内
	{
		archivepath = _parentFolders.Front().FolderPath;
		archivename = _parentFolders.Front().ItemName;
		FromChangeRealDo(archivepath, L"7z", archivename);
		return;
	}
	CRecordVector<UInt32> indices;
	GetOperatedItemIndices(indices);
	if (indices.Size() == 0)
		:: MessageBoxW(g_HWND,LangString(0x07000045),L"CoolRAR",MB_ICONERROR);
	for (int i = 0; i< indices.Size(); i++ )
	{
		archivepath = _currentFolderPrefix;
		archivename = GetItemRelPath(indices[i]);
		if(JudgeFileType(_currentFolderPrefix + archivename))//是否是压缩文件
			FromChangeRealDo(archivepath, L"7z", archivename);
		else if (indices.Size() == 1)						 //如果只选中了一个文件且不是压缩文件则弹出提示			
			:: MessageBoxW(g_HWND,LangString(0x07000045),L"CoolRAR",MB_ICONERROR);
	}
}
//将压缩文件转换为自解压格式
void CPanel::PressItself()
{
	CRecordVector<UInt32> indices;
	GetOperatedItemIndices(indices);
	UStringVector _fileNames;
	UINT32 index;
	
	if (_parentFolders.Size() > 0)//如果在压缩文件内部
	{
		UString realname,currentpath;
		realname =_parentFolders.Back().ItemName;
		currentpath =_parentFolders.Back().FolderPath;
		_fileNames.Add(currentpath+realname);
		UString archivePathPrefix;
		FromChangeRealDo(currentpath,L"exe",realname);
		return;
	}
	index =indices[0];
	UString realname;//文件名以及文件类型
	realname =GetItemName(index);
	if( !JudgeFileType(_currentFolderPrefix+realname+L"\\") )//选定的文件是否为压缩文件
	{
		::MessageBoxW(HWND(*this),LangString(0x07000045),LangString(0x07000012),MB_ICONWARNING);
		return;
	}
	_fileNames.Add(_currentFolderPrefix+realname);
	UString archivePathPrefix;
	FromChangeRealDo(_currentFolderPrefix,L"exe",realname);
}
void CPanel::OpenParentFolder()
{
  LoadFullPath(); // Maybe we don't need it ??
  UString focucedName;
  if (!_currentFolderPrefix.IsEmpty() &&
      _currentFolderPrefix.Back() == WCHAR_PATH_SEPARATOR)
  {
    focucedName = _currentFolderPrefix;
    focucedName.DeleteBack();
    if (focucedName != L"\\\\.")
    {
      int pos = focucedName.ReverseFind(WCHAR_PATH_SEPARATOR);
      if (pos >= 0)
        focucedName = focucedName.Mid(pos + 1);
    }
  }

  CDisableTimerProcessing disableTimerProcessing1(*this);
  CMyComPtr<IFolderFolder> newFolder;
  _folder->BindToParentFolder(&newFolder);
  if (newFolder)
    _folder = newFolder;
  else
  {
    if (!_parentFolders.IsEmpty())
    {
      _folder.Release();
      _library.Free();
      CFolderLink &link = _parentFolders.Back();
      _folder = link.ParentFolder;
      _library.Attach(link.Library.Detach());
      focucedName = link.ItemName;
      if (_parentFolders.Size() > 1)
        OpenParentArchiveFolder();
      _parentFolders.DeleteBack();
      if (_parentFolders.IsEmpty())
        _flatMode = _flatModeForDisk;
    }
  }

  UStringVector selectedItems;

  LoadFullPath();
  RefreshListCtrl(focucedName, -1, true, selectedItems);
  _listView.EnsureVisible(_listView.GetFocusedItem(), false);
  RefreshStatusBar();
}

void CPanel::CloseOpenFolders()
{
  while (_parentFolders.Size() > 0)
  {
    _folder.Release();
    _library.Free();
    _folder = _parentFolders.Back().ParentFolder;
    _library.Attach(_parentFolders.Back().Library.Detach());
    if (_parentFolders.Size() > 1)
      OpenParentArchiveFolder();
    _parentFolders.DeleteBack();
  }
  _flatMode = _flatModeForDisk;
  _folder.Release();
  _library.Free();
}

void CPanel::OpenRootFolder()
{
  CDisableTimerProcessing disableTimerProcessing1(*this);
  _parentFolders.Clear();
  SetToRootFolder();
  RefreshListCtrl(UString(), -1, true, UStringVector());
  
}

void CPanel::OpenDrivesFolder()
{
  CloseOpenFolders();
  #ifdef UNDER_CE
  NFsFolder::CFSFolder *folderSpec = new NFsFolder::CFSFolder;
  _folder = folderSpec;
  folderSpec->InitToRoot();
  #else
  CFSDrives *folderSpec = new CFSDrives;
  _folder = folderSpec;
  folderSpec->Init();
  #endif
  RefreshListCtrl();
}


void CPanel::ShowTreeFile()
{

	UString name;
	UString LastName = _currentFolderPrefix;
	HTREEITEM root;
	int i=LastName.Find(L"\\");
	while(i!= -1)
	{
		name = name+LastName.Left(i+1);
		LastName = LastName.Right(LastName.Length() - i-1);
		root = GetTreeItemHt(name);
		if(root != NULL)
		{
			HTREEITEM childroot=_treeView.GetChildItem(root);
			if (childroot == NULL)
			{
				AddTree(name,root,false,true);
			}
			changTreeSize = true;
		_treeView.Expand(root);
		
		}
		i=LastName.Find(L"\\");
	}
}

HTREEITEM CPanel::GetTreeItemHt(UString FilePath)
{

	TreePathStruct treePathStruct;
	int sign = FilePath.Find(L":");//将驱动器盘符转化为大写供树形列表判定
	if(sign < 0)
		return NULL;
	UString replacestring = FilePath;
	FilePath.MakeUpper();
	while(FilePath.Length() > sign)	
	{
		FilePath.DeleteBack();
	}
	replacestring.Delete(0,1);
	FilePath +=replacestring;
	if (FilePath != NULL)
	{
		for(int i=0 ;i<treePathVector.Size();i++)
		{
			treePathStruct=treePathVector[i];
			if(treePathStruct.FilePath == FilePath)return treePathStruct.Root;
		}
	}

	return NULL;

}


void CPanel::OpenFolder(int index)
{
  if (index == kParentIndex)
  {
    OpenParentFolder();
    return;
  }
  CMyComPtr<IFolderFolder> newFolder;
  _folder->BindToFolder(index, &newFolder);
  if (!newFolder)
    return;
  _folder = newFolder;
  LoadFullPath();

  RefreshListCtrl();
  UINT state = LVIS_SELECTED;
  _listView.SetItemState(_listView.GetFocusedItem(), state, state);
  _listView.EnsureVisible(_listView.GetFocusedItem(), false);
}
void CPanel::OpenRealeaseFile()
{
	//打开压缩文件函数
	UString fileName=L"";
	UString title = L"查找压缩文件";
	UString s = L"所有文件";
	HWND hwnd = FindWindow("FM",NULL);//获取主对话框句柄
	UString resPath;
	if (!MyBrowseForFile(hwnd,  title, fileName, s, resPath))
		return;
	BindToPathAndRefresh(resPath);

}


void CPanel::GetNowPath(UString &nowPath)
{
	nowPath =_currentFolderPrefix;
}



#ifndef _UNICODE
typedef int (WINAPI * SHFileOperationWP)(LPSHFILEOPSTRUCTW lpFileOp);
#endif
static void MyDeleteFile(const UString path,bool toRecycleBin,HWND parentHwnd,bool toAsk)
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

extern UString GetMyDocumentsPath();
static THREAD_FUNC_DECL CoderThread(void *p)
{
	UStringVector names;
	names.Add(tempFilePath);
	UString tempFilePathKeep = tempFilePath;//保存当前文件的临时解压路径，防止多个文件同时转换压缩格式时临时解压路径被篡改
	if (from == L"7z")//转为7Z
	{
		archivename +=L".7z";
		CompressFiles(archivepath, archivename, L"7z", names, false, false, true);
	}

	else             //转为自解压
	{
		archivename +=L".exe";
	    CompressItself(archivepath, archivename, L"7z", names, false, false, true);
	}

	MyDeleteFile(tempFilePathKeep,false,NULL,false);
	return 0;

}
extern CApp g_App;
void CPanel::FromChangeRealDo(UString karchivepath,UString kfrom,UString karchivename)
{
	archivepath = karchivepath;
	from        = kfrom;
	archivename = karchivename;
	tempFilePath.Empty();
	tempDir.Empty();
	UString keepname = archivename;//记录源压缩文件名

	int index = archivename.ReverseFind('.');
	UString ext = archivename.Mid(index + 1);
	ext.MakeLower();
	if (ext == from)                //如果文件格式与用户指定格式相同则直接返回
		return;

	index = archivename.Length() - index;
	for (int i = 0; i < index ; i++)  //出去文件名中的后缀名
	{
		archivename.DeleteBack();
	}
	UString temppath = GetMyDocumentsPath() + L"Local Settings" +WCHAR_PATH_SEPARATOR + L"Temp"+WCHAR_PATH_SEPARATOR;
	tempDir = temppath + kTempDirPrefix;               //解压到的路径
	UString tempDirNorm = tempDir;
	NFile::NName::NormalizeDirPathPrefix(tempDirNorm);//创建临时目录
	tempFilePath = tempDirNorm + archivename;		  //解压出来的文件的绝对路径
	if(_parentFolders.Size() > 0)                     //如果在压缩文件内
		g_App.OnCopyDirectly(false,false,tempFilePath + WCHAR_PATH_SEPARATOR,0,NExtract::NOverwriteMode::kWithoutPrompt);
	else
	{
		UStringVector paths;
		paths.Add(archivepath + keepname);
		::ExtractArchives(paths,tempFilePath, false,true);
	}

	NWindows::CThread  FromChangeTread;//创建线程进行转换处理
	FromChangeTread.Create(CoderThread, this);
	FromChangeTread.Detach();

}