// Windows/Control/ListView.cpp

#include "StdAfx.h"

#include "Windows/Control/TreeView.h"

bool WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize)
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
bool MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{

	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);

	if(dwSize < dwMinSize)
	{
		return false;
	}
	MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);  
	return true;
}




namespace NWindows {
namespace NControl {

bool CTreeView::CreateEx(DWORD exStyle, DWORD style,
      int x, int y, int width, int height,
      HWND parentWindow, HMENU idOrHMenu,
      HINSTANCE instance, LPVOID createParam)
{
  return CWindow::CreateEx(exStyle, WC_TREEVIEW, TEXT(""), style, x, y, width,
      height, parentWindow, idOrHMenu, instance, createParam);
}

HTREEITEM   CTreeView::InsertItem(UString Name,UINT uImag,UINT uChild)
{ 
	TCHAR szBuffer[4096];

	memset(szBuffer,0,4096*sizeof(TCHAR));

	WCharToMByte(Name.GetBuffer(Name.Length()),szBuffer,sizeof(szBuffer)/sizeof(szBuffer[4096]));

	
	TVINSERTSTRUCT item;
	item.hParent=TVI_ROOT ;
	item.hInsertAfter=TVI_ROOT;
	item.item.mask=TVIF_IMAGE|TVIF_TEXT|TVIF_CHILDREN|TVIF_SELECTEDIMAGE;
	item.item.pszText=szBuffer;
	item.item.iImage=uImag;
	item.item.iSelectedImage=uImag;
	item.item.cChildren=uChild;
	return TreeView_InsertItem(_window,&item);
}

HTREEITEM   CTreeView::InsertItem(HTREEITEM  hParent,UString Name,UINT uImag,UINT uChild)
{ 
	TCHAR szBuffer[4096];

	memset(szBuffer,0,4096*sizeof(TCHAR));

	WCharToMByte(Name.GetBuffer(Name.Length()),szBuffer,sizeof(szBuffer)/sizeof(szBuffer[4096]));

	TVINSERTSTRUCT item;
	item.hParent=hParent ;
	item.hInsertAfter=TVI_ROOT;
	item.item.mask=TVIF_IMAGE|TVIF_TEXT|TVIF_CHILDREN|TVIF_SELECTEDIMAGE;
	item.item.pszText=szBuffer;
	item.item.iImage=uImag;
	item.item.iSelectedImage=uImag;
	item.item.cChildren=uChild;
	return TreeView_InsertItem(_window,&item);
}


HTREEITEM   CTreeView::InsertItem(HTREEITEM  hParent,UString Name,UINT uImag)
{ 
	TCHAR szBuffer[4096];

	memset(szBuffer,0,4096*sizeof(TCHAR));

	WCharToMByte(Name.GetBuffer(Name.Length()),szBuffer,sizeof(szBuffer)/sizeof(szBuffer[4096]));

	TVINSERTSTRUCT item;
	item.hParent=hParent ;
	item.hInsertAfter=TVI_ROOT;
	item.item.mask=TVIF_IMAGE|TVIF_TEXT|TVIF_SELECTEDIMAGE;
	item.item.pszText=szBuffer;
	item.item.iImage=uImag;
	item.item.iSelectedImage=uImag;
	return TreeView_InsertItem(_window,&item);
}


}}
