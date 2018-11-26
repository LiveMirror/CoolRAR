//PsWordDialog.h

#ifndef __PSWORDDIALOG_H
#define __PSWORDDIALOG_H

#include "Windows/Control/Dialog.h"
#include "Windows/Control/ComboBox.h"
#include "PsWordDialogRes.h"
#include "Windows/Control/Edit.h"

class CPsWordDialog: public NWindows::NControl::CModalDialog
{
private:
	NWindows::NControl::CComboBox _Algorithm;
	NWindows::NControl::CEdit _psWordin;
	NWindows::NControl::CEdit _psWordinnormal;
	NWindows::NControl::CEdit _psWordjudeg;
	UString _psWord;
public:
	virtual bool  OnInit();
	INT_PTR Create(HWND parent = 0) { return CModalDialog::Create(IDD_DIALOG_PsWord, parent);}
	virtual void ChangePassWord();
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	bool EnterPsWord();
};

#endif
