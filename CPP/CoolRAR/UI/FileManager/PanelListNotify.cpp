// PanelListNotify.cpp

#include "StdAfx.h"

#include "resource.h"

#include "Common/IntToString.h"
#include "Common/StringConvert.h"

#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

#include "../Common/PropIDUtils.h"
#include "../../PropID.h"
#include "LangUtils.h"
#include "Panel.h"
#include "FormatUtils.h"
#include "ProgramLocation.h"
#include "ThemeDialog.h"

using namespace NWindows;
extern HINSTANCE g_hInstance;


UString ConvertSizeToString(UInt64 value)
{
  wchar_t s[32];
  ConvertUInt64ToString(value, s);
  int i = MyStringLen(s);
  int pos = sizeof(s) / sizeof(s[0]);
  s[--pos] = L'\0';
  while (i > 3)
  {
    s[--pos] = s[--i];
    s[--pos] = s[--i];
    s[--pos] = s[--i];
    s[--pos] = L',';
  }
  while (i > 0)
    s[--pos] = s[--i];
  return s + pos;
}


LRESULT CPanel::SetItemText(LVITEMW &item)
{
  if (_dontShowMode)
    return 0;

  UINT32 realIndex = GetRealIndex(item);
 

  if ((item.mask & LVIF_TEXT) == 0)
    return 0;

  if (realIndex == kParentIndex)
    return 0;
  UString s;
  UINT32 subItemIndex = item.iSubItem;
  PROPID propID = _visibleProperties[subItemIndex].ID;
 

  NCOM::CPropVariant prop;
 

  if (_folder->GetProperty(realIndex, propID, &prop) != S_OK)
      throw 2723407;
	
  if ((prop.vt == VT_UI8 || prop.vt == VT_UI4) && (
      propID == kpidSize ||
      propID == kpidPackSize ||
      propID == kpidNumSubDirs ||
      propID == kpidNumSubFiles ||
      propID == kpidPosition ||
      propID == kpidNumBlocks ||
      propID == kpidClusterSize ||
      propID == kpidTotalSize ||
      propID == kpidFreeSpace
      ))
    s = ConvertSizeToString(ConvertPropVariantToUInt64(prop));
  else if(propID ==kpidAttrib)
  {
	  CMyComBSTR name;
	  VARTYPE varType;
	  _folder->GetPropertyInfo(0, &name, &propID, &varType);
		

	  NCOM::CPropVariant prop;
	  _folder->GetProperty(realIndex, propID, &prop);//初始化PROP获得文件完整名称
		 
	  UString         fullpath = _currentFolderPrefix+prop.bstrVal ;
      SHFILEINFOW		sfi;
	  SHGetFileInfoW(fullpath,0,&sfi,sizeof(SHFILEINFO),SHGFI_SMALLICON|SHGFI_ICON|SHGFI_TYPENAME     );
	  s =sfi.szTypeName;//获得文件类型

  }
  else
  {
    s = ConvertPropertyToString(prop, propID, false);
    s.Replace(wchar_t(0xA), L' ');
    s.Replace(wchar_t(0xD), L' ');
  }
  int size = item.cchTextMax;
  if (size > 0)
  {
    if (s.Length() + 1 > size)
      s = s.Left(size - 1);
    MyStringCopy(item.pszText, (const wchar_t *)s);
  }
  return 0;
}

#ifndef UNDER_CE
extern DWORD g_ComCtl32Version;
#endif

void CPanel::OnItemChanged(NMLISTVIEW *item)
{
  int index = (int)item->lParam;
  if (index == kParentIndex)
    return;
  bool oldSelected = (item->uOldState & LVIS_SELECTED) != 0;
  bool newSelected = (item->uNewState & LVIS_SELECTED) != 0;
  // Don't change this code. It works only with such check
  if (oldSelected != newSelected)
    _selectedStatusVector[index] = newSelected;
}

extern bool g_LVN_ITEMACTIVATE_Support;

void CPanel::OnNotifyActivateItems()
{
  bool alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
  bool ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
  bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
  if (!shift && alt && !ctrl)
    Properties();
  else
    OpenSelectedItems(!shift || alt || ctrl);
}

