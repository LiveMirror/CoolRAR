// PasswordDialog.h

#ifndef __FINDDIALOG_H
#define __FINDDIALOG_H

#include "Windows/Control/Dialog.h"
#include "Windows/Control/Edit.h"
#include "Windows/Control/ComboBox.h"
#include "FindDialogRes.h"

class CFindDialog: public NWindows::NControl::CModalDialog
{

	NWindows::NControl::CComboBox _FileName;
	NWindows::NControl::CComboBox _Ustring;
	NWindows::NControl::CComboBox _Location;
	NWindows::NControl::CComboBox _FileType;


	virtual bool OnInit();

	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	

	void OnButtonHelp();
	void OnButtonOk();
	void OnButtonCancle();
	void OnButtonSave();

	public :

	UString *FileName;//Ҫ�����ļ���
	UString *UstringName;//Ҫ���ҵ��ַ���
	UString *Location;//����·��
	bool *IsSize;//��Сд
	bool *Ischild;//��Ŀ¼
	bool *IsFile;//�ļ���

	INT_PTR Create(HWND parentWindow = 0) { return CModalDialog::Create(IDD_DIALOG_FIND, parentWindow); }
};

#endif
