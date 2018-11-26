// PanelKey.cpp

#include "StdAfx.h"

#include "Panel.h"
#include "HelpUtils.h"

#include "../../PropID.h"
#include "App.h"
#include "PsWordDialog.h"
#include "../Common/CompressCall.h"


static LPCWSTR kHelpTopic = L"FM/index.htm";

extern HINSTANCE g_hInstance;
extern CApp g_App;
extern void OptionsDialog(HWND hwndOwner, HINSTANCE /* hInstance */);
struct CVKeyPropIDPair
{
  WORD VKey;
  PROPID PropID;
};

static CVKeyPropIDPair g_VKeyPropIDPairs[] =
{
  { VK_F3, kpidName },
  { VK_F4, kpidExtension },
  { VK_F5, kpidMTime },
  { VK_F6, kpidSize },
  { VK_F7, kpidNoProperty }
};

static int FindVKeyPropIDPair(WORD vKey)
{
  for (int i = 0; i < sizeof(g_VKeyPropIDPairs) / sizeof(g_VKeyPropIDPairs[0]); i++)
    if (g_VKeyPropIDPairs[i].VKey == vKey)
      return i;
  return -1;
}


bool CPanel::OnKeyDown(LPNMLVKEYDOWN keyDownInfo, LRESULT &result)
{
  if (keyDownInfo->wVKey == VK_TAB && keyDownInfo->hdr.hwndFrom == _listView)
  {
    _panelCallback->OnTab();
    return false;
  }
  bool alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
  bool ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
  bool rightCtrl = (::GetKeyState(VK_RCONTROL) & 0x8000) != 0;
  bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
  bool num = (::GetKeyState(VK_NUMLOCK) & 0x8000) != 0;
  result = 0;

  if (keyDownInfo->wVKey >= '0' && keyDownInfo->wVKey <= '9' &&
      (rightCtrl || alt))
  {
    int index = keyDownInfo->wVKey - '0';
    if (shift)
    {
      SetBookmark(index);
      return true;
    }
    else
    {
      OpenBookmark(index);
      return true;
    }
  }

  if ((keyDownInfo->wVKey == VK_F2 ||
    keyDownInfo->wVKey == VK_F1) && alt && !ctrl && !shift)
  {
    _panelCallback->SetFocusToPath(keyDownInfo->wVKey == VK_F1 ? 0 : 1);
    return true;
  }

  if ((keyDownInfo->wVKey == VK_F9) && !alt && !ctrl && !shift)
  {
    g_App.SwitchOnOffOnePanel();
  }

  if(keyDownInfo->wVKey >= VK_F3 && keyDownInfo->wVKey <= VK_F12 && ctrl)
  {
    int index = FindVKeyPropIDPair(keyDownInfo->wVKey);
    if (index >= 0)
      SortItemsWithPropID(g_VKeyPropIDPairs[index].PropID);
  }

  switch(keyDownInfo->wVKey)
  {
    case VK_SHIFT:
    {
      _selectionIsDefined = false;
      _prevFocusedItem = _listView.GetFocusedItem();
      break;
    }
    
    case VK_F1:
    {
       ShowHelpWindow(NULL, kHelpTopic);
      break;
    }
  
    case VK_F2:
    {
      if (!alt && !ctrl &&!shift)
      {
        RenameFile();
        return true;
      }
      break;
    }
	case VK_F3:
	{
		if (!alt && !ctrl &&!shift)
		{
			SearchFilename();
			return true;
		}
		break;
	}
    case VK_F4:
    {
      if (!alt && !ctrl && !shift)
      {
        EditItem();
        return true;
      }
      if (!alt && !ctrl && shift)
      {
        CreateFile();
        return true;
      }
      break;
    }
    case VK_F5:
    {
      if (!alt && !ctrl)
      {
	    RefreshListCtrl();
        RefreshStatusBar();
        return true;
      }
      break;
    }
    case VK_DELETE:
    {
	  if(_deleteEnable)
      DeleteItems(!shift);
      return true;
    }
    case VK_INSERT:
    {
      if (!alt)
      {
        if (ctrl && !shift)
        {
          EditCopy();
          return true;
        }
        if (shift && !ctrl)
        {
          return true;
        }
        if (!shift && !ctrl && _mySelectMode)
        {
          OnInsert();
          return true;
        }
      }
      return false;
    }
    case VK_DOWN:
    {
      if(shift)
        OnArrowWithShift();
      return false;
    }
    case VK_UP:
    {
      if (alt)
        _panelCallback->OnSetSameFolder();
      else if(shift)
        OnArrowWithShift();
      return false;
    }
    case VK_RIGHT:
    {
      if (alt)
        _panelCallback->OnSetSubFolder();
      else if(shift)
        OnArrowWithShift();
      return false;
    }
    case VK_LEFT:
    {
      if (alt)
        _panelCallback->OnSetSubFolder();
      else if(shift)
        OnArrowWithShift();
      return false;
    }
    case VK_NEXT:
    {
      if (ctrl && !alt && !shift)
      {
        return true;
      }
      break;
    }
    case VK_ADD:
    {
      if (alt)
        SelectByType(true);
      else if (shift)
        SelectAll(true);
      else if(!ctrl)
        SelectSpec(true);
      return true;
    }
    case VK_SUBTRACT:
    {
      if (alt)
        SelectByType(false);
      else if (shift)
        SelectAll(false);
      else
        SelectSpec(false);
      return true;
    }
    case VK_BACK:
      OpenParentFolder();
      return true;
    case 'A':
      if(ctrl)
      {
        SelectAll(true);
        return true;
      }
	  if(alt)
	  {
		 AddToArchive();
		 return true;
	  }
      return false;
    case 'X':
      if (ctrl)
      {
        return true;
      }
	  if(alt)
	  {
		  PressItself();
		  return true;
	  }
      return false;
    case 'C':
      if (ctrl)
      {
        CopyFiles();
        return true;
      }
      return false;
    case 'V':
      if (ctrl)
      {
		PlasterFiles();
        return true;
      }
	  if(alt)
	  {
		  OpenSelectedItems(true); 
		  return true;
	  }
      return false;
    case 'N':
      if (ctrl)
      {
        CreateFile();
        return true;
      }
      return false;
    case 'R':
      if(ctrl)
      {
        OnReload();
        return true;
      }
	  if(alt)
	  {
		 g_App.RepairFile();
		 return true;
	  }
      return false;
    case 'Z':
      if(ctrl && _deleteEnable)
      {
        ChangeComment();
        return true;
      }
      return false;
	case 'O':
		if(ctrl)
		{
			OpenRealeaseFile();
			return true;
		}
		return false;
	case 'P':
		if(ctrl)
		{
			CPsWordDialog p_Word;
			p_Word.Create();
			return true;
		}
		return false;
	case '+':
		if(num)
		{
			SelectSpec(true);
			RefreshStatusBar();
			return true;
		}
		return false;
	case '*':
		if(num)
		{
			InvertSelection();
			RefreshStatusBar();
			return true;
		}
		return false;
	case 'E':
		if(alt)
		{
			ExtractArchives();
			return true;
		}
		return false;
	case 'T':
		if(alt)
		{
			TestArchives();
			return true;
		}
		if(ctrl)
		{
			ChangTreeViewMode();
			return true;
		}
		return false;
		
	case 'W':
		if(alt)
		{
			ExtractDirectly();
			return true;
		}
		return false;
	case 'M':
		if(alt)
		{
			ChangeComment();
			return true;
		}
		return false;
	
	case 'Q':
		if(alt)
		{
			FromChange();
			return true;
		}
		return false;
	
	case 'I':
		if(alt)
		{
			Properties();
			return true;
		}
		if(ctrl)
		{
			PrintFile();
			return true;
		}
		return false;
	case 'B':
		if(alt)
		{
			Benchmark();
			return true;
		}
		return false;
	case 'S':
		if(ctrl)
		{
			OptionsDialog(HWND(*this), g_hInstance);
			return true;
		}
		return false;
	case'F':
		if(ctrl)
		{
			g_App.Favorites();
		}
	case'D':
		if(alt)
		{
			SeaVirus();
		}
	
	
    case '1':
    case '2':
    case '3':
    case '4':
      if(ctrl)
      {
        return true;
      }
      return false;
    case VK_MULTIPLY:
      {
        InvertSelection();
        return true;
      }
  }
  return false;
}
