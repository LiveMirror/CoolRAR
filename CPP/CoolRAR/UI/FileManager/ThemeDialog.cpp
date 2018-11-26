#include "StdAfx.h"
#include "ThemeDialog.h"
#include "HelpUtils.h"
#include "resource.h"
#include "App.h"
#include "Panel.h"
#include "Windows/CommonDialog.h"
#include "Common/MyString.h"
#include "Windows/FileIO.h"
#include "Windows/Control/ImageList.h"
#include "Windows/FileDir.h"
#include "LangUtils.h"
#include "Windows/Registry.h"


using namespace NWindows;
using namespace NRegistry;

extern CApp g_App;
extern HINSTANCE g_hInstance;
static LPCWSTR kThemeHelpTopic = L"dialog/index.htm";
static LPCWSTR ThemeIdentifies = L"CoolRAR_theme_description.txt";//主题文件标识.

static NSynchronization::CCriticalSection g_CS;
static const TCHAR *kCuPrefix = TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR") TEXT(STRING_PATH_SEPARATOR);
static const TCHAR *kKeyName = TEXT("Theme");
static const WCHAR *kThemeName = L"ThemeName";

static CIDLangPair kIDLangPairs[] =
{
	{ IDC_THEME_USED_TEXT                     , 0x04000437 },
	{ IDC_BUTTON_SEACHE_THEME                 , 0x04000438 },
	{ IDC_BUTTON_ADD_THEME                    , 0x04000439 },
	{ IDC_BUTTON_DELETE_THEME                 , 0x04000440 },
	{ IDC_BUTTON_ENSURE_THEME                 , 0x05000001 },
	{ IDC_BUTTON_HELP_THEME                   , 0x05000003 },
	{ IDC_CHECK_THEME_USE                     , 0x04000441 },
};

CThemeTitle ThemeTitle;
static CSysString GetKeyPath(const CSysString &path) { return kCuPrefix + path; }

static LONG OpenMainKey(CKey &key, LPCTSTR keyName)
{
	return key.Open(HKEY_CURRENT_USER, GetKeyPath(keyName), KEY_READ);
}

static LONG CreateMainKey(CKey &key, LPCTSTR keyName)
{
	return key.Create(HKEY_CURRENT_USER, GetKeyPath(keyName));
}

bool CThemeDialog::OnInit()
{	
	LangSetWindowText(HWND(*this), 0x04000442);
	LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
	ThemeTitle.Read();
	Themelistview.DeleteAllItems();
	g_App.GetTheme(ThemeStr);
	g_App.GetThemeFolderName(ThemeFolderName);
	ListViewOnit();
	return CModalDialog::OnInit();
}

