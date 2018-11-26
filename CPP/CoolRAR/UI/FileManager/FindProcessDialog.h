// PasswordDialog.h

#ifndef __FINDPROCESSDIALOG_H
#define __FINDPROCESSDIALOG_H

#include "Windows/Control/Dialog.h"
#include "Windows/Control/Edit.h"
#include "FindProcessDialogRes.h"
#include "Windows/Control/ListView.h"

struct FileNameStr
{
	UString FilePath;
	UString  FileName;
};
#ifndef _UNICODE
typedef BOOL (WINAPI * ShellExecuteExWP)(LPSHELLEXECUTEINFOW lpExecInfo);
#endif

typedef CObjectVector<FileNameStr> FileNameStrVector;
class CFindProcessDialog: public NWindows::NControl::CModalDialog
{
public:
	UString FileName;//要查找文件名
	UString UstringName;//要查找的字符串
	UString Location;//查找路径
	bool IsSize;//大小写
	bool Ischild;//子目录
	bool IsFile;//文件中


	NWindows::NControl::CListView _filelistview;

	FileNameStrVector filenamestrVect;
	void ListviewOnInit();   //列表控件初始化设置
	void InsertFileInfo(UString filename,UString filepath,int totalnum);   //为列表控件各项插入值
	bool OpenFouceItem();   //打开选中的文件
	void DoExtract();  //调用解压缩模块
	void DoOpenParentFloder(); //定位到文件所在父文件夹
	void UStringOk(UString FileP);
	void FileOk(UString FileP);
	bool OpenFileFind(UString FileP);
	
	virtual bool OnCommand(int code, int itemID, LPARAM lParam);
	virtual bool OnTimer(WPARAM , LPARAM );
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);

  virtual bool OnInit();
  INT_PTR Create(HWND parentWindow = 0) { return CModalDialog::Create(IDD_DIALOG_FIND_PROCESS, parentWindow); }
};
#endif
