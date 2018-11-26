// ConventionalPage.h

#ifndef __CONVENTIONAL_PAGE_H
#define __CONVENTIONAL_PAGE_H

#include "Windows/Control/OptionPage.h"

#include "Windows/FileFind.h"
#include "Windows/Control/TreeView.h"
#include "../FileManager/SysIconUtils.h"

#include "Windows/Control/ComboBox.h"
#include "../Common/ExtractMode.h"
struct TreePathStructEX
{
	UString FilePath;
	HTREEITEM Root;
};
typedef CObjectVector<TreePathStructEX> TreePathStructVectorEX;

class CConventionalPage;
class CMyTreeViewEx: public NWindows::NControl::CTreeView
{
public:
	WNDPROC _origWindowProc;
	CConventionalPage * _ConvenPage;
	LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
};

class CConventionalPage: public NWindows::NControl::COptionPage
{
    CMyTreeViewEx _pathtree;
	
public:

	 NWindows::NControl::CComboBox _path;

	 UString DirectoryPath;
	 

	 
	 NExtract::NOverwriteMode::EEnum OverwriteMode;

	TreePathStructVectorEX treePathVector;
	virtual bool OnInit();
	virtual bool OnCommand(int code, int itemID, LPARAM param);
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);

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

	void InitTreeList();
	void ShowFile();
    void CreatFoldor();
	UString GetNewFileName(int i);
	

};
#endif
