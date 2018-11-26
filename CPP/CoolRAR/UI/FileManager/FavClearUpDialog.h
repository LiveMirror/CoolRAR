#ifndef __FAVCLEARUPSDIALOG_H
#define __FAVCLEARUPSDIALOG_H

#include "Windows/Control/Dialog.h"
#include "FavClearUpDialogRes.h"
#include "Windows/Control/ListView.h"



class CFavClearUpDialog: public NWindows::NControl::CModalDialog
{
private:
		UStringVector cSign;
		UStringVector cStrings;
		UStringVector cSignKeep;    //���ڱ��浱ǰ�ղؼ���Ϣ���û������ļ��к�ѡ��ȡ��ʱ���ļ�����Ϣ���л�ԭ
		UStringVector cStringsKeep;
		NWindows::NControl::CListView favlistview;
public:
		void ListviewOnInit();
		void ItemDelete();
		void FavoritesEdit();
		void MoveDown();
		void MoveUp();
		virtual bool OnInit();
		virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
		INT_PTR Create(HWND parentWindow = 0) { return CModalDialog::Create(IDD_DIALOG_FAVCLEARUP, parentWindow); }
};

#endif
