#include "StdAfx.h"
#include "ExtractDialog.h"
#include "resource.h"
#include "App.h"
#include "Panel.h"
#include "HelpUtils.h"


using namespace NWindows;
static LRESULT APIENTRY TreeViewSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWindow tempDialog(hwnd);
	CMyTreeView *w = (CMyTreeView *)(tempDialog.GetUserDataLongPtr());
	if (w == NULL)
		return 0;
	return w->OnMessage(message, wParam, lParam);
}
LRESULT CMyTreeView::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
#ifndef _UNICODE
	if (g_IsNT)
		return CallWindowProcW(_origWindowProc, *this, message, wParam, lParam);
	else
#endif
		return CallWindowProc(_origWindowProc, *this, message, wParam, lParam);
}

bool CExtractDialog::OnNotify(UINT /* controlID */, LPNMHDR header)
{
	if (header->hwndFrom == _pathtree)
		return OnNotifyList(header);
	return false;
} 
bool CExtractDialog::OnNotifyList(LPNMHDR header)
{

	HTREEITEM _currentRoot = _pathtree.GetSelectendItem();
	if (_currentRoot != NULL)
	{
		if (DirectoryPath != GetTreeItem(_currentRoot))
		{
			DirectoryPath = GetTreeItem(_currentRoot);
			_path.SetText(DirectoryPath);

		}

	}



	static HTREEITEM ParentRoot =NULL;
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(header);

	if (pNMTreeView != NULL )
	{
		if ( 2 == pNMTreeView->action)
		{
			if ( ParentRoot != pNMTreeView->itemNew.hItem)
			{
				ParentRoot = pNMTreeView->itemNew.hItem;
				UString path = GetTreeItem(ParentRoot);

				if (path != L"" && changTreeSize == false)
				{
					return	TreeClick(ParentRoot);
				}
				else changTreeSize =false;
			}
			else changTreeSize =false;
		}
		else if ( 1 == pNMTreeView->action)
		{
			changTreeSize =false;
		}


	}

	switch(header->code)
	{
	case NM_CLICK:
		{
			return true;
		}
	case NM_DBLCLK:
		{
			TreeClick();
		}
	}
	return false;
}
bool CExtractDialog::OnInit()
{
	InitTree();
	_path.Attach(GetItem(IDC_CON_PAGE_COMBO_PATH));
	_path.SetText(DirectoryPath);


	overmode = false;
#endif

	switch(OverwriteMode)
	{
	case NExtract::NOverwriteMode::kAskBefore:
		CheckRadioButton(IDC_CON_PAGE_BOX_COVER_ONE,IDC_EXT_PAGE_BOX_COVER_FOUR,IDC_CON_PAGE_BOX_COVER_ONE);
		break;
	case  NExtract::NOverwriteMode::kWithoutPrompt:
		CheckRadioButton(IDC_CON_PAGE_BOX_COVER_ONE,IDC_EXT_PAGE_BOX_COVER_FOUR,IDC_CON_PAGE_BOX_COVER_TWO);
		break;
	case  NExtract::NOverwriteMode::kSkipExisting:
		CheckRadioButton(IDC_CON_PAGE_BOX_COVER_ONE,IDC_EXT_PAGE_BOX_COVER_FOUR,IDC_CON_PAGE_BOX_COVER_THREE);
		break;
	case  NExtract::NOverwriteMode::kAutoRename:
		CheckRadioButton(IDC_CON_PAGE_BOX_COVER_ONE,IDC_EXT_PAGE_BOX_COVER_FOUR,IDC_CON_PAGE_BOX_COVER_FOUR);
		break;
	default:break;
	}

	CheckButton(IDC_EXT_PAGE_BOX_UPDATE_ONE,true);

	return COptionPage::OnInit();
}

bool CExtractDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_EXT_PAGE_BOX_COVER_ONE:
		overmode = false;
		httpworkdat.WorkDateUp(100);
		return true;
	case IDC_EXT_PAGE_BOX_COVER_TWO:
		overmode = false;
		httpworkdat.WorkDateUp(101);
		return true;
	case IDC_EXT_PAGE_BOX_COVER_THREE:
		overmode = false;
		httpworkdat.WorkDateUp(102);
		return true;
	case IDC_EXT_PAGE_BOX_COVER_FOUR:
		overmode = false;
		httpworkdat.WorkDateUp(103);
		return true;
	case IDC_EXT_PAGE_BUTTON_SHOW_TREE :
		ShowFile();
		httpworkdat.WorkDateUp(104);
		return true;
	case IDC_EXT_PAGE_BUTTON_NEW_F:
		CreatFoldor();
		httpworkdat.WorkDateUp(105);
		return true;
	}
	return COptionPage::OnButtonClicked(buttonID, buttonHWND);
}

