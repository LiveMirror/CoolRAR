#include "StdAfx.h"
#include "GeneralPage.h"
#include "GeneralPageRes.h"

#include "../FileManager/BrowseDialog.h"
#include "../FileManager/FormatUtils.h"
#include "Common/IntToString.h"
#include "Common/StringConvert.h"
#include "DeletePageRes.h"
#include "../FileManager/HttpUpdat.h"
#include "../FileManager/LangUtils.h"

extern HttpUpdat httpworkdat;

using namespace NWindows;  

static LPCWSTR kExeExt = L".exe";
static LPCWSTR k7zFormat = L"7z";
static const int kHistorySize = 20;
static CIDLangPair kIDLangPairs[] = 
{
	{ IDD_PAGE_GENREAL, 0x04000501 },
	{ IDC_GEN_PAGE_TEXT_FILE, 0x04000502 },
	{ IDC_GEN_PAGE_BUTTON_SEARCH, 0x04000503 },
	{ IDC_GEN_PAGE_GROUP_PRESSSTYLE, 0x04000504 },
	{ IDC_GEN_PAGE_TEXT_UPDATESTYLE, 0x04000505 },
	{ IDC_GEN_PAGE_TEXT_PRESSBYTE, 0x04000506 },
	{ IDC_GEN_PAGE_TEXT_PRESSLEVEL,0x04000507 },
	{ IDC_GEN_PAGE_GROUP_EDIT, 0x04000508 },
	{ IDC_GEN_PAGE_BOX_EDIT_ONE, 0x04000509 },
	{ IDC_GEN_PAGE_BOX_EDIT_TWO, 0x04000510 },
	{ IDC_GEN_PAGE_BOX_EDIT_THREE, 0x04000511 },
	{ IDC_GEN_PAGE_TEXT_7ZGS, 0x04000512 },
	{ IDC_GEN_PAGE_BOX_EDIT_FOUR, 0x04000513 }
};

#define MY_SIZE_OF_ARRAY(x) (sizeof(x) / sizeof(x[0]))
enum EMethodID
{
	kCopy,
	kLZMA,
	kLZMA2,
	kPPMd,
	kBZip2,
	kDeflate,
	kDeflate64
};

struct CFormatInfo
{
	LPCWSTR Name;
	UInt32 LevelsMask;
	const EMethodID *MathodIDs;
	int NumMethods;
	bool Filter;
	bool Solid;
	bool MultiThread;
	bool SFX;
	bool Encrypt;
	bool EncryptFileNames;
};
static const LPCWSTR kMethodsNames[] =
{
	L"Copy",
	L"LZMA",
	L"LZMA2",
	L"PPMd",
	L"BZip2",
	L"Deflate",
	L"Deflate64"
};

static const EMethodID g_7zMethods[] =
{
	kLZMA,
	kLZMA2,
	kPPMd,
	kBZip2
};

static const EMethodID g_7zSfxMethods[] =
{
	kCopy,
	kLZMA,
	kLZMA2,
	kPPMd
};

static EMethodID g_ZipMethods[] =
{
	kDeflate,
	kDeflate64,
	kBZip2,
	kLZMA
};

static EMethodID g_GZipMethods[] =
{
	kDeflate
};

static EMethodID g_BZip2Methods[] =
{
	kBZip2
};

static EMethodID g_XzMethods[] =
{
	kLZMA2
};

#define METHODS_PAIR(x) x, MY_SIZE_OF_ARRAY(x)

