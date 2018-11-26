// PanelItems.cpp

#include "StdAfx.h"

#include "../../../../C/Sort.h"

#include "Common/StringConvert.h"
#include <Windowsx.h>
#include "Windows/Menu.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

#include "../../PropID.h"

#include "resource.h"

#include "LangUtils.h"
#include "Panel.h"
#include "PropertyName.h"
#include "RootFolder.h"

#include "FindDialog.h"
#include "FindProcessDialog.h"

#include "SdDialog.h"
#include "Windows/Process.h"
#include "ViewSettings.h"
#include "App.h"

using namespace NWindows;

static int GetColumnAlign(PROPID propID, VARTYPE varType)
{
  switch(propID)
  {
    case kpidCTime:
    case kpidATime:
    case kpidMTime:
      return LVCFMT_LEFT;
  }
  switch(varType)
  {
    case VT_UI1:
    case VT_I2:
    case VT_UI2:
    case VT_I4:
    case VT_INT:
    case VT_UI4:
    case VT_UINT:
    case VT_I8:
    case VT_UI8:
    case VT_BOOL:
      return LVCFMT_RIGHT;
    
    case VT_EMPTY:
    case VT_I1:
    case VT_FILETIME:
    case VT_BSTR:
      return LVCFMT_LEFT;
    
    default:
      return LVCFMT_CENTER;
  }
}

void CPanel::InitColumns()
{
	if (_needSaveInfo)
	SaveListViewInfo();
	
  _listView.DeleteAllItems();
  _selectedStatusVector.Clear();

  ReadListViewInfo();


  PROPID sortID;
  sortID  = _listViewInfo.SortID;

  _ascending = _listViewInfo.Ascending;

  _properties.Clear();

  _needSaveInfo = true;

  UInt32 numProperties;
  _folder->GetNumberOfProperties(&numProperties);
  int i;
  for (i = 0; i < (int)numProperties; i++)
  {
    CMyComBSTR name;
    PROPID propID;
    VARTYPE varType;

    if (_folder->GetPropertyInfo(i, &name, &propID, &varType) != S_OK)
      throw 1;

    if (propID == kpidIsDir)
      continue;

    CItemProperty prop;
    prop.Type = varType;
    prop.ID = propID;
    prop.Name = GetNameOfProperty(propID, name);
    prop.Order = -1;
    prop.IsVisible = true;
    prop.Width = 100;
    _properties.Add(prop);
  }
	
  for (;;)
    if (!_listView.DeleteColumn(0))
      break;

  int order = 0;
  for(i = 0; i < _listViewInfo.Columns.Size(); i++)
  {
    const CColumnInfo &columnInfo = _listViewInfo.Columns[i];
    int index = _properties.FindItemWithID(columnInfo.PropID);
    if (index >= 0)
    {
      CItemProperty &item = _properties[index];
      item.IsVisible = columnInfo.IsVisible;
      item.Width = columnInfo.Width;
      if (columnInfo.IsVisible)
        item.Order = order++;
      continue;
    }
  }
  for(i = 0; i < _properties.Size(); i++)
  {
    CItemProperty &item = _properties[i];
    if (item.Order < 0)
      item.Order = order++;
  }

  _visibleProperties.Clear();
	if( _listViewInfo.Columns.Size() == 0)//第一次启动FM时设置默认LISTVIEW列数
	{
		for(int j =0; j < _properties.Size(); j++ )
		{
			_properties[j].IsVisible = false;
		}
		if (_properties.Size() >= 1)
		{
			_properties[0].IsVisible =true;
			_visibleProperties.Add(_properties[0]);//名称
		}
		if (_properties.Size() >= 2)
		{
			_properties[1].IsVisible =true;
			_visibleProperties.Add(_properties[1]);//大小
		}
		if (_properties.Size() >= 5)
		{
			_properties[5].IsVisible =true;
			_visibleProperties.Add(_properties[5]);//属性
		}
		if (_properties.Size() >= 3)
		{
			_properties[2].IsVisible =true;
			_visibleProperties.Add(_properties[2]);//修改时间
		}
		
		
	}
	else
	{
		for (i = 0; i < _properties.Size(); i++)
		{
			const CItemProperty &prop = _properties[i];
			if (prop.IsVisible)
				_visibleProperties.Add(prop);
		}
	}
  _sortID = kpidName;

  _sortID = _listViewInfo.SortID;

  for (i = 0; i < _visibleProperties.Size(); i++)
  {
    InsertColumn(i);
  }
   
}

