#include "StdAfx.h"

#include "Common/StringConvert.h"

#include "Windows/COM.h"
#include "Windows/Clipboard.h"
#include "Windows/Menu.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

#include "../../PropID.h"
#include "../Common/PropIDUtils.h"
#include "../Explorer/ContextMenu.h"

#include "App.h"
#include "LangUtils.h"
#include "MyLoadMenu.h"
#include "PluginInterface.h"
#include "PropertyName.h"

#include "resource.h"
#include "PropertyNameRes.h"
#include "InforDialog.h"


using namespace NWindows;



LONG g_DllRefCount = 0;

static const UINT kSevenZipStartMenuID = kPluginMenuStartID ;
static const UINT kSystemStartMenuID = kPluginMenuStartID + 100;

void CPanel::InvokeSystemCommand(const char *command)
{
  NCOM::CComInitializer comInitializer;
  if (!IsFsOrPureDrivesFolder())
    return;
  CRecordVector<UInt32> operatedIndices;
  GetOperatedItemIndices(operatedIndices);
  if (operatedIndices.IsEmpty())
  {
	::MessageBoxW(NULL,LangString(0x07000001),LangString(0x07000002),MB_ICONWARNING);
    return;
  }
  CMyComPtr<IContextMenu> contextMenu;
  if (CreateShellContextMenu(operatedIndices, contextMenu) != S_OK)
    return;

  CMINVOKECOMMANDINFO ci;
  ZeroMemory(&ci, sizeof(ci));
  ci.cbSize = sizeof(CMINVOKECOMMANDINFO);
  ci.hwnd = GetParent();
  ci.lpVerb = command;
  contextMenu->InvokeCommand(&ci);
}

static const wchar_t *kSeparator = L"----------------------------\n";
static const wchar_t *kPropValueSeparator = L": ";

extern UString ConvertSizeToString(UInt64 value);

static void AddPropertyString(PROPID propID, const wchar_t *nameBSTR,
							  const NCOM::CPropVariant &prop, UString &s,UString&t)
{
	if (prop.vt != VT_EMPTY)
	{
		UString val;

		if ((prop.vt == VT_UI8 || prop.vt == VT_UI4) && (
			propID == kpidSize ||
			propID == kpidPackSize ||
			propID == kpidNumSubDirs ||
			propID == kpidNumSubFiles ||
			propID == kpidNumBlocks ||
			propID == kpidClusterSize ||
			propID == kpidTotalSize ||
			propID == kpidFreeSpace ||
			propID == kpidPhySize ||
			propID == kpidHeadersSize ||
			propID == kpidFreeSpace
			))
			val = ConvertSizeToString(ConvertPropVariantToUInt64(prop));
		else
			val = ConvertPropertyToString(prop, propID);

		if (!val.IsEmpty())
		{
			t = GetNameOfProperty(propID, nameBSTR);
			s = val;
		}
	}
}

