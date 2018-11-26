// CommentsPage.h

#ifndef  __COMMENTS_PAGE_H
#define  __COMMENTS_PAGE_H

#include "Windows/Control/OptionPage.h"
#include "../Common/ExtractMode.h"
#include "Windows/Control/Edit.h"
#include "Windows/Control/ComboBox.h"
#include "DeletePage.h"

class CCommentsPage: public NWindows::NControl::COptionPage
{
	
	NWindows::NControl::CEdit _Cedit;
	NWindows::NControl::CComboBox _ComboBox;
	virtual void OnOK();
	void OpenComments();
	void SaveValues();
public:
	CDeletePage* Delete;
	UString Title;
	UString Static;
	UString Value;
	UString path;
	UStringVector Strings;
	virtual bool OnInit();
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	virtual bool OnCommand(int code, int itemID, LPARAM lParam);

};
#endif