void CPanel::InsertColumn(int index)
{
  const CItemProperty &prop = _visibleProperties[index];
  LV_COLUMNW column;
  if(prop.ID != _sortID || _upArrangeIconIndex == -1
	                    || _downArrangeIconIndex == -1)//假若当前加入的列类型不同与排列类型或图片资源加载失败则不显示箭头
  {
	  column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
	  column.cx = prop.Width;
	  column.fmt = GetColumnAlign(prop.ID, prop.Type);
	  column.iOrder = prop.Order;
	  column.iSubItem = index;
	  column.pszText = (wchar_t *)(const wchar_t *)prop.Name;
	  _listView.InsertColumn(index, &column);
	  return;
  }
  else
  {
	  column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER | LVCF_IMAGE;
	  column.cx = prop.Width;
	  column.fmt = GetColumnAlign(prop.ID, prop.Type);
	  column.iOrder = prop.Order;
	  column.iSubItem = index;
	  if( _ascending )//根据升降序选择ICO
	  {
		  column.iImage =_upArrangeIconIndex;
	  }  
	  else
	  {
		  column.iImage =_downArrangeIconIndex;
	  }
	  column.pszText = (wchar_t *)(const wchar_t *)prop.Name;
	  _listView.InsertColumn(index, &column);
  }
 
}

void CPanel::RefreshListCtrl()
{
  RefreshListCtrl(UString(), -1, true, UStringVector());
}

int CALLBACK CompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM lpData);


void CPanel::GetSelectedNames(UStringVector &selectedNames)
{
  selectedNames.Clear();

  CRecordVector<UInt32> indices;
  GetSelectedItemsIndices(indices);
  selectedNames.Reserve(indices.Size());
  for (int  i = 0; i < indices.Size(); i++)
    selectedNames.Add(GetItemRelPath(indices[i]));

 
  selectedNames.Sort();
}

void CPanel::SaveSelectedState(CSelectedState &s)
{
  s.FocusedName.Empty();
  s.SelectedNames.Clear();
  s.FocusedItem = _listView.GetFocusedItem();
  {
    if (s.FocusedItem >= 0)
    {
      int realIndex = GetRealItemIndex(s.FocusedItem);
      if (realIndex != kParentIndex)
        s.FocusedName = GetItemRelPath(realIndex);
    }
  }
  GetSelectedNames(s.SelectedNames);
}

void CPanel::RefreshListCtrl(const CSelectedState &s)
{
  bool selectFocused = s.SelectFocused;
  if (_mySelectMode)
    selectFocused = true;
  RefreshListCtrl(s.FocusedName, s.FocusedItem, selectFocused, s.SelectedNames);
}

void CPanel::RefreshListCtrlSaveFocused()
{
  CSelectedState state;
  SaveSelectedState(state);
  RefreshListCtrl(state);
}

