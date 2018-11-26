// PanelSort.cpp

#include "StdAfx.h"

#include "Windows/PropVariant.h"

#include "../../PropID.h"

#include "Panel.h"

using namespace NWindows;

static UString GetExtension(const UString &name)
{
  int dotPos = name.ReverseFind(L'.');
  if (dotPos < 0)
    return UString();
  return name.Mid(dotPos);
}

int CALLBACK CompareItems2(LPARAM lParam1, LPARAM lParam2, LPARAM lpData)
{
  if (lpData == NULL)
    return 0;
  CPanel *panel = (CPanel*)lpData;
  
  switch(panel->_sortID)
  {
    case kpidName:
    {
      const UString name1 = panel->GetItemName((int)lParam1);
      const UString name2 = panel->GetItemName((int)lParam2);
      int res = name1.CompareNoCase(name2);

      return res;
    }
    case kpidNoProperty:
    {
      return MyCompare(lParam1, lParam2);
    }
    case kpidExtension:
    {
      const UString ext1 = GetExtension(panel->GetItemName((int)lParam1));
      const UString ext2 = GetExtension(panel->GetItemName((int)lParam2));
      return ext1.CompareNoCase(ext2);
    }
  }

  PROPID propID = panel->_sortID;

  NCOM::CPropVariant propVariant1, propVariant2;
  // Name must be first property
  panel->_folder->GetProperty((UINT32)lParam1, propID, &propVariant1);
  panel->_folder->GetProperty((UINT32)lParam2, propID, &propVariant2);
  if (propVariant1.vt != propVariant2.vt)
    return 0; // It means some BUG
  if (propVariant1.vt == VT_BSTR)
  {
    return _wcsicmp(propVariant1.bstrVal, propVariant2.bstrVal);
  }
  return propVariant1.Compare(propVariant2);
}

int CALLBACK CompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM lpData)
{
  if (lpData == NULL) return 0;
  if (lParam1 == kParentIndex) return -1;
  if (lParam2 == kParentIndex) return 1;

  CPanel *panel = (CPanel*)lpData;

  bool isDir1 = panel->IsItemFolder((int)lParam1);
  bool isDir2 = panel->IsItemFolder((int)lParam2);
  
  if (isDir1 && !isDir2) return -1;
  if (isDir2 && !isDir1) return 1;

  int result = CompareItems2(lParam1, lParam2, lpData);
  return panel->_ascending ? result: (-result);
}



void CPanel::SortItemsWithPropID(PROPID propID)
{
  if (propID == _sortID)
    _ascending = !_ascending;
  else
  {
    _sortID = propID;
    _ascending = true;
    switch (propID)
    {
      case kpidSize:
      case kpidPackSize:
      case kpidCTime:
      case kpidATime:
      case kpidMTime:
        _ascending = false;
      break;
    }
  }
  
  _listView.SortItems(CompareItems, (LPARAM)this);
  _listView.EnsureVisible(_listView.GetFocusedItem(), false);
  InitColumns();//选择排列方式后更新箭头显示位置
  RefreshListCtrl();

}

bool CPanel::ItemsListRuleJudge(PROPID propID)
{
	if(propID == _sortID)
		return true;
	else
		return false;
}

void CPanel::OnColumnClick(LPNMLISTVIEW info)
{
  SortItemsWithPropID(_visibleProperties[info->iSubItem].ID);
}
