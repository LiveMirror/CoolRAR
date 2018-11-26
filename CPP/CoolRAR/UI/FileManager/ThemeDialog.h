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
	UStringVector ThemeFolderName;//主题包文件夹名.
	NWindows::NControl::CListView Themelistview;
public:
	UString ThemeFolder;//Themes 文件夹路径。
	UString ThemesBagName;//主题包文件名。
	UString ThemeBagPath; //主题包路径存放路径。
	UString ReadFile(UString ThemeIdenPath);//读取标识文件的内容.。
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


