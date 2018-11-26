
#include <windows.h>
#include <shlobj.h>
#include "Common/MyCom.h"
#include "MyCom2.h"



class CDropTargetEx:
	public IDropTarget,
	public CMyUnknownImp
{
	CMyComPtr<IDataObject> m_DataObject;
	 UStringVector m_SourcePaths;
	 bool m_DropIsAllowed; 
	 bool m_SetPathIsOK;

	 DWORD GetEffect(DWORD keyState, POINTL pt, DWORD allowedEffect);

public:
	MY_UNKNOWN_IMP1_MT(IDropTarget)
	STDMETHOD(DragEnter)(IDataObject * dataObject, DWORD keyState, POINTL pt, DWORD *effect);
	STDMETHOD(DragOver)(DWORD keyState, POINTL pt, DWORD * effect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject * dataObject, DWORD keyState, POINTL pt, DWORD *effect);
	
	BOOL DragdropRegister(HWND hwnd,DWORD AcceptKeyState);

};