void CPanel::SetFocusedSelectedItem(int index, bool select)
{
  UINT state = LVIS_FOCUSED;
  if (select)
    state |= LVIS_SELECTED;
  _listView.SetItemState(index, state, state);
  if (!_mySelectMode && select)
  {
    int realIndex = GetRealItemIndex(index);
    if (realIndex != kParentIndex)
      _selectedStatusVector[realIndex] = true;
  }
}
extern CApp g_App;
void CPanel::RefreshListCtrl(const UString &focusedName, int focusedPos, bool selectFocused,
    const UStringVector &selectedNames)
{
  _dontShowMode = false;
  LoadFullPathAndShow();
  CDisableTimerProcessing timerProcessing(*this);

  if (focusedPos < 0)
    focusedPos = 0;

  _listView.SetRedraw(false);

  LVITEMW item;
  ZeroMemory(&item, sizeof(item));
  
  _listView.DeleteAllItems();
  _selectedStatusVector.Clear();
  _startGroupSelect = 0;

  _selectionIsDefined = false;
  

  if (!_folder)
  {
    // throw 1;
    SetToRootFolder();
  }
  
  _headerToolBar.EnableButton(kParentFolderID, !IsRootFolder());
  _isInThePressFile = g_App.ChangeToolBarState();

  CMyComPtr<IFolderSetFlatMode> folderSetFlatMode;
  _folder.QueryInterface(IID_IFolderSetFlatMode, &folderSetFlatMode);
  if (folderSetFlatMode)
    folderSetFlatMode->SetFlatMode(BoolToInt(_flatMode));

  if (_folder->LoadItems() != S_OK)
    return;

  InitColumns();


  UInt32 numItems;
  _folder->GetNumberOfItems(&numItems);

  bool showDots = _showDots && !IsFSDrivesFolder();

  _listView.SetItemCount(numItems + (showDots ? 1 : 0));

  _selectedStatusVector.Reserve(numItems);
  int cursorIndex = -1;

  CMyComPtr<IFolderGetSystemIconIndex> folderGetSystemIconIndex;
  if (!IsFSFolder() || _showRealFileIcons)
    _folder.QueryInterface(IID_IFolderGetSystemIconIndex, &folderGetSystemIconIndex);

  if (showDots)
  {
    UString itemName = L"..";
    item.iItem = _listView.GetItemCount();
    if (itemName.CompareNoCase(focusedName) == 0)
      cursorIndex = item.iItem;
    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    int subItem = 0;
    item.iSubItem = subItem++;
    item.lParam = kParentIndex;
    item.pszText = (wchar_t *)(const wchar_t *)itemName;
    UInt32 attrib = FILE_ATTRIBUTE_DIRECTORY;
    item.iImage = _extToIconMap.GetIconIndex(attrib, itemName);
    if (item.iImage < 0)
      item.iImage = 0;
    if(_listView.InsertItem(&item) == -1)
      return;
  }
  

  for(UInt32 i = 0; i < numItems; i++)
  {
    UString itemName = GetItemName(i);
    const UString relPath = GetItemRelPath(i);
    if (relPath.CompareNoCase(focusedName) == 0)
      cursorIndex = _listView.GetItemCount();
    bool selected = false;
    if (selectedNames.FindInSorted(relPath) >= 0)
      selected = true;
    _selectedStatusVector.Add(selected);

    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

    if (!_mySelectMode)
      if (selected)
      {
        item.mask |= LVIF_STATE;
        item.state = LVIS_SELECTED;
      }
  
    int subItem = 0;
    item.iItem = _listView.GetItemCount();
    
    item.iSubItem = subItem++;
    item.lParam = i;
    
    UString correctedName;
    if (itemName.Find(L"     ") >= 0)
    {
      int pos = 0;
      for (;;)
      {
        int posNew = itemName.Find(L"     ", pos);
        if (posNew < 0)
        {
          correctedName += itemName.Mid(pos);
          break;
        }
        correctedName += itemName.Mid(pos, posNew - pos);
        correctedName += L" ... ";
        pos = posNew;
        while (itemName[++pos] == ' ');
      }
      item.pszText = (wchar_t *)(const wchar_t *)correctedName;
    }
    else
      item.pszText = (wchar_t *)(const wchar_t *)itemName;

    NCOM::CPropVariant prop;
    _folder->GetProperty(i, kpidAttrib, &prop);
    UInt32 attrib = 0;
    if (prop.vt == VT_UI4)
      attrib = prop.ulVal;
    else if (IsItemFolder(i))
      attrib |= FILE_ATTRIBUTE_DIRECTORY;

    bool defined  = false;

    if (folderGetSystemIconIndex)
    {
      folderGetSystemIconIndex->GetSystemIconIndex(i, &item.iImage);
      defined = (item.iImage > 0);
    }
    if (!defined)
    {
      if (_currentFolderPrefix.IsEmpty())
      {
        int iconIndexTemp;
        GetRealIconIndex(itemName + WCHAR_PATH_SEPARATOR, attrib, iconIndexTemp);
        item.iImage = iconIndexTemp;
      }
      else
      {
        item.iImage = _extToIconMap.GetIconIndex(attrib, itemName);
      }
    }
    if (item.iImage < 0)
      item.iImage = 0;

    if(_listView.InsertItem(&item) == -1)
      return; // error
  }
  

  if(_listView.GetItemCount() > 0 && cursorIndex >= 0)
    SetFocusedSelectedItem(cursorIndex, selectFocused);
  _listView.SortItems(CompareItems, (LPARAM)this);
  if (cursorIndex < 0 && _listView.GetItemCount() > 0)
  {
    if (focusedPos >= _listView.GetItemCount())
      focusedPos = _listView.GetItemCount() - 1;
    SetFocusedSelectedItem(focusedPos, showDots);
  }
  _listView.EnsureVisible(_listView.GetFocusedItem(), false);
  _listView.SetRedraw(true);
  _listView.InvalidateRect(NULL, true);

}

