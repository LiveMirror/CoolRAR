//InforDialog.h

#ifndef __INFORDIALOG_H
#define __INFORDIALOG_H

#include "Windows/Control/Dialog.h"
#include "Windows/Control/ComboBox.h"
#include "InforDialogRes.h"

class CInforDialog:public NWindows::NControl::CModalDialog
{
 public:
	virtual bool  OnInit();
	INT_PTR Create(HWND parent = 0) { return CModalDialog::Create(IDD_DIALOG_INFOR, parent);}
	void SetInformation(int count ,UStringVector &message,UStringVector &itemname);
    void GetInformation(int count ,UStringVector &message,UStringVector &itemname);
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
private:
	UStringVector name;
	UStringVector namevalue;
	int number;
};

#endif
