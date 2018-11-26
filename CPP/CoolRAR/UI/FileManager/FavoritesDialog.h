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
	UString _pressfloder;//ѹ���ļ����ļ�����
	 NWindows::NControl::CEdit _floder;
	 NWindows::NControl::CEdit _pressfile;
	 NWindows::NControl::CEdit _sign;
public:
		void GetFloderPath(const UString _currentFolderPrefix);//��ȡ��ǰ��Ŀ¼
		void GetSign(const UString getsign);                   //��ȡ��ʶ
		void SetFavoritesPath();
		virtual bool OnInit();
		virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
		INT_PTR Create(HWND parentWindow = 0) { return CModalDialog::Create(IDD_DIALOG_FAVORITE, parentWindow); }
};

#endif
