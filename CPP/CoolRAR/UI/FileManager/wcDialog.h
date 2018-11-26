//wcDialog.h

#ifndef __WCDIALOG_H
#define __WCDIALOG_H

#include "Windows/Control/Dialog.h"
#include "Windows/Control/ComboBox.h"
#include "wcDialogRes.h"
enum{
	FirstStpe =0,
	RealeaseOneStpe,
	RealeaseTwoStpe,
	
};
enum{
	PressOneStpe =11,
	PressTwoStpe,
	PressThrStpe,
};
enum{
	AddOneStpe =21,
	AddTwoStpe,

};

class CwcDialog: public NWindows::NControl::CModalDialog
{
	
private:
	int stpeSign;
	 NWindows::NControl::CComboBox _path;
	 NWindows::NControl::CComboBox _pressname;
public:

	UString Title;
	UString Static;
	UString Value;
	UString Pressname;
	UString fullPath;
	UStringVector Strings;
	UStringVector PressString;
	
	virtual bool OnInit();
	        void ReflashVector(UStringVector &VString,UString string,NWindows::NControl::CComboBox &Object);
	virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
	INT_PTR Create(HWND parent = 0) { return CModalDialog::Create(IDD_DIALOG_WC, parent); }
	virtual bool CheckRadioButton(int buttonID);
	virtual void NextRealeaseOne();
	virtual void FrontRealeaseTwo();
			//����ļ�
		    void SearchFile();
			void SearchPressFile();
			//�����µ�ѹ���ļ���һ��
			void NewPressOne();
			void NewPressTwo();
			void NewPressThree();
			void PressTwoFront();
			//�����ļ����Ѵ��ڵ�ѹ���ļ���
			void AddRealeaseOne();
			void AddRealeaseTwo();
			void AddRealeaseFront();
	
};

#endif