bool CExtractDialog::OnCommand(int code, int itemID, LPARAM param)
{
	if (itemID == IDC_EXT_PAGE_COMBO_PATH)
	{
		switch(code)
		{
		case CBN_KILLFOCUS:
			_path.GetText(DirectoryPath);
			return true;
		}

	}
	return COptionPage::OnCommand(code, itemID, param);
}
extern bool WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize);

bool CExtractDialog::GetDriveInfo(UString& driveName)
{
	UString volumeName;
	bool needRead = true;
	TCHAR szBuffer[4096];
	memset(szBuffer,0,4096*sizeof(TCHAR));
	WCharToMByte(driveName.GetBuffer(driveName.Length()),szBuffer,sizeof(szBuffer)/sizeof(szBuffer[4096]));
	DWORD dwSerialNumber;
	if (!::GetVolumeInformation(szBuffer,
		NULL, 0, &dwSerialNumber, NULL, NULL, NULL, 0))
	{
		needRead =false;
	}
	if (NFile::NSystem::MyGetDriveType(driveName) == DRIVE_FIXED)
	{
		UString fileSystemName;
		DWORD volumeSerialNumber, maximumComponentLength, fileSystemFlags;
		NFile::NSystem::MyGetVolumeInformation(driveName,
			volumeName,
			&volumeSerialNumber, &maximumComponentLength, &fileSystemFlags,
			fileSystemName);
		if (volumeName.Length() == 0)
		{
			volumeName = L"本地磁盘";
		}
	}
	else if(NFile::NSystem::MyGetDriveType(driveName) == DRIVE_CDROM )
	{
		UString fileSystemName;
		DWORD volumeSerialNumber, maximumComponentLength, fileSystemFlags;
		NFile::NSystem::MyGetVolumeInformation(driveName,
			volumeName,
			&volumeSerialNumber, &maximumComponentLength, &fileSystemFlags,
			fileSystemName);
		if (volumeName.Length() == 0)
		{
			volumeName = L"驱动器";
		}
	}
	else if (NFile::NSystem::MyGetDriveType(driveName) == DRIVE_REMOVABLE)
	{
		UString fileSystemName;
		DWORD volumeSerialNumber, maximumComponentLength, fileSystemFlags;
		NFile::NSystem::MyGetVolumeInformation(driveName,
			volumeName,
			&volumeSerialNumber, &maximumComponentLength, &fileSystemFlags,
			fileSystemName);
		if (volumeName.Length() == 0)
		{
			volumeName = L"可移动磁盘";
		}
	}
	else 
	{
		UString fileSystemName;
		DWORD volumeSerialNumber, maximumComponentLength, fileSystemFlags;
		NFile::NSystem::MyGetVolumeInformation(driveName,
			volumeName,
			&volumeSerialNumber, &maximumComponentLength, &fileSystemFlags,
			fileSystemName);
		if (volumeName.Length() == 0)
		{
			volumeName = L"其它";
		}
	}

	if(driveName.Length() != 0 && driveName.Right(1) == L"\\")
	{
		driveName = driveName.Left(driveName.Length() -1);
	}

	driveName = volumeName+L"  ( "+driveName+L" )";
	return needRead;
}

bool CExtractDialog::AddDriveTree(UString driveName,UString path,bool needchild,HTREEITEM  ParentRoot)
{

	CFindFile Findfile;
	int iconIndex ;//系统图标
	UString SavePath = path;
	UString strPath= SavePath+L"*.*";
	TreePathStruct treePathStruct;

	if (!needchild)
	{
		CFileInfoW info;
		DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
		//if (info.Find(path))attrib = info.Attrib;
		GetRealIconIndex(path, attrib,iconIndex);
		HTREEITEM Root=_pathtree.InsertItem(ParentRoot,driveName,iconIndex);
		treePathStruct.Root=Root;
		treePathStruct.FilePath=path;
		treePathVector.Add(treePathStruct);
	}
	else
	{
		int childNum;//子文件个数
		UStringVector childFileVector;//子文件名
		childFileVector =Findfile.FindDirFile(path);
		childNum=childFileVector.Size();
		CFileInfoW info;
		DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
		if (info.Find(path))attrib = info.Attrib;
		GetRealIconIndex(path, attrib,iconIndex);
		HTREEITEM OneRoot =_pathtree.InsertItem(ParentRoot,driveName, iconIndex,childNum);
		treePathStruct.FilePath=path;
		treePathStruct.Root=OneRoot;
		treePathVector.Add(treePathStruct);
		// 		for (int i= 0; i<childNum ;i++)
		// 		{
		// 			AddTree(path+childFileVector[i]+L"\\",OneRoot,false,false);//
		// 		}
	}
	return true;
}