void CPanel::Properties()
{
	CRecordVector<UInt32> indices;
	GetOperatedItemIndices(indices);
	if(indices.Size() != 1)
	{
		::MessageBoxW(HWND(*this),LangString(0x07000045),LangString(0x07000012),MB_ICONINFORMATION);
		return;
	}
	
	
	CMyComPtr<IGetFolderArchiveProperties> getFolderArchiveProperties;
	_folder.QueryInterface(IID_IGetFolderArchiveProperties, &getFolderArchiveProperties);
	{
		UString message;
		UString itemname;
		UStringVector messageVector;
		UStringVector itemnameVector;
		int count =0;
	
		CRecordVector<UInt32> operatedIndices;
		GetOperatedItemIndices(operatedIndices);
		if (operatedIndices.Size() == 1)
		{
			UInt32 index = operatedIndices[0];
			UInt32 numProps;
			if (_folder->GetNumberOfProperties(&numProps) == S_OK)
			{
				for (UInt32 i = 0; i < numProps; i++)
				{
					CMyComBSTR name;
					PROPID propID;
					VARTYPE varType;

					if (_folder->GetPropertyInfo(i, &name, &propID, &varType) != S_OK)
						continue;

					NCOM::CPropVariant prop;
					if (_folder->GetProperty(index, propID, &prop) != S_OK)
						continue;
					AddPropertyString(propID, name, prop, message,itemname);		
					messageVector.Add(message);
					itemnameVector.Add(itemname);
					count ++;
					message.Empty();
					itemname.Empty();
				}
			}
		}
		
		//开始获得文件类型
		itemname = LangString(IDS_PROP_FILE_TYPE, 0x02000214);//项目名称
		UString         fullpath = _currentFolderPrefix+messageVector.Front();
		SHFILEINFOW		sfi;
		SHGetFileInfoW(fullpath,0,&sfi,sizeof(SHFILEINFO),SHGFI_SMALLICON|SHGFI_ICON|SHGFI_TYPENAME    );

		message =sfi.szTypeName;//获得文件类型
		messageVector.Add(message);
		itemnameVector.Add(itemname);
		message.Empty();
		itemname.Empty();
		count ++;

		{
			NCOM::CPropVariant prop;
			if (_folder->GetFolderProperty(kpidPath, &prop) == S_OK)
			{
				AddPropertyString(kpidName, L"Path", prop, message,itemname);
				messageVector.Add(message);
				itemnameVector.Add(itemname);
				count ++;
				message.Empty();
				itemname.Empty();
			}
		}

		CMyComPtr<IFolderProperties> folderProperties;
		_folder.QueryInterface(IID_IFolderProperties, &folderProperties);
		if (folderProperties)
		{
			UInt32 numProps;
			if (folderProperties->GetNumberOfFolderProperties(&numProps) == S_OK)
			{
				for (UInt32 i = 0; i < numProps; i++)
				{
					CMyComBSTR name;
					PROPID propID;
					VARTYPE vt;
					if (folderProperties->GetFolderPropertyInfo(i, &name, &propID, &vt) != S_OK)
						continue;
					NCOM::CPropVariant prop;
					if (_folder->GetFolderProperty(propID, &prop) != S_OK)
						continue;
					AddPropertyString(propID, name, prop, message,itemname);
					messageVector.Add(message);
					itemnameVector.Add(itemname);
					count ++;
					message.Empty();
					itemname.Empty();
				}
			}
		}

		CMyComPtr<IGetFolderArchiveProperties> getFolderArchiveProperties;
		_folder.QueryInterface(IID_IGetFolderArchiveProperties, &getFolderArchiveProperties);
		if (getFolderArchiveProperties)
		{
			CMyComPtr<IFolderArchiveProperties> getProps;
			getFolderArchiveProperties->GetFolderArchiveProperties(&getProps);
			if (getProps)
			{
				UInt32 numProps;
				if (getProps->GetNumberOfArchiveProperties(&numProps) == S_OK)
				{
					if (numProps > 0)
						message += kSeparator;
					for (UInt32 i = 0; i < numProps; i++)
					{
						CMyComBSTR name;
						PROPID propID;
						VARTYPE vt;
						if (getProps->GetArchivePropertyInfo(i, &name, &propID, &vt) != S_OK)
							continue;
						NCOM::CPropVariant prop;
						if (getProps->GetArchiveProperty(propID, &prop) != S_OK)
							continue;
						AddPropertyString(propID, name, prop, message,itemname);
						messageVector.Add(message);
						itemnameVector.Add(itemname);
						count ++;
						message.Empty();
						itemname.Empty();
					}
				}
			}
		}
		CInforDialog information;
		information.GetInformation(count ,messageVector,itemnameVector);
		DWORD b=GetLastError();
		information.Create();


	}
}


void CPanel::EditCopy()
{
  UString s;
  CRecordVector<UInt32> indices;
  GetSelectedItemsIndices(indices);
  for (int i = 0; i < indices.Size(); i++)
  {
    if (i > 0)
      s += L"\xD\n";
    s += GetItemName(indices[i]);
  }
  ClipboardSetText(_mainWindow, s);
}


