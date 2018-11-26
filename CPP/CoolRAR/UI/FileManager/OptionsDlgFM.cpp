// OptionsDialog.cpp

#include "StdAfx.h"
#include "Windows/Control/Dialog.h"
#include "Windows/Control/optionPage.h"
#include "../FileManager/DialogSize.h"
#include "../FileManager/HttpUpdat.h"

#include "DeletePage.h"
#include "FilePage.h"
#include "GeneralPage.h"
#include "TimePage.h"

#include "DeletePageRes.h"
#include "FilePageRes.h"
#include "GeneralPageRes.h"
#include "TimePageRes.h"
#include "FMAdvancedPage.h"
#include "FMConventionalPage.h"
#include "FMconventionalpageres.h"
#include "FMAdvancedPageRes.h"
#include "LangUtils.h"
#include "../Common/ZipRegistry.h"

#include "Windows/FileDir.h"
#include "Windows/FileName.h"
#include "Windows/System.h"

using namespace NWindows;
using namespace NFile;
using namespace NName;
using namespace NDirectory;
extern HttpUpdat httpworkdat;
namespace NOptinsAddDlg
{
	bool CInfo::GetFullPathName(UString &result) const
	{
#ifndef UNDER_CE
		NDirectory::MySetCurrentDirectory(CurrentDirPrefix);
#endif
		return MyGetFullPathName(ArchiveName, result);
	}
}

INT_PTR OptionsDialog(HWND hwndOwner, HINSTANCE /* hInstance */,CConventionalPage pagewc,
					  CAdvancedPage AdvPage,
					  NExtract::NPathMode::EEnum      &PathMode,
					  NExtract::NOverwriteMode::EEnum &OverwriteMode,
					  UString &Path)
{
	CObjectVector<NControl::CPgInfo> pages;
	UINT pageIDs[] = { SIZED_DIALOG(IDD_PAGE_FMCONVENTIONAL),
		SIZED_DIALOG(IDD_PAGE_FMADVANCED)
	};
	NControl::COptionPage *pagePinters[] = { &pagewc ,&AdvPage};
	for (int i = 0; i < 2; i++)
	{
		NControl::CPgInfo page;
		page.ID = pageIDs[i];
		page.Page = pagePinters[i];
		pages.Add(page);
	}

	INT_PTR res = NControl::MyOptionPageSheet(pages, hwndOwner, L"解压路径和选项");

	OverwriteMode =pagewc.OverwriteMode;
	Path = pagewc.DirectoryPath;
	PathMode     = AdvPage.PathMode;
	NExtract::CInfo info;
	info.OverwriteMode = OverwriteMode;
	info.PathMode = PathMode;
	info.Save();
	httpworkdat.DllGetUpdat();
	httpworkdat.DestroyDll();


	return res;
}
INT_PTR OptionsDlg::OptionsAddDialog(HWND hwndOwner, HINSTANCE /* hInstane */)
{
	CGeneralPage generalPae;
	CDeletePage deletePage;
	CFilePage filePage;
	CTimePage timePage;
	deletePage.ArcFormats = ArcFormats;
	deletePage.Info = &Info;
	deletePage.ArcIndices = ArcIndices;
	deletePage.OriginalFileName = &OriginalFileName;




	generalPae.Delete = &deletePage;


	CObjectVector<NControl::CPgInfo> pages;
	UINT pageIDs[] = { 
		SIZED_DIALOG(IDD_PAGE_GENREAL),
		SIZED_DIALOG(IDD_PAGE_DELETE),
		SIZED_DIALOG(IDD_PAGE_FILE),
		SIZED_DIALOG(IDD_PAGE_TIME)
	};
	NControl::COptionPage *pagePinters[] = { &generalPae,&deletePage,&filePage,
		&timePage};
	for (int i = 0; i < 4; i++)
	{
		NControl::CPgInfo page;
		page.ID = pageIDs[i];
		page.Page = pagePinters[i];
		pages.Add(page);
	}

	INT_PTR res = NControl::MyOptionPageSheet(pages, hwndOwner, L"压缩文件名和参数");


	httpworkdat.DllGetUpdat();
	httpworkdat.DestroyDll();



	return res;

}


