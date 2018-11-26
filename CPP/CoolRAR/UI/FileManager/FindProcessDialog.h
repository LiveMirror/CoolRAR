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
	UString FileName;//Ҫ�����ļ���
	UString UstringName;//Ҫ���ҵ��ַ���
	UString Location;//����·��
	bool IsSize;//��Сд
	bool Ischild;//��Ŀ¼
	bool IsFile;//�ļ���


	NWindows::NControl::CListView _filelistview;

	FileNameStrVector filenamestrVect;
	void ListviewOnInit();   //�б�ؼ���ʼ������
	void InsertFileInfo(UString filename,UString filepath,int totalnum);   //Ϊ�б�ؼ��������ֵ
	bool OpenFouceItem();   //��ѡ�е��ļ�
	void DoExtract();  //���ý�ѹ��ģ��
	void DoOpenParentFloder(); //��λ���ļ����ڸ��ļ���
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
