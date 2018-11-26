#ifndef __EXTRACTDIALOG_H
#define __EXTRACTDIALOG_H

#include "Windows/Control/Dialog.h"
#include "Windows/Control/ComboBox.h"
#include "Windows/FileFind.h"
#include "Windows/Control/TreeView.h"
#include "SysIconUtils.h"
#include "ExtractDialogRes.h"

struct TreePathStruct
{
	UString FilePath;
	HTREEITEM Root;
};
typedef CObjectVector<TreePathStruct> TreePathStructVector;

class CMyTreeView: public NWindows::NControl::CTreeView
{
public:
	WNDPROC _origWindowProc;
	CExtractDialog * _ConvenPage;
	LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
};

class CExtractDialog: public NWindows::NControl::CModalDialog
{
	
	CMyTreeView _pathtree;

public:

	NWindows::NControl::CComboBox _path;

	UString DirectoryPath;
	bool overmode;
	TreePathStructVector treePathVector;
	virtual bool OnInit();
	virtual bool OnCommand(int code, int itemID, LPARAM param);
	
	bool OnNotify(UINT /* controlID */, LPNMHDR header);
	bool OnNotifyList(LPNMHDR header);
	bool TreeClick(HTREEITEM hItem = NULL);

	bool changTreeSize; //展开方式

	UString GetTreeItem(HTREEITEM  hItem);
	HTREEITEM GetTreeItemHt(UString FilePath);

	bool AddTree(UString Filepath,HTREEITEM  ParentRoot,bool Layers,bool PartBool);

	bool AddDriveTree(UString driveName,UString path,bool needchild,HTREEITEM  ParentRoot);

	bool GetDriveInfo(UString& driveName);
	void InitTree();
	void ShowFile();
	void CreatFoldor();
	UString GetNewFileName(int i);
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	INT_PTR Create(HWND parent = 0) { return CModalDialog::Create(IDD_DIALOG_EXTRACT, parent); }
	
};

#endif