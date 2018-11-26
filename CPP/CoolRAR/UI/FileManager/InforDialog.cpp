#include "StdAfx.h"
#include "InforDialogRes.h"
#include "InforDialog.h"
#include "HelpUtils.h"
#include "LangUtils.h"

using namespace NWindows;

static CIDLangPair kIDLangPairs[] =
{
	{ IDC_INFOR_TEXT_STATIS_STATE              , 0x04000415 },
	{ IDC_INFOR_TEXT_ACCOMPLISH                , 0x04000416 },
	{ IDC_INFOR_TEXT_COMPRES_SRATE             , 0x04000417 },
	{ IDC_INFOR_TEXT_COMPRES_SRATE_VALU        , 0x04000418 },
	{ IDC_INFOR_TEXT_COMPRESS_SIZE             , 0x04000419 },
	{ IDC_INFOR_TEXT_COMPRESS_TIME             , 0x04000420 },
	{ IDC_INFOR_TEXT_COMPRESS_START            , 0x04000421 },
	
	{ IDC_INFOR_BUTTON_OK                      , 0x05000001 },
	{ IDC_INFOR_BUTTON_CANCEL                  , 0x05000002 },
	{ IDC_INFOR_BUTTON_HELP                    , 0x05000003 },
	
};
bool CInforDialog::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000422);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	SetInformation(number, namevalue, name);

   return CModalDialog::OnInit();
}

void CInforDialog::SetInformation(int count ,UStringVector &message,UStringVector &itemname)
{
	int j =0;
	
	if(message.Size()!=0)
	{	
		for(int i =0; i <=count ;i++)
		{	
				if( !(message.operator[](i).IsEmpty())
					&&i ==count )                       //假使当前为文件路径信息则放入负责显示文件路径信息文本控件
				{
					UString path =message.operator[](i);
					if (path.Length() > 110)         //当文件路径过长时将超出的部分用省略号代替
					{
						while(path.Length() > 110 || path.Back() != WCHAR_PATH_SEPARATOR)
						{
							path.DeleteBack();
						}
						path +=L"...";

					}
					SetItemText(IDC_INFOR_TEXT_COMPRESS_SIZE_VALU,path);

					
				}
				else if(itemname.operator[](i) ==L"注释")
				{
					if(message.operator [](i).Length() > 10)//注释太长的话剪短并加上省略
					{
						while(message.operator [](i).Length() >10)
						{
							message.operator [](i).DeleteBack();
						}
						message.operator [](i)+=L"...";
					}
					
					SetItemText(IDC_INFOR_TEXT_COMPRESS_START_VALU,message.operator[](i));
				}
				else if(itemname.operator[](i) ==L"固实")
				{
					SetItemText(IDC_INFOR_TEXT_COMPRESS_TIME_VALU,message.operator[](i));
				}
				else if(   !(message.operator[](i).IsEmpty())  )
				{
					
					SetItemText(j+IDC_INFOR_TEXT_FILE_VALU,message.operator[](i));
					 
					SetItemText(j+IDC_INFOR_TEXT_FILE,itemname.operator[](i));
					
					j++;
				}								
		}
	}
	else
	{
		::MessageBoxW(HWND(*this),LangString(0x07000012),LangString(0x07000025),MB_ICONWARNING);
	}
}

void CInforDialog::GetInformation(int count ,UStringVector &message,UStringVector &itemname)
{
	
	number =count -1;
	for(int i =0;i <= number; i++)
	{
		UString abac =message.operator [](i);
		namevalue.Add(message.operator [](i));
		name.Add(itemname.operator [](i));
		
	}
	
}
bool  CInforDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
		case IDC_INFOR_BUTTON_OK:
		case IDC_INFOR_BUTTON_CANCEL:
			OnOK();
		break;

		case IDC_INFOR_BUTTON_HELP:
			ShowHelpWindow(NULL, L"dialog/wen_jian_xin_xi.htm");
		break;
		
		default:
			return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
	}
	return false;
}