HRESULT CPanel::CreateShellContextMenu(
    const CRecordVector<UInt32> &operatedIndices,
    CMyComPtr<IContextMenu> &systemContextMenu)
{
  systemContextMenu.Release();
  UString folderPath = GetFsPath();

  CMyComPtr<IShellFolder> desktopFolder;
  RINOK(::SHGetDesktopFolder(&desktopFolder));
  if (!desktopFolder)
  {
    return E_FAIL;
  }
  
  // Separate the file from the folder.

  
  // Get a pidl for the folder the file
  // is located in.
  LPITEMIDLIST parentPidl;
  DWORD eaten;
  RINOK(desktopFolder->ParseDisplayName(
      GetParent(), 0, (wchar_t *)(const wchar_t *)folderPath,
      &eaten, &parentPidl, 0));
  
  // Get an IShellFolder for the folder
  // the file is located in.
  CMyComPtr<IShellFolder> parentFolder;
  RINOK(desktopFolder->BindToObject(parentPidl,
      0, IID_IShellFolder, (void**)&parentFolder));
  if (!parentFolder)
  {
    return E_FAIL;
  }
  
  // Get a pidl for the file itself.
  CRecordVector<LPITEMIDLIST> pidls;
  pidls.Reserve(operatedIndices.Size());
  for (int i = 0; i < operatedIndices.Size(); i++)
  {
    LPITEMIDLIST pidl;
    UString fileName = GetItemRelPath(operatedIndices[i]);
    if (IsFSDrivesFolder())
      fileName += WCHAR_PATH_SEPARATOR;
    RINOK(parentFolder->ParseDisplayName(GetParent(), 0,
      (wchar_t *)(const wchar_t *)fileName, &eaten, &pidl, 0));
    pidls.Add(pidl);
  }

  ITEMIDLIST temp;
  if (pidls.Size() == 0)
  {
    temp.mkid.cb = 0;

    pidls.Add(&temp);
  }

  // Get the IContextMenu for the file.
  CMyComPtr<IContextMenu> cm;
  RINOK( parentFolder->GetUIObjectOf(GetParent(), pidls.Size(),
      (LPCITEMIDLIST *)&pidls.Front(), IID_IContextMenu, 0, (void**)&cm));
  if (!cm)
  {
    return E_FAIL;
  }
  systemContextMenu = cm;
  return S_OK;
}

void CPanel::CreateSystemMenu(HMENU menuSpec,
    const CRecordVector<UInt32> &operatedIndices,
    CMyComPtr<IContextMenu> &systemContextMenu)
{
  systemContextMenu.Release();

  CreateShellContextMenu(operatedIndices, systemContextMenu);

  if (systemContextMenu == 0)
    return;
  
  // Set up a CMINVOKECOMMANDINFO structure.
  CMINVOKECOMMANDINFO ci;
  ZeroMemory(&ci, sizeof(ci));
  ci.cbSize = sizeof(CMINVOKECOMMANDINFO);
  ci.hwnd = GetParent();
 
  {

    CMenu popupMenu;
 
    if(!popupMenu.CreatePopup())
      throw 210503;

    HMENU hMenu = popupMenu;

    DWORD Flags = CMF_EXPLORE;
  
    systemContextMenu->QueryContextMenu(hMenu, 0, kSystemStartMenuID, 0x7FFF, Flags);
    

    {
      CMenu menu;
      menu.Attach(menuSpec);
      CMenuItem menuItem;
      menuItem.fMask = MIIM_SUBMENU | MIIM_TYPE | MIIM_ID;
      menuItem.fType = MFT_STRING;
      menuItem.hSubMenu = popupMenu.Detach();
      menuItem.StringValue = LangString(IDS_SYSTEM, 0x030202A0);
      menu.InsertItem(0, true, menuItem);
    }
   
  }
}

