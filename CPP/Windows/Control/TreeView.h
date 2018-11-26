// Windows/Control/TreeView.h

#ifndef __WINDOWS_CONTROL_TREEVIEW_H
#define __WINDOWS_CONTROL_TREEVIEW_H

#include "Windows/Window.h"

#include <commctrl.h>

namespace NWindows {
namespace NControl {


class CTreeView: public NWindows::CWindow
{
public:  
	bool CreateEx(DWORD exStyle, DWORD style,
				int x, int y, int width, int height,
				HWND parentWindow, HMENU idOrHMenu,
				HINSTANCE instance, LPVOID createParam);

#ifndef UNDER_CE
	bool SetUnicodeFormat(bool fUnicode){ return BOOLToBool(TreeView_SetUnicodeFormat(_window, BOOLToBool(fUnicode))); }
#endif

	HIMAGELIST SetImageList(HIMAGELIST  imageList, INT imageListType)
	{ return TreeView_SetImageList(_window, imageList, imageListType); }
	
	HTREEITEM  InsertItem(UString Name,UINT uImag,UINT uChild);

	HTREEITEM InsertItem(HTREEITEM  hParent,UString Name,UINT uImag,UINT uChild);

	HTREEITEM InsertItem(HTREEITEM  hParent,UString Name,UINT uImag);
	
	BOOL Expand(HTREEITEM hItem){ return TreeView_Expand(_window, hItem, TVE_EXPAND);}

	BOOL DeleteItem(HTREEITEM hItem){ return TreeView_DeleteItem(_window, hItem);}

	HTREEITEM GetSelectendItem(){  return TreeView_GetSelection(_window);}
	
	HTREEITEM GetChildItem(HTREEITEM HItem) { return TreeView_GetChild(_window,HItem);}

	HTREEITEM GetSelection() { return TreeView_GetSelection(_window);}

	HTREEITEM GetNextSiblin(HTREEITEM HItem) {return TreeView_GetNextSibling(_window, HItem);}

};

	}}

#endif