bool CPanel::OnNotifyList(LPNMHDR header, LRESULT &result)
{
  switch(header->code)
  {
    case LVN_ITEMCHANGED:
    {
      if (_enableItemChangeNotify)
      {
        if (!_mySelectMode)
          OnItemChanged((LPNMLISTVIEW)header);
        RefreshStatusBar();
      }
      return false;
    }
   

    case LVN_GETDISPINFOW:
    {
      LV_DISPINFOW *dispInfo = (LV_DISPINFOW *)header;

      //is the sub-item information being requested?

      if ((dispInfo->item.mask & LVIF_TEXT) != 0 ||
        (dispInfo->item.mask & LVIF_IMAGE) != 0)
        SetItemText(dispInfo->item);
      return false;
    }
    case LVN_KEYDOWN:
    {
      bool boolResult = OnKeyDown(LPNMLVKEYDOWN(header), result);
      RefreshStatusBar();
      return boolResult;
    }

    case LVN_COLUMNCLICK:
      OnColumnClick(LPNMLISTVIEW(header));
      return false;

    case LVN_ITEMACTIVATE:
      if (g_LVN_ITEMACTIVATE_Support)
      {
        OnNotifyActivateItems();
        return false;
      }
      break;
    case NM_DBLCLK:
    case NM_RETURN:
      if (!g_LVN_ITEMACTIVATE_Support)
      {
        OnNotifyActivateItems();
        return false;
      }
      break;

    case NM_RCLICK:
      RefreshStatusBar();
      break;

   
    case NM_CLICK:
    {
      // we need SetFocusToList, if we drag-select items from other panel.
      SetFocusToList();
      RefreshStatusBar();
      if (_mySelectMode)
        #ifndef UNDER_CE
        if (g_ComCtl32Version >= MAKELONG(71, 4))
        #endif
          OnLeftClick((MY_NMLISTVIEW_NMITEMACTIVATE *)header);
      return false;
    }
    case LVN_BEGINLABELEDITW:
      result = OnBeginLabelEdit((LV_DISPINFOW *)header);
      return true;
    case LVN_ENDLABELEDITW:
      result = OnEndLabelEdit((LV_DISPINFOW *)header);
      return true;

    case NM_CUSTOMDRAW:
    {
      if (_mySelectMode)
        return OnCustomDraw((LPNMLVCUSTOMDRAW)header, result);
      break;
    }
    case LVN_BEGINDRAG:
    {
      OnDrag((LPNMLISTVIEW)header);
      RefreshStatusBar();
      break;
    }
  }
  return false;
}

bool CPanel::OnCustomDraw(LPNMLVCUSTOMDRAW lplvcd, LRESULT &result)
{
  switch(lplvcd->nmcd.dwDrawStage)
  {
  case CDDS_PREPAINT :
    result = CDRF_NOTIFYITEMDRAW;
    return true;
    
  case CDDS_ITEMPREPAINT:
   
    int realIndex = (int)lplvcd->nmcd.lItemlParam;
    bool selected = false;
    if (realIndex != kParentIndex)
      selected = _selectedStatusVector[realIndex];
    if (selected)
      lplvcd->clrTextBk = RGB(255, 192, 192);
    else
      lplvcd->clrTextBk = _listView.GetBkColor();
    result = CDRF_NOTIFYITEMDRAW;
    return true;

        /* At this point, you can change the background colors for the item
        and any subitems and return CDRF_NEWFONT. If the list-view control
        is in report mode, you can simply return CDRF_NOTIFYSUBITEMREDRAW
        to customize the item's subitems individually */
  }
  return false;
}

static int GetRealIconIndex(LPCWSTR path, DWORD attributes)
{
	int index = -1;
	if (GetRealIconIndex(path, attributes, index) != 0)
		return index;
	return -1;
}

void CPanel::OnRefreshStatusBar()


