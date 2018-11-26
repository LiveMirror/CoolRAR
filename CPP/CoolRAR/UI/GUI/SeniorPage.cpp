#include "StdAfx.h"
#include "SeniorPage.h"
#include "../Explorer/MyMessages.h"
#include "Common/StringConvert.h"

#include "../FileManager/LangUtils.h"

using namespace NWindows;


static CIDLangPair kIDLangPairs[] =
{
	{ IDD_PAGE_SENIOR, 0x04000527 },
	{ IDC_SEN_PAGE_GROUP_EDIT, 0x04000528 },
	{ IDC_SEN_PAGE_TEXT_ONE, 0x04000529 },
	{ IDC_SEN_PAGE_TEXT_TWO, 0x04000530 },
	{ IDC_SEN_PAGE_BOX_EDIT_ONE, 0x04000531},
	{ IDC_SEN_PAGE_TEXT_THREE, 0x04000532 },
	{ IDC_SEN_PAGE_BOX_EDIT_TWO, 0x04000533 }
};

bool CSeniorPage::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000527);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));

	_password1Control.Attach(GetItem(IDC_SEN_PAGE_EDITTEXT_ONE));
	_password2Control.Attach(GetItem(IDC_SEN_PAGE_EDITTEXT_TWO));
	_password1Control.SetText(Delete->Info->Password);
	_password2Control.SetText(Delete->Info->Password);
	_encryptionMethod.Attach(GetItem(IDC_SEN_PAGE_COMBO_ONE));

	 UpdatePasswordControl();
	 
	 SetEncryptionMethod();

	 CheckControlsEnable();

	 SetTimer(101,10);
	
 return COptionPage::OnInit();
}

void CSeniorPage::UpdatePasswordControl()
{
	bool showPassword = IsShowPasswordChecked();
	TCHAR c = showPassword ? 0: TEXT('*');
	_password1Control.SetPasswordChar(c);
	_password2Control.SetPasswordChar(c);
	UString password;
	_password1Control.GetText(password);
	_password1Control.SetText(password);
	_password2Control.GetText(password);
	_password2Control.SetText(password);

	int cmdShow = showPassword ? SW_HIDE : SW_SHOW;
	ShowItem(IDC_SEN_PAGE_TEXT_TWO, cmdShow);
	_password2Control.Show(cmdShow);
}


bool CSeniorPage::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	
	case IDC_SEN_PAGE_BOX_EDIT_ONE:	
		{
			UpdatePasswordControl();
			SavePass();
			return true;
		}
	case IDC_SEN_PAGE_BOX_EDIT_TWO:
		{
			SavePass();
			return true;
		}
	}
	return COptionPage::OnButtonClicked(buttonID, buttonHWND);
}


static bool IsAsciiString(const UString &s)
{
	for (int i = 0; i < s.Length(); i++)
	{
		wchar_t c = s[i];
		if (c < 0x20 || c > 0x7F)
			return false;
	}
	return true;
}

void CSeniorPage::SavePass()
{
	_password1Control.GetText(Delete->Info->Password);
	if (IsZipFormat())
	{
		if (!IsAsciiString(Delete->Info->Password))
		{
			ShowErrorMessageHwndRes(*this, IDS_PASSWORD_USE_ASCII, 0x02000B11);
			return;
		}
		UString method = GetEncryptionMethodSpec();
		method.MakeUpper();
		if (method.Find(L"AES") == 0)
		{
			if (Delete->Info->Password.Length() > 99)
			{
				_password1Control.SetText("");
				
				ShowErrorMessageHwndRes(*this, IDS_PASSWORD_IS_TOO_LONG, 0x02000B12);
				return;
			}
		}
	}
	if (!IsShowPasswordChecked())
	{
		UString password2;
		_password2Control.GetText(password2);
		if (password2 != Delete->Info->Password)
		{
			_password1Control.SetText("");
			_password2Control.SetText("");
			Delete->Info->Password=L"";
			ShowErrorMessageHwndRes(*this, IDS_PASSWORD_PASSWORDS_DO_NOT_MATCH, 0x02000B10);
			return;
		}
	}
	Delete->Info->EncryptionMethod = GetEncryptionMethodSpec();
	Delete->m_RegistryInfo.EncryptHeaders = Delete->Info->EncryptHeaders = IsButtonCheckedBool(IDC_SEN_PAGE_BOX_EDIT_TWO);


	Delete->m_RegistryInfo.ShowPassword = IsShowPasswordChecked();

	Delete->m_RegistryInfo.Save();
}