void CPanel::CreateFileMenu(HMENU menuSpec)
{
  CreateFileMenu(menuSpec, _sevenZipContextMenu, _systemContextMenu, true);
		
}

void CPanel::CreateSevenZipMenu(HMENU menuSpec,
    const CRecordVector<UInt32> &operatedIndices,
    CMyComPtr<IContextMenu> &sevenZipContextMenu)
{
  sevenZipContextMenu.Release();

  CMenu menu;
  menu.Attach(menuSpec);
 
  bool sevenZipMenuCreated = false;

  CMyComPtr<IContextMenu> contextMenu;
  contextMenu = new CZipContextMenu;
  {
    CMyComPtr<IInitContextMenu> initContextMenu;
    if (contextMenu.QueryInterface(IID_IInitContextMenu, &initContextMenu) != S_OK)
      return;
    UString currentFolderUnicode = _currentFolderPrefix;
    UStringVector names;
    int i;
    for(i = 0; i < operatedIndices.Size(); i++)
      names.Add(currentFolderUnicode + GetItemRelPath(operatedIndices[i]));
    CRecordVector<const wchar_t *> namePointers;
    for(i = 0; i < operatedIndices.Size(); i++)
      namePointers.Add(names[i]);
    
   
    if (initContextMenu->InitContextMenu(currentFolderUnicode, &namePointers.Front(),
        operatedIndices.Size()) == S_OK)
    {
      HRESULT res = contextMenu->QueryContextMenu(menu, 0, kSevenZipStartMenuID,
          kSystemStartMenuID - 1, 0);
      sevenZipMenuCreated = (HRESULT_SEVERITY(res) == SEVERITY_SUCCESS);
      if (sevenZipMenuCreated)
        sevenZipContextMenu = contextMenu;
      
    }
  }
}

void CPanel::CreateFileMenu(HMENU menuSpec,
    CMyComPtr<IContextMenu> &sevenZipContextMenu,
    CMyComPtr<IContextMenu> &systemContextMenu,
    bool programMenu)
{
  sevenZipContextMenu.Release();
  systemContextMenu.Release();

  CRecordVector<UInt32> operatedIndices;
  GetOperatedItemIndices(operatedIndices);

  CMenu menu;
  menu.Attach(menuSpec);
  
 

  int i;
  for (i = 0; i < operatedIndices.Size(); i++)
    if (IsItemFolder(operatedIndices[i]))
      break;
  bool allAreFiles = (i == operatedIndices.Size());
  LoadFileMenu(menu, menu.GetItemCount(), programMenu,
      IsFSFolder(), operatedIndices.Size(), allAreFiles);
}

bool CPanel::InvokePluginCommand(int id)
{
  return InvokePluginCommand(id, _sevenZipContextMenu, _systemContextMenu);
}

bool CPanel::InvokePluginCommand(int id,
    IContextMenu *sevenZipContextMenu, IContextMenu *systemContextMenu)
{
  UInt32 offset;
  bool isSystemMenu = (id >= kSystemStartMenuID);
  if (isSystemMenu)
    offset = id  - kSystemStartMenuID;
  else
    offset = id  - kSevenZipStartMenuID;

  #ifdef UNDER_CE
  CMINVOKECOMMANDINFO
  #else
  CMINVOKECOMMANDINFOEX
  #endif
    commandInfo;
  commandInfo.cbSize = sizeof(commandInfo);
  commandInfo.fMask = 0
  #ifndef UNDER_CE
  | CMIC_MASK_UNICODE
  #endif
  ;
  commandInfo.hwnd = GetParent();
  commandInfo.lpVerb = (LPCSTR)(MAKEINTRESOURCE(offset));
  commandInfo.lpParameters = NULL;
  CSysString currentFolderSys = GetSystemString(_currentFolderPrefix);
  commandInfo.lpDirectory = (LPCSTR)(LPCTSTR)(currentFolderSys);
  commandInfo.nShow = SW_SHOW;
  commandInfo.lpParameters = NULL;
  #ifndef UNDER_CE
  commandInfo.lpTitle = "";
  commandInfo.lpVerbW = (LPCWSTR)(MAKEINTRESOURCEW(offset));
  UString currentFolderUnicode = _currentFolderPrefix;
  commandInfo.lpDirectoryW = currentFolderUnicode;
  commandInfo.lpTitleW = L"";
  commandInfo.ptInvoke.x = 0;
  commandInfo.ptInvoke.y = 0;
  #endif
  HRESULT result;
  if (isSystemMenu)
    result = systemContextMenu->InvokeCommand(LPCMINVOKECOMMANDINFO(&commandInfo));
  else
    result = sevenZipContextMenu->InvokeCommand(LPCMINVOKECOMMANDINFO(&commandInfo));
  if (result == NOERROR)
  {
    KillSelection();
    return true;
  }
  return false;
}

