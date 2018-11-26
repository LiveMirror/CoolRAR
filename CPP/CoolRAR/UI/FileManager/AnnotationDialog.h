// AnnotationDialog.h

#ifndef __ANNOTATIONDIALGO_H
#define __ANNOTATIONDIALGO_H

#include "AnnotationDialogRes.h"
#include "Windows/Control/Dialog.h"
#include "Windows/Control/Edit.h"

class CAnnotationDialog: public NWindows::NControl::CModalDialog
{
	NWindows::NControl::CEdit _Cedit;
    virtual void OnOK();
	void OpenDialog();
public:
	UString Title;
	UString Static;
	UString Value;
	UStringVector Strings;
	virtual bool OnInit();
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
        INT_PTR Create(HWND parent = 0) { return CModalDialog::Create(IDD_DIALOG_ANNOTATION, parent);}
};

#endif