bool CExtractDialog::AddTree(UString Filepath,HTREEITEM  ParentRoot,bool Layers,bool PartBool)
{
	CFindFile Findfile;
	int iconIndex ;//系统图标
	int childNum;//子文件个数
	UStringVector childFileVector;//子文件名
	UString ItemName;//文件名称
	UString SavePath = Filepath;
	UString strPath= SavePath+L"*.*";
	TreePathStruct treePathStruct;

	childFileVector =Findfile.FindDirFile(SavePath);
	childNum=childFileVector.Size();

	if (SavePath.Right(1) == L"\\")
	{
		SavePath = SavePath.Left(SavePath.Length()-1);
	}
	while(SavePath.Right(1) != L"\\" && SavePath.Length() != 0)
	{
		ItemName = SavePath.Right(1)+ItemName;
		SavePath = SavePath.Left(SavePath.Length()-1);
	}

	CFileInfoW info;
	DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
	if (info.Find(Filepath))attrib = info.Attrib;
	GetRealIconIndex(Filepath, attrib,iconIndex);


	HTREEITEM OneRoot;
	if(ItemName == L"桌面" || PartBool)
	{
		OneRoot = ParentRoot;
	}
	else
	{
		OneRoot =_pathtree.InsertItem(ParentRoot,ItemName, iconIndex,childNum);
		treePathStruct.FilePath=Filepath;
		treePathStruct.Root=OneRoot;
		treePathVector.Add(treePathStruct);
	}

	CFileInfoW fileinfo;
	bool  bWork = Findfile.FindFirst(strPath,fileinfo);

	while(bWork)
	{
		bWork = Findfile.FindNext(fileinfo);
		if(fileinfo.Name == L"..")continue;
		if(bWork)
		{
			SavePath = Filepath + fileinfo.Name + L"\\";
			if(fileinfo.IsDir())
			{
				childFileVector =Findfile.FindDirFile(SavePath);
				childNum=childFileVector.Size();
				if (info.Find(SavePath))attrib = info.Attrib;
				GetRealIconIndex(SavePath, attrib,iconIndex);
				if(childNum != 0)
				{
					if(Layers)
					{
						AddTree(SavePath,OneRoot,false,false);
					}
					else
					{
						HTREEITEM root = _pathtree.InsertItem(OneRoot,fileinfo.Name,iconIndex,childNum);
						treePathStruct.FilePath=SavePath;
						treePathStruct.Root=root;

						treePathVector.Add(treePathStruct);
					}
				}
				else 
				{
					HTREEITEM root = _pathtree.InsertItem(OneRoot,fileinfo.Name,iconIndex);
					treePathStruct.FilePath=SavePath;
					treePathStruct.Root=root;

					treePathVector.Add(treePathStruct);
				}
			}
		}
	}
	return true;
}

bool CExtractDialog::TreeClick(HTREEITEM hItem)
{
	bool BSetText =false;
	HTREEITEM ParentRoot;
	if (hItem == NULL)
	{
		BSetText =true;
		ParentRoot = _pathtree.GetSelectendItem();
	}
	else ParentRoot = hItem;


	if(ParentRoot == NULL)return false;
	HTREEITEM ChildRoot = _pathtree.GetChildItem(ParentRoot);
	if(ChildRoot == NULL)
	{
		UString path = GetTreeItem(ParentRoot);
		AddTree(path,ParentRoot,false,true);
	}

	if (BSetText)
	{
		DirectoryPath = GetTreeItem(ParentRoot);
		_path.SetText(DirectoryPath);
	}



	return true;
}

UString CExtractDialog::GetTreeItem(HTREEITEM hItem)
{
	TreePathStruct treePathStruct;
	if (hItem != NULL)
	{
		for(int i=0 ;i<treePathVector.Size();i++)
		{
			treePathStruct=treePathVector[i];
			if(treePathStruct.Root == hItem)return treePathStruct.FilePath;
		}
	}
	return L"";

}