void CThemeDialog::OnOK()
{
	CModalDialog::OnOK();
}
bool CThemeDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
	switch(buttonID)
	{
	case IDC_BUTTON_ENSURE_THEME:
		OnOK();
		break;
	case IDC_BUTTON_HELP_THEME:
		ShowHelpWindow(NULL, kThemeHelpTopic);
		break;
	case IDC_BUTTON_ADD_THEME:
		AddTheme();
		break;
	case IDC_BUTTON_DELETE_THEME:
		DeleteTheme();
		break;
	case IDC_BUTTON_SEACHE_THEME:
		LoadTheme();
		break;
	default:
		g_App.AppState.ThemeTitle.Save();
		break;

	}
	return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
}
void CThemeDialog::AddTheme()
{	
	UString ThemeName;
	UString fileName=L"";
	UString title = L"选择主题文件";
	WCHAR s[MAX_PATH]=L"*.theme.zip\0*.theme.zip";
	UString resPath;
	if (AllOpenFileName(HWND(*this),title,fileName,s,resPath)!=S_OK)
	{

		UString CoolRARFolder=L"\\CoolRAR";
        UString ThemeszPath = GetAppDataPath();
		ThemeszPath +=CoolRARFolder;
		::CreateDirectoryW(ThemeszPath.GetBuffer(ThemeszPath.Length()),NULL);
	    ThemeFolder=ThemeszPath+L"\\Themes";
		::CreateDirectoryW(ThemeFolder.GetBuffer(ThemeFolder.Length()),NULL);
		ThemeszPath.ReleaseBuffer();
		ThemeFolder.ReleaseBuffer();
		
		UString TempThemeFile,ThemeTemp,ThemeFileName;

		int pos = resPath.ReverseFind(WCHAR_PATH_SEPARATOR);
		TempThemeFile = resPath.Right(resPath.Length()-pos-1);//主题压缩包名及后缀名
		int nPostmp=TempThemeFile.ReverseFind(L'.');
		ThemeTemp = TempThemeFile.Left(nPostmp);
		int nPos=ThemeTemp.ReverseFind(L'.');
		ThemeFileName = ThemeTemp.Left(nPos);//压缩包名。
	    ThemesBagName = ThemeFileName;
	    ThemeBagPath = ThemeFolder+WSTRING_PATH_SEPARATOR+ThemesBagName;//主题文件夹路径

		UStringVector ArcThemePath;
		ArcThemePath.Add(resPath);
		if(::MessageBoxW(NULL,LangString(0x07000033)+TempThemeFile+LangString(0x07000034),LangString(0x07000035),MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			g_App.ExtractThemes(ArcThemePath,ThemeBagPath);

			::MessageBoxW(NULL,LangString(0x07000036),LangString(0x07000035),MB_ICONINFORMATION);
			Sleep(100);

			ThemeName=ReadFile(ThemeBagPath+WSTRING_PATH_SEPARATOR+ThemeIdentifies);//标识文件路径.
			if (ThemeName ==L"")
			{	
				return;
			}
			else
			{
				g_App.GetTheme(ThemeStr);
				g_App.GetThemeFolderName(ThemeFolderName);

				g_App.SetTheme(ThemeName);
				g_App.SetThemeFolderName(ThemesBagName);
				ThemeTitle.Save();
				if (ThemeStr.Find(ThemeName)<0)
				{
					Themelistview.InsertItem(ThemeStr.Size()+1,ThemeName);
				}
			}
		}
	}
}
void CThemeDialog::ListViewOnit()
{
	Themelistview.Attach(GetItem(IDC_EDITTEXT_THEME));
	const UString &item=L"默认主题";
	LV_COLUMNW column;
	column.mask = LVCF_WIDTH | LVCF_TEXT ;
	column.cx = 185;
	column.pszText = L"";
	Themelistview.InsertColumn(1, &column);
	
	Themelistview.DeleteAllItems();
	g_App.GetTheme(ThemeStr);
	if (ThemeStr.Find(item) ==-1)
	{
		ThemeStr.Insert(0,item);
		g_App.SetTheme(ThemeStr);
		g_App.GetTheme(ThemeStr);
	}	
	if (ThemeStr.Size() == 0)
	{
		Themelistview.SetItemState_FocusedSelected(0);
		return;
	}
	for(int i =0; i <ThemeStr.Size(); i++)
	{
		Themelistview.InsertItem(i,ThemeStr.operator [](i));
	}
	UString themeValues;
	themeValues=ReadThemeReg();
	if (themeValues !=L"")
	{
		int Fouse = ThemeFolderName.Find(themeValues);
		Themelistview.SetItemState_FocusedSelected( Fouse+1);
	}
	else
	{
		Themelistview.SetItemState_FocusedSelected(0);
	}
}
void CThemeDialog::DeleteTheme()
{
	UString ListImagePath;
	int ThemeFouse=Themelistview.GetFocusedItem();
	if (ThemeFouse>0)
	{	
		ThemeTitle.Read();
		g_App.GetThemeFolderName(ThemeFolderName);
		UString deleteFolderName = ThemeFolderName.operator [](ThemeFouse-1);
		UString RegThemeValue = ReadThemeReg();

		if (deleteFolderName == RegThemeValue)
		{
			int Fouse=0;
			RegThemeValue=L"";
			CKey key;
			CreateMainKey(key,kKeyName);
			key.SetValue(kThemeName,RegThemeValue);
			g_App.ThemeDefault();
			g_App.UpdateThemeIcon(ListImagePath,Fouse);
			g_App.UpdateStausIco(ListImagePath,Fouse);
		}

		UString deleteFolderPath,tempPath;

		tempPath=GetAppDataPath();
		tempPath+=L"\\CoolRAR\\Themes\\";

		deleteFolderPath = tempPath + deleteFolderName;
		g_App.MyDeleteFile(deleteFolderPath,true,GetActiveWindow(),false);
		if(!ThemeStr.IsEmpty())
		{
			ThemeStr.Delete(ThemeFouse);
			ThemeFolderName.Delete(ThemeFouse-1);

		}
		g_App.SetTheme(ThemeStr);
		g_App.SetThemeFolderName(ThemeFolderName);
		ThemeTitle.Save();
		Themelistview.DeleteItem(Themelistview.GetFocusedItem());
	}
	else 
	{
		return;
	}
}
UString CThemeDialog::ReadFile(UString ThemeIdenPath)
{
	UString themeName;
	NWindows::NFile::NIO::CInFile inFileTxt;
	inFileTxt.Open(ThemeIdenPath);
	UInt32 processedSize;
	
	char buf[MAX_PATH*2] = {0};
	inFileTxt.Read(buf, MAX_PATH*2,processedSize);
	
	themeName = (wchar_t*)buf;
	return themeName;
}
void CThemeDialog::LoadTheme()
{
	UString ThemesPath,BtnBmpPath,themeValue;
	UString ListImagePath;
	ThemesPath = GetAppDataPath()+WSTRING_PATH_SEPARATOR+L"CoolRAR"+WSTRING_PATH_SEPARATOR+L"Themes";

	int ThemeFouse=Themelistview.GetFocusedItem();
	ThemeTitle.Read();
	if (ThemeFouse <0)
	{
		return;
	}
	if (ThemeFouse==0)
	{
		g_App.ThemeDefault();
		g_App.UpdateThemeIcon(ListImagePath,ThemeFouse);
		g_App.UpdateStausIco(ListImagePath,ThemeFouse);
		themeValue = L"";
	}
	else
	{
		g_App.GetThemeFolderName(ThemeFolderName);
		themeValue=ThemeFolderName.operator [](ThemeFouse-1);
		BtnBmpPath = ThemesPath+WSTRING_PATH_SEPARATOR+themeValue+WSTRING_PATH_SEPARATOR+L"toolbar";
		ListImagePath = ThemesPath+WSTRING_PATH_SEPARATOR+themeValue;
		g_App.SaveThemeBmp( BtnBmpPath );
		g_App.UpdateThemeIcon(ListImagePath,ThemeFouse);
		g_App.UpdateStausIco(ListImagePath,ThemeFouse);
	}
	CKey key;
	CreateMainKey(key,kKeyName);
	key.SetValue(kThemeName,themeValue);
}

UString CThemeDialog::ReadThemeReg()
{	
	UString ThemeKeyValue;
	CKey key;
	if (OpenMainKey(key,kKeyName) == ERROR_SUCCESS)
	{
		if (key.QueryValue(kThemeName,ThemesBagName) == ERROR_SUCCESS)
		{
			if (ThemesBagName == L"")
			{
				ThemeKeyValue=L"";
			}
			else
			{
				ThemeKeyValue=ThemesBagName;
			}
		}
		else
		{
			ThemeKeyValue=L"";
		}
	}
	else
	{
		ThemeKeyValue = L"";
	}
	return ThemeKeyValue;
}
UString CThemeDialog::GetAppDataPath()
{
	UString Pathtemp;
	WCHAR szPath[MAX_PATH];
	UString ThemesPath,BtnBmpPath,themeValue;
	if(SUCCEEDED(SHGetFolderPathW(NULL, 
		CSIDL_APPDATA |CSIDL_FLAG_CREATE, 
		NULL, 
		0, 
		szPath))) 
	{
		 Pathtemp = szPath;
	}
	return Pathtemp;
}
void CThemeDialog::MeunTheme(int Fouse)
{
	UString ThemesPath,BtnBmpPath,themeValue;
	UString ListImagePath;
	ThemesPath = GetAppDataPath()+WSTRING_PATH_SEPARATOR+L"CoolRAR"+WSTRING_PATH_SEPARATOR+L"Themes";

	//int ThemeFouse=Themelistview.GetFocusedItem();
	ThemeTitle.Read();
	if (Fouse <0)
	{
		return;
	}
	if (Fouse==0)
	{
		g_App.ThemeDefault();
		g_App.UpdateThemeIcon(ListImagePath,Fouse);
		g_App.UpdateStausIco(ListImagePath,Fouse);
		themeValue = L"";
	}
	else
	{
		g_App.GetThemeFolderName(ThemeFolderName);
		themeValue=ThemeFolderName.operator [](Fouse-1);
		BtnBmpPath = ThemesPath+WSTRING_PATH_SEPARATOR+themeValue+WSTRING_PATH_SEPARATOR+L"toolbar";
		ListImagePath = ThemesPath+WSTRING_PATH_SEPARATOR+themeValue;
		g_App.SaveThemeBmp( BtnBmpPath );
		g_App.UpdateThemeIcon(ListImagePath,Fouse);
		g_App.UpdateStausIco(ListImagePath,Fouse);
	}
	CKey key;
	CreateMainKey(key,kKeyName);
	key.SetValue(kThemeName,themeValue);
}
