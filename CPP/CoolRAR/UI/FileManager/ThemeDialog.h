//ThemeDialog.h

#ifndef __THEMEDIALOG_H
#define __THEMEDIALOG_H

#include "ThemeDialogRes.h"
#include "Windows/Control/Dialog.h"
#include "Windows/Control/ListView.h"

class CThemeDialog: public NWindows::NControl::CModalDialog
{
private:
	UStringVector ThemeStr;
	UStringVector ThemeFolderName;//������ļ�����.
	NWindows::NControl::CListView Themelistview;
public:
	UString ThemeFolder;//Themes �ļ���·����
	UString ThemesBagName;//������ļ�����
	UString ThemeBagPath; //�����·�����·����
	UString ReadFile(UString ThemeIdenPath);//��ȡ��ʶ�ļ�������.��
	void AddTheme();
	void ListViewOnit();
	void DeleteTheme();
	void LoadTheme();
	void MeunTheme(int Fouse);

	UString ReadThemeReg();
	UString GetAppDataPath();

	virtual bool OnInit();
	virtual void OnOK();
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	INT_PTR Create(HWND parent = 0) { return CModalDialog::Create(IDD_DIALOG_THEME, parent);}
};

#endif


