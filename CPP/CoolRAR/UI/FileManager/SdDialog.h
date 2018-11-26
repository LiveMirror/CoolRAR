// SdDialog.h

#ifndef __SDDIALOG_H
#define __SDDIALOG_H

#include "Windows/Control/Dialog.h"
#include "Windows/Control/Edit.h"
#include "Windows/Control/ComboBox.h"
#include "SdDialogRes.h"

class CSdDialog: public NWindows::NControl::CModalDialog
{
public:

	NWindows::NControl::CComboBox _comBox;

	NWindows::NControl::CEdit _pathControl;
	NWindows::NControl::CEdit _filepathControl;

	UString* _path;//ɱ�����Ŀ¼
	UString* _filepath;//��ɱ�ļ�Ŀ¼

  virtual bool OnInit();
  	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
  INT_PTR Create(HWND parentWindow = 0) { return CModalDialog::Create(IDD_DIALOG_SD, parentWindow); }
};

#endif