HTREEITEM CExtractDialog::GetTreeItemHt(UString FilePath)
{

	TreePathStruct treePathStruct;
	if (FilePath != NULL)
	{
		for(int i=0 ;i<treePathVector.Size();i++)
		{
			treePathStruct=treePathVector[i];
			if(treePathStruct.FilePath == FilePath)return treePathStruct.Root;
		}
	}

	return NULL;

}

void CExtractDialog::ShowFile()
{

	UString name;
	UString LastName = DirectoryPath;
	HTREEITEM root;
	int i=LastName.Find(L"\\");
	while(i!= -1)
	{
		name = name+LastName.Left(i+1);
		LastName = LastName.Right(LastName.Length() - i-1);
		root = GetTreeItemHt(name);
		if(root != NULL)
		{
			HTREEITEM childroot=_pathtree.GetChildItem(root);
			if (childroot == NULL)
			{
				AddTree(name,root,false,true);
			}
			changTreeSize =true;
			_pathtree.Expand(root);
		}
		i=LastName.Find(L"\\");
	}
}

void CExtractDialog::CreatFoldor()
{
	_path.GetText(DirectoryPath);
	UString name;
	UString LastName = DirectoryPath;
	HTREEITEM root;
	int i=LastName.Find(L"\\");
	while(i!= -1)
	{
		name = name+LastName.Left(i+1);
		LastName = LastName.Right(LastName.Length() - i-1);
		root = GetTreeItemHt(name);
		if(root != NULL)
		{
			HTREEITEM childroot=_pathtree.GetChildItem(root);
			if (childroot == NULL)
			{
				AddTree(name,root,false,true);
			}
			changTreeSize =true;
			_pathtree.Expand(root);
		}
		i=LastName.Find(L"\\");
	}
	name =  GetTreeItem(root);
	if(DirectoryPath == name)
	{
		LastName =name;
		if(LastName.Right(1) != L"\\")LastName=LastName+L"\\";
		int i=1;
		UString	LName=LastName+GetNewFileName(i);
		HTREEITEM childroot=_pathtree.GetChildItem(root);
		while(childroot != NULL)
		{
			name =  GetTreeItem(childroot);
			if(name == LName)
			{
				i++;
				LName=LastName+GetNewFileName(i);
				childroot=_pathtree.GetChildItem(root);
			}
			else
			{
				childroot=_pathtree.GetNextSiblin(childroot);
			}
		}
		UString itemName =GetNewFileName(i);
		childroot =_pathtree.InsertItem(root,itemName.Left(itemName.Length()-1), 4);
		changTreeSize =true;
		_pathtree.Expand(root);
		TreePathStruct treePathStruct;
		treePathStruct.FilePath = LName;
		treePathStruct.Root = childroot;
		treePathVector.Add(treePathStruct);
		DirectoryPath = LName;
		_path.SetText(DirectoryPath);
	}
	else
	{

		UString itemNext;
		LastName = DirectoryPath;
		if(LastName.Right(1) == L"\\")LastName=LastName.Left(LastName.Length()-1);
		while(LastName.Right(1) != L"\\" && LastName.Length() != 0)
		{
			itemNext = LastName.Right(1)+itemNext;
			LastName=LastName.Left(LastName.Length()-1);
		}
		root = GetTreeItemHt(LastName);
		root =_pathtree.InsertItem(root,itemNext, 4,1);
		TreePathStruct treePathStruct;
		treePathStruct.FilePath = DirectoryPath;
		treePathStruct.Root = root;
		treePathVector.Add(treePathStruct);


		UString itemName =GetNewFileName(1);
		HTREEITEM childroot =_pathtree.InsertItem(root,itemName.Left(itemName.Length()-1), 4);
		changTreeSize =true;
		_pathtree.Expand(root);
		treePathStruct.FilePath = DirectoryPath+itemName;
		treePathStruct.Root = childroot;
		treePathVector.Add(treePathStruct);
		DirectoryPath = DirectoryPath+itemName;
		_path.SetText(DirectoryPath);
	}
}