void CPanel::GetSelectedItemsIndices(CRecordVector<UInt32> &indices) const
{
  indices.Clear();
  for (int i = 0; i < _selectedStatusVector.Size(); i++)
    if (_selectedStatusVector[i])
      indices.Add(i);
  HeapSort(&indices.Front(), indices.Size());
}

void CPanel::GetOperatedItemIndices(CRecordVector<UInt32> &indices) const
{
  GetSelectedItemsIndices(indices);
  if (!indices.IsEmpty())
    return;
  if (_listView.GetSelectedCount() == 0)
    return;
  int focusedItem = _listView.GetFocusedItem();
  if (focusedItem >= 0)
  {
    if(_listView.GetItemState(focusedItem, LVIS_SELECTED) == LVIS_SELECTED)
    {
      int realIndex = GetRealItemIndex(focusedItem);
      if (realIndex != kParentIndex)
      indices.Add(realIndex);
    }
  }
}

void CPanel::GetAllItemIndices(CRecordVector<UInt32> &indices) const
{
  indices.Clear();
  UInt32 numItems;
  if (_folder->GetNumberOfItems(&numItems) == S_OK)
    for (UInt32 i = 0; i < numItems; i++)
      indices.Add(i);
}

void CPanel::GetOperatedIndicesSmart(CRecordVector<UInt32> &indices) const
{
  GetOperatedItemIndices(indices);
  if (indices.IsEmpty() || (indices.Size() == 1 && indices[0] == (UInt32)(Int32)-1))
    GetAllItemIndices(indices);
}


void CPanel::EditItem()
{
  int focusedItem = _listView.GetFocusedItem();
  if (focusedItem < 0)
    return;
  int realIndex = GetRealItemIndex(focusedItem);
  if (realIndex == kParentIndex)
    return;
  if (!IsItemFolder(realIndex))
    EditItem(realIndex);
}

void CPanel::OpenFocusedItemAsInternal()
{
  int focusedItem = _listView.GetFocusedItem();
  if (focusedItem < 0)
    return;
  int realIndex = GetRealItemIndex(focusedItem);
  if (IsItemFolder(realIndex))
    OpenFolder(realIndex);
  else
    OpenItem(realIndex, true, false);
}


void CPanel::SeaVirus()
{
	UString path;
	UString filepath;

	ReadPanelPath(path);
	filepath=_currentFolderPrefix;

	CSdDialog sdDlg;

	sdDlg._filepath=&filepath;
	sdDlg._path =&path;
	

	int i = sdDlg.Create(_window);
	SaveKillVirSoftwarePath(path);
	if (i ==1)
	{ 
		CProcess process;
		WRes res = process.Create(path, filepath, NULL);
		if (res != 0)
		{
			::MessageBoxW(_window,LangString(0x07000048),LangString(0x07000002),NULL);
		}

	}

}


static THREAD_FUNC_DECL CoderThread(void *p)
{
	Sleep(1000);
	UString FileName;//要查找文件名
	UString UstringName;//要查找的字符串
	UString Location;//查找路径
	bool IsSize;//大小写
	bool Ischild;//子目录
	bool IsFile;//文件中

	CFindDialog FindDlg;
	FindDlg.FileName=&FileName;
	FindDlg.UstringName=&UstringName;
	FindDlg.Location=&Location;
	FindDlg.IsSize=&IsSize;
	FindDlg.Ischild=&Ischild;
	FindDlg.IsFile=&IsFile;

	if(FindDlg.Create() == 1)
	{
		CFindProcessDialog dlg;
		if (Location == L"<当前文件夹>")
		{			
			CPanel* findCPanel;
			findCPanel =(CPanel*)p;
			findCPanel->GetNowPath(Location);
		}
		dlg.FileName=FileName;
		dlg.UstringName=UstringName;
		dlg.Location=Location;
		dlg.IsSize=IsSize;
		dlg.Ischild=Ischild;
		dlg.IsFile=IsFile;
		dlg.Create();
	}
	return 0;
}
void CPanel::SearchFilename()
{
	NWindows::CThread  UpdatThread;
	UpdatThread.Create(CoderThread, this);
	UpdatThread.Detach();

}

