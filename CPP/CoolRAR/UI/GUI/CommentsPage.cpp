#include "StdAfx.h"
#include "CommentsPage.h"
#include "CommentsPageRes.h"
#include "../FileManager/App.h"
#include "../FileManager/HelpUtils.h"
#include "Windows/CommonDialog.h"

#include "../FileManager/LangUtils.h"


#include "Windows/FileIO.h"
using namespace NWindows;
using namespace NFile;
using namespace NFind;
using namespace NIO;

static CIDLangPair kIDLangPairs[] =
{
	{ IDD_PAGE_COMMENTS, 0x04000549 },
	{ IDC_TEXT1_PAGE_COMMENTS, 0x04000550 },
	{ IDC_BUTTON1_PAGE_COMMENTS, 0x04000551 },
	{ IDC_TEXT2_PAGE_COMMENTS, 0x04000552 }
};

bool CCommentsPage::OnInit()
{

	LangSetWindowText(HWND(*this), 0x04000549);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));

	_ComboBox.Attach(GetItem(IDC_COMBO1_PAGE_COMMENTS));
	_ComboBox.SetText(path);
	_Cedit.Attach(GetItem(IDC_EDIT_PAGE_COMMENTS));
	SetText(Title);
	NControl::CStatic staticContol;
	staticContol.Attach(GetItem(IDC_EDIT_PAGE_COMMENTS));
	staticContol.SetText(Static);
	_Cedit.SetText(Value);
	for(int i = 0; i < Strings.Size(); i++)
		_Cedit.GetText(Strings[i]);
	NormalizeSize();
	return COptionPage::OnInit(); 
}


bool CCommentsPage::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_BUTTON1_PAGE_COMMENTS:
		OpenComments();
		break;
	case IDOK:
		OnOK();
		return true;
	}
	return COptionPage::OnButtonClicked(buttonID,buttonHWND);
}

void CCommentsPage::OnOK()
{
	_Cedit.GetText(Value);
	COptionPage::OnOK();
}
void CCommentsPage::OpenComments()
{
	UString fileName=L"";
	UString title = L"选择注释文件";
	WCHAR s[MAX_PATH]=L"*.txt\0*.txt\0所有文件\0*.*\0";
	UString resPath;
	if (AllOpenFileName(HWND(*this),  title, fileName, s, resPath))
	{
		CInFile infile;
		path=resPath;
		Strings.Add(path);
		for (int i = 0; i < Strings.Size(); i++)
		{
			_ComboBox.AddString(Strings[i]);
		}
		_ComboBox.SetCurSel(-1);
		_ComboBox.SetText(path);
		infile.Open(resPath);
		UInt64  fileLen;
		UInt32 processedSize;
		AString s;
		infile.GetLength(fileLen);
		char *p=s.GetBuffer((int)((size_t)fileLen)+1);
		infile.Read(p,(UInt32)fileLen,processedSize);
		p[fileLen] = 0;
		s.ReleaseBuffer();
		int nlen;
		nlen=MultiByteToWideChar(CP_ACP,0,p,-1,NULL,0);
		WCHAR *ptch;
		ptch=new WCHAR[nlen];
		MultiByteToWideChar(CP_ACP,0,p,-1,ptch,nlen);
		_Cedit.SetText(ptch);
		_Cedit.GetText(Delete->Info->CommentsValue);

		delete [] ptch;
		ptch = NULL;

	}
}
void CCommentsPage::SaveValues()
{
	_Cedit.GetText(Delete->Info->CommentsValue);
}
bool CCommentsPage::OnCommand(int code, int itemID, LPARAM lParam)
{
	if (code == EN_KILLFOCUS)
	{
		switch(itemID)
		{
		case IDC_EDIT_PAGE_COMMENTS:
			SaveValues();
			return true;
		}
	}
	return COptionPage::OnCommand(code, itemID, lParam);
}