void CExtractDialog::InitTree()
{

	_pathtree.Attach(GetItem(IDC_EXT_PAGE_TREE));
#ifndef UNDER_CE
	_pathtree.SetUnicodeFormat(true);
#endif
	_pathtree.SetUserDataLongPtr(LONG_PTR(&_pathtree));
	_pathtree._ConvenPage = this;

#ifndef _UNICODE
	if(g_IsNT)
		_pathtree._origWindowProc =
		(WNDPROC)_pathtree.SetLongPtrW(GWLP_WNDPROC, LONG_PTR(TreeViewSubclassProc));
	else
#endif
		_pathtree._origWindowProc =
		(WNDPROC)_pathtree.SetLongPtr(GWLP_WNDPROC, LONG_PTR(TreeViewSubclassProc));

	_pathtree.SetImageList(GetSysImageList(true), LVSIL_SMALL);
	_pathtree.SetImageList(GetSysImageList(true), LVSIL_NORMAL);

	int iconIndex;
	int childNum;
	UString ItemName;
	UString Filepath;
	UStringVector childFileVector;

	TreePathStruct treePathStruct;

	//我的桌面
	ItemName = RootFolder_GetName_Desktop(iconIndex);
	Filepath = GetMyDesktopPath();
	CFindFile Findfile;
	childFileVector =Findfile.FindDirFile(Filepath);
	childNum=childFileVector.Size();
	if(childNum == 0)childNum =1;  //强制我的桌面存在子节点
	HTREEITEM DesktopRoot=_pathtree.InsertItem(ItemName,iconIndex,childNum);
	treePathStruct.FilePath=Filepath;
	treePathStruct.Root=DesktopRoot;
	treePathVector.Add(treePathStruct);

	//我的文档
	ItemName = RootFolder_GetName_Documents(iconIndex);
	childFileVector =Findfile.FindDirFile(GetMyDocumentsPath());
	childNum=childFileVector.Size();
	HTREEITEM DocumentsRoot=_pathtree.InsertItem(DesktopRoot,ItemName,iconIndex,childNum);
	treePathStruct.FilePath=GetMyDocumentsPath();
	treePathStruct.Root=DocumentsRoot;

	treePathVector.Add(treePathStruct);
	//我的电脑
	ItemName = RootFolder_GetName_Computer(iconIndex);
	UStringVector driveStrings;
	MyGetLogicalDriveStrings(driveStrings);
	childNum = driveStrings.Size();
	HTREEITEM ComputerRoot=_pathtree.InsertItem(DesktopRoot,ItemName,iconIndex,childNum);
	treePathStruct.FilePath=L"我的电脑";
	treePathStruct.Root=ComputerRoot;

	treePathVector.Add(treePathStruct);

	for (int i = 0; i < driveStrings.Size(); i++)
	{
		UString s = driveStrings[i];
		UString driveName = s;
		bool needChild = GetDriveInfo(driveName);//我的电脑
		AddDriveTree(driveName,s,needChild,ComputerRoot);

	}
	changTreeSize =true;
	_pathtree.Expand(ComputerRoot);//展开我的电脑
	AddTree(GetMyDesktopPath(),DesktopRoot,false,false);//我的桌面文件夹
	changTreeSize =true;
	_pathtree.Expand(DesktopRoot);//展开我的桌面

}
UString CExtractDialog::GetNewFileName(int i)
{
	switch(i)
	{
	case 1:return L"新建文件夹\\";
	case 2:return L"新建文件夹 (2)\\";
	case 3:return L"新建文件夹 (3)\\";
	case 4:return L"新建文件夹 (4)\\";
	case 5:return L"新建文件夹 (5)\\";
	case 6:return L"新建文件夹 (6)\\";
	case 7:return L"新建文件夹 (7)\\";
	case 8:return L"新建文件夹 (8)\\";
	case 9:return L"新建文件夹 (9)\\";
	case 10:return L"新建文件夹 (10)\\";
	case 11:return L"新建文件夹 (11)\\";
	case 12:return L"新建文件夹 (12)\\";
	case 13:return L"新建文件夹 (13)\\";
	case 14:return L"新建文件夹 (14)\\";
	case 15:return L"新建文件夹 (15)\\";
	case 16:return L"新建文件夹 (16)\\";
	case 17:return L"新建文件夹 (17)\\";
	case 18:return L"新建文件夹 (18)\\";
	case 19:return L"新建文件夹 (19)\\";
	case 20:return L"新建文件夹 (20)\\";
	case 21:return L"新建文件夹 (21)\\";
	case 22:return L"新建文件夹 (22)\\";
	case 23:return L"新建文件夹 (23)\\";
	default: return  L"新建文件夹(2)\\";
	}
}