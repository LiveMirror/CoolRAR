
#include "StdAfx.h"
#include "Drapdrop.h"
#include "Windows\Memory.h"
#include "Common\MyString.h"
#include "Common\StringConvert.h"
#include "Windows/Shell.h"
#include "EnumFormatEtc.h"

class CDataObject:
	public IDataObject,
	public CMyUnknownImp
{
private:
	FORMATETC m_Etc;
	UINT m_SetFolderFormat;

public:
	MY_UNKNOWN_IMP1_MT(IDataObject)

		STDMETHODIMP GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM medium);
	STDMETHODIMP GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM medium);
	STDMETHODIMP QueryGetData(LPFORMATETC pformatetc );

	STDMETHODIMP GetCanonicalFormatEtc ( LPFORMATETC /* pformatetc */, LPFORMATETC pformatetcOut)
	{ pformatetcOut->ptd = NULL; return ResultFromScode(E_NOTIMPL); }

	STDMETHODIMP SetData(LPFORMATETC etc, STGMEDIUM *medium, BOOL release);
	STDMETHODIMP EnumFormatEtc(DWORD drection, LPENUMFORMATETC *enumFormatEtc);

	STDMETHODIMP DAdvise(FORMATETC * /* etc */, DWORD /* advf */, LPADVISESINK /* pAdvSink */, DWORD * /* pdwConnection */)
	{ return OLE_E_ADVISENOTSUPPORTED; }
	STDMETHODIMP DUnadvise(DWORD /* dwConnection */) { return OLE_E_ADVISENOTSUPPORTED; }
	STDMETHODIMP EnumDAdvise( LPENUMSTATDATA * /* ppenumAdvise */) { return OLE_E_ADVISENOTSUPPORTED; }

	CDataObject();

	NMemory::CGlobal hGlobal;
	UString Path;
};

BOOL CDropTargetEx::DragdropRegister(HWND hwnd,DWORD AcceptKeyState)
{
	OleInitialize(NULL);

	HRESULT s = ::RegisterDragDrop(hwnd ,this);
	if (SUCCEEDED(s))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

static void ReadUnicodeStrings(const wchar_t *p, size_t size, UStringVector &names)
{
	names.Clear();
	UString name;
	for (;size > 0; size--)
	{
		wchar_t c = *p++;
		if (c == 0)
		{
			if (name.IsEmpty())
				break;
			names.Add(name);
			name.Empty();
		}
		else
			name += c;
	}
}

static void ReadAnsiStrings(const char *p, size_t size, UStringVector &names)
{
	names.Clear();
	AString name;
	for (;size > 0; size--)
	{
		char c = *p++;
		if (c == 0)
		{
			if (name.IsEmpty())
				break;
			names.Add(GetUnicodeString(name));
			name.Empty();
		}
		else
			name += c;
	}
}
static void GetNamesFromDataObject(IDataObject *dataObject, UStringVector &names)
{
	names.Clear();
	FORMATETC etc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM medium;
	HRESULT res = dataObject->GetData(&etc, &medium);
	if (res != S_OK)
		return;
	if (medium.tymed != TYMED_HGLOBAL)
		return;
	{
		NWindows::NMemory::CGlobal global;
		global.Attach(medium.hGlobal);
		size_t blockSize = GlobalSize(medium.hGlobal);
		NWindows::NMemory::CGlobalLock dropLock(medium.hGlobal);
		const DROPFILES* dropFiles = (DROPFILES*)dropLock.GetPointer();
		if (dropFiles == 0)
			return;
		if (blockSize < dropFiles->pFiles)
			return;
		size_t size = blockSize - dropFiles->pFiles;
		const void *namesData = (const Byte *)dropFiles + dropFiles->pFiles;
		if (dropFiles->fWide)
			ReadUnicodeStrings((const wchar_t *)namesData, size / sizeof(wchar_t), names);
		else
			ReadAnsiStrings((const char *)namesData, size, names);
	}
}

STDMETHOD CDropTargetEx::DragEnter(IDataObject * dataObject, DWORD keyState,
									POINTL pt, DWORD *effect)
{
	GetNamesFromDataObject(dataObject, m_SourcePaths);
	QueryGetData(dataObject);
	m_DataObject = dataObject;
	return DragOver(keyState, pt, effect);
}

STDMETHOD CDropTargetEx::DragOver(DWORD keyState, POINTL pt, DWORD *effect)
{
// 	PositionCursor(pt);
// 	SetPath();
	*effect = GetEffect(keyState, pt, *effect);
	return S_OK;
}


STDMETHOD CDropTargetEx::DragLeave()
{
// 	RemoveSelection();
// 	SetPath(false);
	m_DataObject.Release();
	return S_OK;
}

DWORD CDropTargetEx::GetEffect(DWORD keyState, POINTL /* pt */, DWORD allowedEffect)
{
	if (!m_DropIsAllowed || !m_PanelDropIsAllowed)
		return DROPEFFECT_NONE;

	if (!m_SetPathIsOK)
		allowedEffect &= ~DROPEFFECT_MOVE;

	DWORD effect = 0;
	if (keyState & MK_CONTROL)
		effect = allowedEffect & DROPEFFECT_COPY;
	else if (keyState & MK_SHIFT)
		effect = allowedEffect & DROPEFFECT_MOVE;
	if (effect == 0)
	{
		if (allowedEffect & DROPEFFECT_COPY)
			effect = DROPEFFECT_COPY;
		if (allowedEffect & DROPEFFECT_MOVE)
		{
				effect = DROPEFFECT_MOVE;
		}
	}
	if (effect == 0)
		return DROPEFFECT_NONE;
	return effect;
}