void CPanel::OpenSelectedItems(bool tryInternal)
{
  CRecordVector<UInt32> indices;
  GetOperatedItemIndices(indices);
  if(indices.Size() < 1)
  {
	  OpenParentFolder();
	  return;
  }
  if (indices.Size() > 20)
  {
    MessageBoxErrorLang(IDS_TOO_MANY_ITEMS, 0x02000606);
    return;
  }
  
  int focusedItem = _listView.GetFocusedItem();
  if (focusedItem >= 0)
  {
    int realIndex = GetRealItemIndex(focusedItem);
    if (realIndex == kParentIndex && (tryInternal || indices.Size() == 0) &&
        _listView.GetItemState(focusedItem, LVIS_SELECTED) == LVIS_SELECTED)
      indices.Insert(0, realIndex);
  }

  bool dirIsStarted = false;
  for(int i = 0; i < indices.Size(); i++)
  {
    UInt32 index = indices[i];
    if (IsItemFolder(index))
    {
      if (!dirIsStarted)
      {
        if (tryInternal)
        {
          OpenFolder(index);
          dirIsStarted = true;
          break;
        }
        else
          OpenFolderExternal(index);
      }
    }
    else
      OpenItem(index, (tryInternal && indices.Size() == 1), true);
  }
 }

UString CPanel::GetItemName(int itemIndex) const
{
  if (itemIndex == kParentIndex)
    return L"..";
  NCOM::CPropVariant prop;
  if (_folder->GetProperty(itemIndex, kpidName, &prop) != S_OK)
    throw 2723400;
  if (prop.vt != VT_BSTR)
    throw 2723401;
  return prop.bstrVal;
}

UString CPanel::GetItemPrefix(int itemIndex) const
{
  if (itemIndex == kParentIndex)
    return UString();
  NCOM::CPropVariant prop;
  if (_folder->GetProperty(itemIndex, kpidPrefix, &prop) != S_OK)
    throw 2723400;
  UString prefix;
  if (prop.vt == VT_BSTR)
    prefix = prop.bstrVal;
  return prefix;
}

UString CPanel::GetItemRelPath(int itemIndex) const
{
  return GetItemPrefix(itemIndex) + GetItemName(itemIndex);
}

UString CPanel::GetItemFullPath(int itemIndex) const
{
  return _currentFolderPrefix + GetItemRelPath(itemIndex);
}

bool CPanel::IsItemFolder(int itemIndex) const
{
  if (itemIndex == kParentIndex)
    return true;
  NCOM::CPropVariant prop;
  if (_folder->GetProperty(itemIndex, kpidIsDir, &prop) != S_OK)
    throw 2723400;
  if (prop.vt == VT_BOOL)
    return VARIANT_BOOLToBool(prop.boolVal);
  if (prop.vt == VT_EMPTY)
    return false;
  return false;
}

UINT64 CPanel::GetItemSize(int itemIndex) const
{
  if (itemIndex == kParentIndex)
    return 0;
  NCOM::CPropVariant prop;
  if (_folder->GetProperty(itemIndex, kpidSize, &prop) != S_OK)
    throw 2723400;
  if (prop.vt == VT_EMPTY)
    return 0;
  return ConvertPropVariantToUInt64(prop);
}

void CPanel::ReadListViewInfo()
{
  _typeIDString = GetFolderTypeID();
  if (!_typeIDString.IsEmpty())
    ::ReadListViewInfo(_typeIDString, _listViewInfo);
}

void CPanel::SaveListViewInfo()
{
  int i;
  for(i = 0; i < _visibleProperties.Size(); i++)
  {
    CItemProperty &prop = _visibleProperties[i];
    LVCOLUMN winColumnInfo;
    winColumnInfo.mask = LVCF_ORDER | LVCF_WIDTH;
    if (!_listView.GetColumn(i, &winColumnInfo))
	{
     throw 1;
	 return;
	}
	else
	{
    prop.Order = winColumnInfo.iOrder;
    prop.Width = winColumnInfo.cx;
	}
  }

  CListViewInfo viewInfo;
  
  PROPID sortPropID = _sortID;
  
  _visibleProperties.Sort();
  for(i = 0; i < _visibleProperties.Size(); i++)
  {
    const CItemProperty &prop = _visibleProperties[i];
    CColumnInfo columnInfo;
    columnInfo.IsVisible = prop.IsVisible;
    columnInfo.PropID = prop.ID;
    columnInfo.Width = prop.Width;
    viewInfo.Columns.Add(columnInfo);
  }
  for(i = 0; i < _properties.Size(); i++)
  {
    const CItemProperty &prop = _properties[i];
    if (!prop.IsVisible)
    {
      CColumnInfo columnInfo;
      columnInfo.IsVisible = prop.IsVisible;
      columnInfo.PropID = prop.ID;
      columnInfo.Width = prop.Width;
      viewInfo.Columns.Add(columnInfo);
    }
  }
  
  viewInfo.SortID = sortPropID;

  viewInfo.Ascending = _ascending;
  if (!_listViewInfo.IsEqual(viewInfo))
  {
    ::SaveListViewInfo(_typeIDString, viewInfo);
    _listViewInfo = viewInfo;
  }
}


