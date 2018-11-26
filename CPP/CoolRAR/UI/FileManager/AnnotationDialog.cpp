#include "StdAfx.h"
#include "Windows/CommonDialog.h"
#include "AnnotationDialog.h"
#include "App.h"
#include "HelpUtils.h"
#include "resource.h"
#include "LangUtils.h"

#include "Windows/FileIO.h"
using namespace NWindows;
using namespace NFile;
using namespace NFind;
using namespace NIO;

static CIDLangPair kIDLangPairs[] =
{
	{ IDC_TEXT_ANNOTATION                                           , 0x04000423 },
	{ IDC_BUTTON_FILE_ANNOTATION                                    , 0x04000424 },
	{ IDC_BUTTON_ENSURE_ANNOTATION                                  , 0x05000001 },
	{ IDC_BUTTON_EXIT_ANNOTRTION                                    , 0x05000002 },
	{ IDC_BUTTON_HELP_ANNOTATION                                    , 0x05000003 },
};

extern CApp g_App;
bool CAnnotationDialog::OnInit()
{ 
	LangSetWindowText(HWND(*this), 0x04000425);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	_Cedit.Attach(GetItem(IDC_EDITTEXT_ANNOTATION));
	SetText(Title);
	NControl::CStatic staticContol;
	staticContol.Attach(GetItem(IDC_EDITTEXT_ANNOTATION));
	staticContol.SetText(Static);
	_Cedit.SetText(Value);
	for(int i = 0; i < Strings.Size(); i++)
		_Cedit.GetText(Strings[i]);
	NormalizeSize();



	return CModalDialog::OnInit();

}


bool CAnnotationDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
		case IDC_BUTTON_FILE_ANNOTATION:
			OpenDialog();
			break;
		case IDC_BUTTON_ENSURE_ANNOTATION:
			OnOK();
			return true;
		case IDC_BUTTON_EXIT_ANNOTRTION:
			CModalDialog::OnCancel();
			break;
		case IDC_BUTTON_HELP_ANNOTATION:
			static LPCWSTR kFMHelpTopic = L"dialog/index.htm";
			ShowHelpWindow(NULL,kFMHelpTopic);
			return true;
	}
	return CModalDialog::OnButtonClicked(buttonID,buttonHWND);
}
void CAnnotationDialog::OnOK()
{
	_Cedit.GetText(Value);
	CModalDialog::OnOK();
}
void CAnnotationDialog::OpenDialog()
{
	UString fileName=L"";
	UString title = L"选择注释文件";
	WCHAR s[MAX_PATH]=L"*.txt\0*.txt\0所有文件\0*.*\0";
	UString resPath;
	if (AllOpenFileName(HWND(*this),  title, fileName, s, resPath))
	{
		
	

		CInFile infile;
		infile.Open(resPath);
		UInt64  fileLen;
		UInt32 processedSize;
		AString s;

		infile.GetLength(fileLen);
		char *p=s.GetBuffer((int)((size_t)fileLen)+1);
		infile.Read(p,(UInt32)fileLen,processedSize);
		p[fileLen] = 0;
		s.ReleaseBuffer();

		int nlen=MultiByteToWideChar(CP_ACP,0,p,-1,NULL,0);
		WCHAR *ptch=new WCHAR[nlen];
		MultiByteToWideChar(CP_ACP,0,p,-1,ptch,nlen);

		_Cedit.SetText(ptch);
		
		delete [] ptch;
		ptch = NULL;
	}
}