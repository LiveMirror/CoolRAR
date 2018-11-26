#ifndef __FAVORITESDIALOG_H
#define __FAVORITESDIALOG_H

#include "Windows/Control/Dialog.h"
#include "Windows/Control/Edit.h"
#include "FavoritesDialogRes.h"
#include "Windows/Control/ListView.h"



class CFavoritesDialog: public NWindows::NControl::CModalDialog
{
private:
	

	 
	UString _folderPrefix;
	UString sign;
	UString _pressfloder;//压缩文件子文件夹名
	 NWindows::NControl::CEdit _floder;
	 NWindows::NControl::CEdit _pressfile;
	 NWindows::NControl::CEdit _sign;
public:
		void GetFloderPath(const UString _currentFolderPrefix);//获取当前根目录
		void GetSign(const UString getsign);                   //获取标识
		void SetFavoritesPath();
		virtual bool OnInit();
		virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
		INT_PTR Create(HWND parentWindow = 0) { return CModalDialog::Create(IDD_DIALOG_FAVORITE, parentWindow); }
};

#endif