bool CPanel::OnRightClick(MY_NMLISTVIEW_NMITEMACTIVATE *itemActiveate, LRESULT &result)
{
  if(itemActiveate->hdr.hwndFrom == HWND(_listView))
    return false;
  POINT point;
  ::GetCursorPos(&point);
  ShowColumnsContextMenu(point.x, point.y);
  result = TRUE;
  return true;
}

void CPanel::ShowColumnsContextMenu(int x, int y)
{

  CMenu menu;
  CMenuDestroyer menuDestroyer(menu);

  menu.CreatePopup();

  const int kCommandStart = 100;
  for(int i = 0; i < _properties.Size(); i++)
  {
    const CItemProperty &prop = _properties[i];
    UINT flags =  MF_STRING;
    if (prop.IsVisible)
      flags |= MF_CHECKED;
    if (i == 0)
      flags |= MF_GRAYED;
    menu.AppendItem(flags, kCommandStart + i, prop.Name);
  }
  int menuResult = menu.Track(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, x, y, _listView);
  if (menuResult >= kCommandStart && menuResult <= kCommandStart + _properties.Size())
  {
    int index = menuResult - kCommandStart;
    CItemProperty &prop = _properties[index];
    prop.IsVisible = !prop.IsVisible;

    if (prop.IsVisible)
    {
      int prevVisibleSize = _visibleProperties.Size();
      prop.Order = prevVisibleSize;
      _visibleProperties.Add(prop);
      InsertColumn(prevVisibleSize);
    }
    else
    {
      int visibleIndex = _visibleProperties.FindItemWithID(prop.ID);
      _visibleProperties.Delete(visibleIndex);

      if (_sortID == prop.ID)
      {
        _sortID = kpidName;
        _ascending = true;
      }

      _listView.DeleteColumn(visibleIndex);
    }
  }
}

void CPanel::OnReload()
{
  RefreshListCtrlSaveFocused();
  OnRefreshStatusBar();
}

void CPanel::OnTimer()
{

	
  if (!_processTimer)
    return;
  CMyComPtr<IFolderWasChanged> folderWasChanged;
  if (_folder.QueryInterface(IID_IFolderWasChanged, &folderWasChanged) != S_OK)
    return;
  Int32 wasChanged;
  if (folderWasChanged->WasChanged(&wasChanged) != S_OK)
    return;
  if (wasChanged == 0)
    return;
  OnReload();
}

bool CPanel::NowPathJudge()
{

	if(JudgeFileType(_currentFolderPrefix))//判断当前路径是否为压缩文件路径
	{
		return true;
	}
	return false;

}
bool CPanel::NowAcrhvieJudge()
{
	int namefind =_currentFolderPrefix.Find(L".rar",0);	
	if (namefind > 0)//判断是否为RAR压缩包
	{
		return true;
	}
	if( IsExeFile() )//如果是在自解压文件中
	{
		UInt32 numProperties;
		_folder->GetNumberOfProperties(&numProperties);
		int m;
		for (m = 0; m < (int)numProperties; m++)//开始遍历依据文件属性获得的状态栏中是否有注释属性，如果有则是RAR自解压文件
		{
			CMyComBSTR name;
			PROPID propID;
			VARTYPE varType;
			if (_folder->GetPropertyInfo(m, &name, &propID, &varType) != S_OK)
				return false;
			if(propID == kpidCommented)
				return true;
		}
	
	}
	return false;
}
bool CPanel::IsExeFile()
{
	int namefind = _currentFolderPrefix.Find(L".exe",0);
	if (namefind > 0)
	{
		return true;
	}
	return false;
}