bool CPanel::OnContextMenu(HANDLE windowHandle, int xPos, int yPos)
{
  if (::GetParent((HWND)windowHandle) == _listView)
  {
    ShowColumnsContextMenu(xPos, yPos);
    return true;
  }

  if (windowHandle != _listView)
    return false;
  

  CRecordVector<UInt32> operatedIndices;
  GetOperatedItemIndices(operatedIndices);

  if (xPos < 0 || yPos < 0)
  {
    if (operatedIndices.Size() == 0)
    {
      xPos = 0;
      yPos = 0;
    }
    else
    {
      int itemIndex = _listView.GetNextItem(-1, LVNI_FOCUSED);
      if (itemIndex == -1)
        return false;
      RECT rect;
      if (!_listView.GetItemRect(itemIndex, &rect, LVIR_ICON))
        return false;
      xPos = (rect.left + rect.right) / 2;
      yPos = (rect.top + rect.bottom) / 2;
    }
    POINT point = {xPos, yPos};
    _listView.ClientToScreen(&point);
    xPos = point.x;
    yPos = point.y;
  }
  
  CMenu menu;
  CMenuDestroyer menuDestroyer(menu);
  menu.CreatePopup();

  CMyComPtr<IContextMenu> sevenZipContextMenu;
  CMyComPtr<IContextMenu> systemContextMenu;
  CreateFileMenu(menu, sevenZipContextMenu, systemContextMenu, false);

  int result = menu.Track(TPM_LEFTALIGN
      #ifndef UNDER_CE
      | TPM_RIGHTBUTTON
      #endif
      | TPM_RETURNCMD | TPM_NONOTIFY,
    xPos, yPos, _listView);

  if (result == 0)
    return true;

  if (result >= kPluginMenuStartID)
  {
    InvokePluginCommand(result, sevenZipContextMenu, systemContextMenu);
    return true;
  }
  if (ContextMenuCommand(result))
    return true;
  return true;
}

bool CPanel::CreatMenuByStatusBar(int xPos, int yPos)
{



	POINT point = {xPos, yPos};
	_statusBar.ClientToScreen(&point);
	xPos = point.x;
	yPos = point.y;


	CMenu menu;
	CMenuDestroyer menuDestroyer(menu);
	menu.CreatePopup();

	CMyComPtr<IContextMenu> sevenZipContextMenu;
	CMyComPtr<IContextMenu> systemContextMenu;
	LoadDriveMenu(menu);
	int result = menu.Track(TPM_LEFTALIGN
#ifndef UNDER_CE
		| TPM_RIGHTBUTTON
#endif
		| TPM_RETURNCMD | TPM_NONOTIFY,
		xPos, yPos, _statusBar);

	if (result == 0)
		return true;

	if (result >= kPluginMenuStartID)
	{
		InvokePluginCommand(result, sevenZipContextMenu, systemContextMenu);
		return true;
	}
	if (ContextMenuCommand(result))
		return true;
	return true;
}
