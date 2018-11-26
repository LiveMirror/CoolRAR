// SeniorPage.h

#ifndef __SENIOR_PAGE_H
#define __SENIOR_PAGE_H
#include "Windows/Control/OptionPage.h"
#include "../Common/ExtractMode.h"
#include "Windows/Control/ComboBox.h"
#include "Windows/Control/Edit.h"
#include "OptionsDlg.h"
#include "SeniorPageRes.h"
#include "DeletePage.h"
class CSeniorPage: public NWindows::NControl::COptionPage
{

	NWindows::NControl::CEdit _password1Control;
	NWindows::NControl::CEdit _password2Control;
	NWindows::NControl::CComboBox _encryptionMethod;

	




public:
	CDeletePage* Delete;
	void UpdatePasswordControl();
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	bool IsShowPasswordChecked() const
	{ return IsButtonChecked(IDC_SEN_PAGE_BOX_EDIT_ONE) == BST_CHECKED; }
	void SavePass();
	bool IsZipFormat();
	UString GetEncryptionMethodSpec();
	int FindRegistryFormat(const UString &name);
	virtual bool OnInit();
	virtual bool OnTimer(WPARAM LM, LPARAM PM);
	virtual bool OnCommand(int code, int itemID, LPARAM lParam);
	void CheckControlsEnable();
	 void SetEncryptionMethod();
};
#endif