bool CSeniorPage::IsZipFormat()
{
	const CArcInfoEx &ai = (*Delete->ArcFormats)[Delete->FormatIndex];
	return (ai.Name.CompareNoCase(L"zip") == 0);
}

UString CSeniorPage::GetEncryptionMethodSpec()
{
	if (_encryptionMethod.GetCount() <= 1)
		return UString();
	if (_encryptionMethod.GetCurSel() <= 0)
		return UString();
	UString result;
	_encryptionMethod.GetText(result);
	result.Replace(L"-", L"");
	return result;
}

void CSeniorPage::SetEncryptionMethod()
{
	_encryptionMethod.ResetContent();
	const CArcInfoEx &ai = (*Delete->ArcFormats)[Delete->FormatIndex];
	if (ai.Name.CompareNoCase(L"7z") == 0)
	{
		_encryptionMethod.AddString(TEXT("AES-256"));
		_encryptionMethod.SetCurSel(0);
	}
	else if (ai.Name.CompareNoCase(L"zip") == 0)
	{
		int index = Delete->FindRegistryFormat(ai.Name);
		UString encryptionMethod;
		if (index >= 0)
		{
			const NCompression::CFormatOptions &fo = Delete->m_RegistryInfo.Formats[index];
			encryptionMethod = fo.EncryptionMethod;
		}
		_encryptionMethod.AddString(TEXT("ZipCrypto"));
		_encryptionMethod.AddString(TEXT("AES-256"));
		_encryptionMethod.SetCurSel(encryptionMethod.Find(L"AES") == 0 ? 1 : 0);
	}
}

int CSeniorPage::FindRegistryFormat(const UString &name)
{
	for (int i = 0; i < Delete->m_RegistryInfo.Formats.Size(); i++)
	{
		const NCompression::CFormatOptions &fo = Delete->m_RegistryInfo.Formats[i];
		if (name.CompareNoCase(GetUnicodeString(fo.FormatID)) == 0)
			return i;
	}
	return -1;
}


void CSeniorPage::CheckControlsEnable()
{
	bool b=false;
	if (Delete->FormatName == L"7z" || Delete->FormatName == L"Zip")b=true;

	EnableItem(IDC_SEN_PAGE_TEXT_ONE, b);
	EnableItem(IDC_SEN_PAGE_TEXT_TWO, b);
	EnableItem(IDC_SEN_PAGE_EDITTEXT_ONE,b);
	EnableItem(IDC_SEN_PAGE_EDITTEXT_TWO,b);
	EnableItem(IDC_SEN_PAGE_BOX_EDIT_ONE, b);
	EnableItem(IDC_SEN_PAGE_COMBO_ONE, b);
	EnableItem(IDC_SEN_PAGE_TEXT_THREE,b);
	if (Delete->FormatName == L"7z")b=true;
		else b =false;
	EnableItem(IDC_SEN_PAGE_BOX_EDIT_TWO,b);

}
bool CSeniorPage::OnCommand(int code, int itemID, LPARAM lParam)
{
	
	if (code == WM_TIMER)
	{
		OnTimer((WPARAM)itemID,lParam);
	}
	
	if (code == EN_KILLFOCUS)
	{
		switch(itemID)
		{
		case IDC_SEN_PAGE_EDITTEXT_ONE:
 			if (IsShowPasswordChecked())
 			{
				SavePass();
			}
			return true;
		case IDC_SEN_PAGE_EDITTEXT_TWO:
			SavePass();
			return true;
		}
		
	}
	return COptionPage::OnCommand(code, itemID, lParam);
}

bool CSeniorPage::OnTimer(WPARAM LM, LPARAM PM)
{
  
	if(!  IsWindowVisible(_window))
	{
		SetEncryptionMethod();
		
		CheckControlsEnable();
	}
	return COptionPage::OnTimer(LM,PM);
}