static const CFormatInfo g_Formats[] =
{
	{
		L"",
			(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
			0, 0,
			false, false, false, false, false, false
	},
	{
		k7zFormat,
			(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
			METHODS_PAIR(g_7zMethods),
			true, true, true, true, true, true
		},
		{
			L"Zip",
				(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
				METHODS_PAIR(g_ZipMethods),
				false, false, true, false, true, false
		},
		{
			L"GZip",
				(1 << 1) | (1 << 5) | (1 << 7) | (1 << 9),
				METHODS_PAIR(g_GZipMethods),
				false, false, false, false, false, false
			},
			{
				L"BZip2",
					(1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
					METHODS_PAIR(g_BZip2Methods),
					false, false, true, false, false, false
			},
			{
				L"xz",
					(1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
					METHODS_PAIR(g_XzMethods),
					false, false, true, false, false, false
				},
				{
					L"Tar",
						(1 << 0),
						0, 0,
						false, false, false, false, false, false
				}
};

bool CGeneralPage::OnInit()
{
	LangSetWindowText(HWND(*this), 0x04000501);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	EnableItem(IDC_GEN_PAGE_COMBO_FILE,false);
	EnableItem(IDC_GEN_PAGE_BUTTON_SEARCH,false);
	Delete->DeleteVisible=false;
	Delete->Info->deleteSourceFile =false;
	Delete->GetHwndFromGeneral(GetItem(IDC_GEN_PAGE_COMBO_PRESSLEVEL));
	m_UpdateMode.Attach(GetItem(IDC_GEN_PAGE_COMBO_UPDATESTYLE));//更新
	m_Volume.Attach(GetItem(IDC_GEN_PAGE_COMBO_PRESSBYTE));//分卷大小

	Delete->m_RegistryInfo.Load();
	AddVolumeItems(m_Volume);
	SetVolumeSize();
	Delete->Info->FormatIndex = -1;
	int i;
	int j=0;
	for (i = 0; i < Delete->ArcIndices.Size(); i++)
	{

		int arcIndex = Delete->ArcIndices[i];
		const CArcInfoEx &ai = (*Delete->ArcFormats)[arcIndex];
		if(ai.Name == L"7z" || ai.Name == L"tar" || ai.Name == L"zip"  )
		{
			
			FormatIdex formatIdex;
			formatIdex.index = j++;
			formatIdex.arcIndex = arcIndex;
			formatIdexVer.Add(formatIdex);
			if (ai.Name.CompareNoCase(Delete->m_RegistryInfo.ArcType) == 0 || i == 0)
			{
				SetFormat(i);
				Delete->Info->FormatIndex = arcIndex;
				
			}
		}

		
	
	}

	SetArchiveName(Delete->Info->ArchiveName);

	for (i = 0; i < Delete->m_RegistryInfo.ArcPaths.Size() && i < kHistorySize; i++)
		m_ArchivePath.AddString(Delete->m_RegistryInfo.ArcPaths[i]);

	m_UpdateMode.AddString(L"添加并替换文件");
	m_UpdateMode.AddString(L"更新并添加文件");
	m_UpdateMode.AddString(L"只刷新已存在的文件");
	m_UpdateMode.AddString(L"同步压缩包内容");
	m_UpdateMode.SetCurSel(0);
	Delete->Info->UpdateMode = NOptinsAddDlg::NUpdateMode::EEnum(m_UpdateMode.GetCurSel());

	CheckButton(IDC_GEN_PAGE_BOX_EDIT_FOUR, Delete->Info->SFXMode);
	CheckButton(IDC_GEN_PAGE_BOX_EDIT_FIVE, Delete->Info->OpenShareForWrite);


	CheckControlsEnable();
	
	SaveInfo();
	Delete->OnInit();
	
	Delete->SetLevel();

	
	return COptionPage::OnInit();
}

void CGeneralPage::CheckControlsEnable()
{

	CheckSOLIDControlsEnable();
	CheckSFXControlsEnable();
}

void CGeneralPage::CheckSFXControlsEnable()
{
	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	bool enable = fi.SFX;
	if (!enable)
		CheckButton(IDC_GEN_PAGE_BOX_EDIT_FOUR, false);
	EnableItem(IDC_GEN_PAGE_BOX_EDIT_FOUR, enable);
}


void CGeneralPage::CheckSOLIDControlsEnable()
{
	const CFormatInfo &fi = g_Formats[GetStaticFormatIndex()];
	Delete->Info->SolidIsSpecified = fi.Solid;

	bool enable = fi.Solid;

	if (!enable)
		CheckButton(IDC_GEN_PAGE_BOX_EDIT_FIVE, false);
	EnableItem(IDC_GEN_PAGE_BOX_EDIT_FIVE, enable);

}

int CGeneralPage::GetStaticFormatIndex()
{
	int formatIndex = GetFormatIndex(GetFormatSur());
	const CArcInfoEx &ai = (*Delete->ArcFormats)[formatIndex];
	for (int i = 0; i < MY_SIZE_OF_ARRAY(g_Formats); i++)
		if (ai.Name.CompareNoCase(g_Formats[i].Name) == 0)
			return i;

	return 0; // -1;
}

int CGeneralPage::GetFormatSur()
{
	int item;

	if ( IsButtonCheckedBool(IDC_GEN_PAGE_BOX_PRESSSTYLE_ONE))
	{
		item = 0;
		Delete->FormatName = L"7z";
	}
	else if( IsButtonCheckedBool(IDC_GEN_PAGE_BOX_PRESSSTYLE_TWO))
	{
		item = 1;
		Delete->FormatName = L"Tar";
	}
	else if( IsButtonCheckedBool(IDC_GEN_PAGE_BOX_PRESSSTYLE_THREE))
	{
		item = 2;
		Delete->FormatName = L"Zip";
	}

	return item;
}

int CGeneralPage::GetFormatIndex(int iformat)
{
	for (int i=0;i<formatIdexVer.Size();i++)
	{
		if(formatIdexVer[i].index == iformat)
		{
			Delete->FormatIndex =formatIdexVer[i].arcIndex;
			return formatIdexVer[i].arcIndex;
		}
	}
	return -1;
}

void CGeneralPage::SetArchiveName(const UString &name)
{
	
	UString fileName = name;
	Delete->Info->FormatIndex = GetFormatIndex(GetFormatSur());
	const CArcInfoEx &ai = (*Delete->ArcFormats)[Delete->Info->FormatIndex];
	m_PrevFormat = Delete->Info->FormatIndex;
	if (ai.KeepName)
	{
		fileName = *Delete->OriginalFileName;
	}
	else
	{
		if (!Delete->Info->KeepName)
		{
			int dotPos = fileName.ReverseFind('.');
			int slashPos = MyMax(fileName.ReverseFind(WCHAR_PATH_SEPARATOR), fileName.ReverseFind('/'));
			if (dotPos >= 0 && dotPos > slashPos + 1)
				fileName = fileName.Left(dotPos);
		}
	}

	if (IsSFX())
		fileName += kExeExt;
	else
	{
		fileName += L'.';
		fileName += ai.GetMainExt();
	}
	m_ArchivePath.SetText(fileName);
}

bool CGeneralPage::IsSFX()
{
	CWindow sfxButton = GetItem(IDC_GEN_PAGE_BOX_EDIT_FOUR);
	Delete->Issfx = sfxButton.IsEnabled() && IsButtonCheckedBool(IDC_GEN_PAGE_BOX_EDIT_FOUR);
	return Delete->Issfx;
}

bool CGeneralPage::SetFormat(int i)
{
	switch(i)
	{
	case 0:
		CheckRadioButton(IDC_GEN_PAGE_BOX_PRESSSTYLE_ONE,IDC_GEN_PAGE_BOX_PRESSSTYLE_THREE,IDC_GEN_PAGE_BOX_PRESSSTYLE_ONE);
		return true;
	case 1:
		CheckRadioButton(IDC_GEN_PAGE_BOX_PRESSSTYLE_ONE,IDC_GEN_PAGE_BOX_PRESSSTYLE_THREE,IDC_GEN_PAGE_BOX_PRESSSTYLE_TWO);
		return true;
	case 2:
		CheckRadioButton(IDC_GEN_PAGE_BOX_PRESSSTYLE_ONE,IDC_GEN_PAGE_BOX_PRESSSTYLE_THREE,IDC_GEN_PAGE_BOX_PRESSSTYLE_THREE);
		return true;
	}
	return false;
}


void CGeneralPage::SetVolumeSize()
{
	UString volumeString;
	m_Volume.GetText(volumeString);
	volumeString.Trim();
	Delete->Info->VolumeSizes.Clear();
	if (!volumeString.IsEmpty())
	{
		if (!ParseVolumeSizes(volumeString, Delete->Info->VolumeSizes))
		{

			MessageBoxW(*this,LangString(0x07000009),LangString(0x07000010),NULL);
			m_Volume.SetText(L"");
			SetVolumeSize();
			return;
		}
		if (!Delete->Info->VolumeSizes.IsEmpty())
		{
			const UInt64 volumeSize = Delete->Info->VolumeSizes.Back();
			if (volumeSize < (100 << 10))
			{
				wchar_t s[32];
				ConvertUInt64ToString(volumeSize, s);
				if (::MessageBoxW(*this, MyFormatNew(IDS_COMPRESS_SPLIT_CONFIRM_MESSAGE, 0x02000D42, s),
					L"CoolRAR", MB_YESNOCANCEL | MB_ICONQUESTION) != IDYES)
				{
					m_Volume.SetText(L"");
					SetVolumeSize();
					return;
				}
			}
		}
	}
}


bool CGeneralPage::OnCommand(int code, int itemID, LPARAM lParam)
{
	
	if (code == CBN_SELCHANGE)
	{
		switch(itemID)
		{
		case IDC_GEN_PAGE_COMBO_UPDATESTYLE:
			{
				Delete->Info->UpdateMode = NOptinsAddDlg::NUpdateMode::EEnum(m_UpdateMode.GetCurSel());
				SaveInfo();
				return true;
			}
		case IDC_GEN_PAGE_COMBO_PRESSBYTE:
			{
				SetVolumeSize();
				SaveInfo();
				return true;

			}
		}
	}
	else if (code == CBN_KILLFOCUS)
	{
		switch(itemID)
		{
		case IDC_GEN_PAGE_COMBO_PRESSBYTE:
			{
				SaveInfo();
				SetVolumeSize();
				return true;

			}
		case IDC_GEN_PAGE_COMBO_FILE:
			{

				SaveInfo();
				SetVolumeSize();
				return true;
			}

		}
	}
	switch(itemID)
	{
	case IDC_GEN_PAGE_COMBO_PRESSLEVEL:
		{
			Delete->SettingPressLevel();
			return true;
		}
	}

	return COptionPage::OnCommand(code, itemID, lParam);
}


bool CGeneralPage::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_GEN_PAGE_BUTTON_SEARCH:
		{
			OnButtonSetArchive();
			SaveInfo();
			return true;
		}
	case IDC_GEN_PAGE_BOX_PRESSSTYLE_ONE:
	case IDC_GEN_PAGE_BOX_PRESSSTYLE_TWO:
	case IDC_GEN_PAGE_BOX_PRESSSTYLE_THREE:
		{
			bool isSFX = IsSFX();
			SaveOptionsInMem();
			Delete->FormatIndex = GetFormatIndex(GetFormatSur());
			Delete->SetLevel();
			Delete->SetMethod();
			Delete->SetSolidBlockSize();
			Delete->SetNumThreads();
			Delete->CheckControlsEnable();
			Delete->SetMemoryUsage();
			CheckControlsEnable();
			SetArchiveName2(isSFX);
			SaveInfo();
			if(Delete->DeleteVisible)Delete->SaveInfo();
			return true;

		}
	case IDC_GEN_PAGE_BOX_EDIT_FOUR:
		{
			OnButtonSFX();
			SaveInfo();
			return true;
		}
	case IDC_GEN_PAGE_BOX_EDIT_ONE:
		{
			Delete->Info->deleteSourceFile = IsButtonCheckedBool(IDC_GEN_PAGE_BOX_EDIT_ONE);
			SaveInfo();
			return true;
		}
	
	}
	return COptionPage::OnButtonClicked(buttonID, buttonHWND);
}


void CGeneralPage::OnButtonSFX()
{
	UString fileName;
	m_ArchivePath.GetText(fileName);
	int dotPos = fileName.ReverseFind(L'.');
	int slashPos = fileName.ReverseFind(WCHAR_PATH_SEPARATOR);
	if (dotPos < 0 || dotPos <= slashPos)
		dotPos = -1;
	if (IsSFX())
	{
		if (dotPos >= 0)
			fileName = fileName.Left(dotPos);
		fileName += kExeExt;
		m_ArchivePath.SetText(fileName);
	}
	else
	{
		if (dotPos >= 0)
		{
			UString ext = fileName.Mid(dotPos);
			if (ext.CompareNoCase(kExeExt) == 0)
			{
				fileName = fileName.Left(dotPos);
				m_ArchivePath.SetText(fileName);
			}
		}
		SetArchiveName2(false); // it's for OnInit
	}
	CheckVolumeEnable();
}
void CGeneralPage::CheckVolumeEnable()
{
	bool isSFX = IsSFX();
	m_Volume.Enable(!isSFX);
	if (isSFX)
		m_Volume.SetText(TEXT(""));
}

void CGeneralPage::SetArchiveName2(bool prevWasSFX)
{
	UString fileName;
	m_ArchivePath.GetText(fileName);
	const CArcInfoEx &prevArchiverInfo = (*Delete->ArcFormats)[m_PrevFormat];
	if (prevArchiverInfo.KeepName || Delete->Info->KeepName)
	{
		UString prevExtension = prevArchiverInfo.GetMainExt();
		if (prevWasSFX)
			prevExtension = kExeExt;
		else
			prevExtension = UString('.') + prevExtension;
		const int prevExtensionLen = prevExtension.Length();
		if (fileName.Length() >= prevExtensionLen)
			if (fileName.Right(prevExtensionLen).CompareNoCase(prevExtension) == 0)
				fileName = fileName.Left(fileName.Length() - prevExtensionLen);
	}
	SetArchiveName(fileName);
}

void CGeneralPage::OnButtonSetArchive()
{
	UString fileName;
	m_ArchivePath.GetText(fileName);
	fileName.Trim();
	Delete->Info->ArchiveName = fileName;
	UString fullFileName;
	if (!Delete->Info->GetFullPathName(fullFileName))
	{
		fullFileName = Delete->Info->ArchiveName;
		return;
	}
	UString title =L"浏览";
	UString s =L"所有文件";
	s += L" (*.*)";
	UString resPath;
	if (!MyBrowseForFile(HWND(*this), title, fullFileName, s, resPath))
		return;
	m_ArchivePath.SetText(resPath);
}

void CGeneralPage::SaveOptionsInMem()
{
	if(Delete->DeleteVisible)Delete->SaveOptionsInMem();
}

int CGeneralPage::FindRegistryFormatAlways(const UString &name)
{
	int index = FindRegistryFormat(name);
	if (index < 0)
	{
		NCompression::CFormatOptions fo;
		fo.FormatID = GetSystemString(name);
		index = Delete->m_RegistryInfo.Formats.Add(fo);
	}
	return index;
}

int CGeneralPage::FindRegistryFormat(const UString &name)
{
	for (int i = 0; i < Delete->m_RegistryInfo.Formats.Size(); i++)
	{
		const NCompression::CFormatOptions &fo = Delete->m_RegistryInfo.Formats[i];
		if (name.CompareNoCase(GetUnicodeString(fo.FormatID)) == 0)
			return i;
	}
	return -1;
}

void AddUniqueString(UStringVector &list, const UString &s)
{
	for (int i = 0; i < list.Size(); i++)
		if (s.CompareNoCase(list[i]) == 0)
			return;
	list.Add(s);
}


void CGeneralPage ::SaveInfo()
{
	SaveOptionsInMem();
	UString s;
	m_ArchivePath.GetText(s);
	s.Trim();
	Delete->m_RegistryInfo.ArcPaths.Clear();
	AddUniqueString(Delete->m_RegistryInfo.ArcPaths, s);
	Delete->Info->ArchiveName = s;
	Delete->Info->UpdateMode = NOptinsAddDlg::NUpdateMode::EEnum(m_UpdateMode.GetCurSel());

	Delete->Info->FormatIndex = GetFormatIndex(GetFormatSur());
	Delete->Info->SFXMode = IsSFX();
	Delete->Info->deleteSourceFile = IsButtonCheckedBool(IDC_GEN_PAGE_BOX_EDIT_ONE);
	Delete->Info->OpenShareForWrite = IsButtonCheckedBool(IDC_GEN_PAGE_BOX_EDIT_THREE);
	
	SetVolumeSize();

	for (int i = 0; i < m_ArchivePath.GetCount(); i++)
	{
		UString sTemp;
		m_ArchivePath.GetLBText(i, sTemp);
		sTemp.Trim();
		AddUniqueString(Delete->m_RegistryInfo.ArcPaths, sTemp);
	}

	if (Delete->m_RegistryInfo.ArcPaths.Size() > kHistorySize)
		Delete->m_RegistryInfo.ArcPaths.DeleteBack();

	Delete->m_RegistryInfo.ArcType = (*Delete->ArcFormats)[Delete->Info->FormatIndex].Name;
	Delete->m_RegistryInfo.Save();

}