{
	CRecordVector<UINT32> indices;
	GetOperatedItemIndices(indices);
	UInt64 filescount=0;
	UInt64 foldercount=0;
	UString filescountstring;
	UString foldercountstring;
	UString filesName;
	UString s_fstring;
	UINT64 filesnumber=0;
	UInt64 fileSize=0;
	UInt32 foldernumber=0;
	UString foldernumberString;
	UString f_fname;
	UString fileSizestring;
	UString filenumberstring;
	UString selectfile;
	UINT selectfiles=0;
	UINT selectfolder=0;
	UINT64 selectfilesize=0;
	UString ffname;
	UString selectfolderstring;
	UString selectfilestring;
	UString selectfilesizestring;


	//设置驱动器和密码两个状态栏图标
	CThemeDialog themeTitle;
	UString ThemeValue;
	ThemeValue = themeTitle.ReadThemeReg();
	HICON iconDrive,iconCode;
	UString iconpath;
	if (ThemeValue != L"")
	{
		UString tempPath;
		tempPath = themeTitle.GetAppDataPath()+L"\\CoolRAR\\Themes\\";
		iconpath=tempPath+WSTRING_PATH_SEPARATOR+ThemeValue;

		iconDrive = (HICON)LoadImageW(g_hInstance,iconpath+L"\\drive.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		iconCode = (HICON)LoadImageW(g_hInstance,iconpath+L"\\key.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
	}
	else
	{
		GetProgramFolderPath(iconpath);
		iconDrive = (HICON)LoadImageW(g_hInstance,iconpath+L"icon\\drive.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		iconCode = (HICON)LoadImageW(g_hInstance,iconpath+L"icon\\key.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
	}

	HIMAGELIST listImageList =GetSysImageList(true);
	UStringVector driveStrings;

	int driveiconindex = GetRealIconIndex(L"C:\\",0);
	_statusBar.SetText(0,L"");
	_statusBar.SendMessageW(SB_SETICON,(WPARAM)0,(LPARAM)iconDrive);
	_statusBar.SendMessageW(SB_SETICON,(WPARAM)1,(LPARAM)iconCode);


	UString selectSizeString;
	if(indices.Size()>0){
		for(int i=0;i<indices.Size();i++){
			NCOM::CPropVariant prop1;
			_folder->GetProperty(indices[i], kpidIsDir, &prop1);
			if(prop1.boolVal){
				selectfolder++;

			}
			else{
				// 
				selectfiles++;
				selectfilesize+=GetItemSize(indices[i]);
			}
		}
	}
	//ffname=
	selectfilesizestring=ConvertSizeToString(selectfilesize);
	selectfilestring=ConvertSizeToString(selectfiles);
	selectfolderstring=ConvertSizeToString(selectfolder);
	if(selectfolder!=0&&selectfiles==0){
		ffname=LangString(0x04000450);
		ffname+=selectfolderstring;
		ffname+=LangString(0x04000451);



	}
	else if(selectfolder==0&&selectfiles!=0){
		ffname=LangString(0x04000450);
		ffname+=selectfilesizestring;
		ffname+=LangString(0x04000452);
		ffname+=selectfilestring;
		ffname+=LangString(0x04000453);

	}else{
		ffname=LangString(0x04000450);
		ffname+=selectfolderstring;
		ffname+=LangString(0x04000454);
		ffname+=selectfilesizestring;
		ffname+=LangString(0x04000452);
		ffname+=selectfilestring;
		ffname+=LangString(0x04000453);

	}
	int focusedItem = _listView.GetFocusedItem();
	UString sizeString;
	UString dateString;
	if (focusedItem >= 0 && _listView.GetSelectedCount() > 0)
	{
		int realIndex = GetRealItemIndex(focusedItem);
		if (realIndex != kParentIndex)
		{ 

			sizeString = ConvertSizeToString(GetItemSize(realIndex));
			NCOM::CPropVariant prop;
			if (_folder->GetProperty(realIndex, kpidMTime, &prop) == S_OK)
				dateString = ConvertPropertyToString(prop, kpidMTime, false);
		}
	}
	for(UINT i=0 ;i<(UINT)_selectedStatusVector.Size(); i++)
	{
		NCOM::CPropVariant Isdir;
		_folder->GetProperty(i, kpidIsDir, &Isdir);
		if( Isdir.boolVal ){
			foldernumber++;
		}


		else{
			filesnumber++;
			fileSize+=GetItemSize(i);

		}


	}
	filenumberstring=ConvertSizeToString(filesnumber);
	fileSizestring=ConvertSizeToString(fileSize);
	foldernumberString=ConvertSizeToString(foldernumber);
	f_fname=LangString(0x04000455);
	f_fname+=foldernumberString;
	f_fname+=LangString(0x04000451);
	f_fname+=LangString(0x04000456);
	f_fname+=fileSizestring;
	f_fname+=LangString(0x04000452);
	f_fname+=filenumberstring;
	f_fname+=LangString(0x04000453);
	
	_statusBar.SetText(2, ffname);

	_statusBar.SetText(3, f_fname);
	
}

void CPanel::UpdateStausIco(UString ImagePath,int Fouse)
{
	HICON iconDrive,iconCode;
	UString iconpath;
	if (Fouse > 0)
	{
		iconpath=ImagePath;
		iconDrive = (HICON)LoadImageW(g_hInstance,iconpath+L"\\drive.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		iconCode = (HICON)LoadImageW(g_hInstance,iconpath+L"\\key.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
	}
	else
	{
		GetProgramFolderPath(ImagePath);
		iconpath=ImagePath;
		iconDrive = (HICON)LoadImageW(g_hInstance,iconpath+L"icon\\drive.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		iconCode = (HICON)LoadImageW(g_hInstance,iconpath+L"icon\\key.ico",
			IMAGE_ICON,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
	}

	HIMAGELIST listImageList =GetSysImageList(true);
	UStringVector driveStrings;

	int driveiconindex = GetRealIconIndex(L"C:\\",0);
	_statusBar.SetText(0,L"");
	_statusBar.SendMessageW(SB_SETICON,(WPARAM)0,(LPARAM)iconDrive);
	_statusBar.SendMessageW(SB_SETICON,(WPARAM)1,(LPARAM)iconCode);

}