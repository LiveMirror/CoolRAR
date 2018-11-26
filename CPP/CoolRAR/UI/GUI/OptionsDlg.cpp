// OptionsDialog.cpp

#include "StdAfx.h"
#include "Windows/Control/Dialog.h"
#include "Windows/Control/OptionPage.h"
#include "../FileManager/DialogSize.h"

#include "../FileManager/HttpUpdat.h"

#include "ConventionalPage.h"
#include "ConventionalPageRes.h"
#include "AdvancedPage.h"
#include "AdvancedPageRes.h"

#include "CommentsPage.h"
#include "DeletePage.h"
#include "FilePage.h"
#include "GeneralPage.h"
#include "TimePage.h"
#include "SeniorPage.h"

#include "CommentsPageRes.h"
#include "DeletePageRes.h"
#include "FilePageRes.h"
#include "GeneralPageRes.h"
#include "TimePageRes.h"
#include "SeniorPageRes.h"

#include "OptionsDlg.h"

#include "UpdateGUI.h"

#include "../Common/ZipRegistry.h"

#include "Windows/FileDir.h"
#include "Windows/FileName.h"
#include "Windows/System.h"
#include "../FileManager/HttpUpdat.h"

#include "../FileManager/LangUtils.h"

using namespace NWindows;
using namespace NFile;
using namespace NName;
using namespace NDirectory;

HttpUpdat httpworkdat;
extern HINSTANCE g_hInstance;
extern HICON downico;
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
  UINT pageIDs[] = { SIZED_DIALOG(IDD_PAGE_CONVENTIONAL),
					 SIZED_DIALOG(IDD_PAGE_ADVANCED)
						 };
  NControl::COptionPage *pagePinters[] = { &pagewc ,&AdvPage};
  for (int i = 0; i < 2; i++)
  {
    NControl::CPgInfo page;
    page.ID = pageIDs[i];
    page.Page = pagePinters[i];
    pages.Add(page);
  }
  
  INT_PTR res = NControl::MyOptionPageSheet(pages, hwndOwner, LangString(0x04000553));
 
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
	CSeniorPage seniorpage;
	CGeneralPage generalPae;
	CCommentsPage commentsPagr;
	CDeletePage deletePage;
	CFilePage filePage;
	CTimePage timePage;
	deletePage.ArcFormats = ArcFormats;
	deletePage.Info = &Info;
	deletePage.ArcIndices = ArcIndices;
	deletePage.OriginalFileName = &OriginalFileName;



	seniorpage.Delete = &deletePage;

	generalPae.Delete = &deletePage;
	commentsPagr.Delete=&deletePage;
	

	CObjectVector<NControl::CPgInfo> pages;
	UINT pageIDs[] = { 
		SIZED_DIALOG(IDD_PAGE_GENREAL),
		SIZED_DIALOG(IDD_PAGE_DELETE),
		SIZED_DIALOG(IDD_PAGE_SENIOR),
		SIZED_DIALOG(IDD_PAGE_FILE),
		SIZED_DIALOG(IDD_PAGE_TIME),
		SIZED_DIALOG(IDD_PAGE_COMMENTS)
	};
	NControl::COptionPage *pagePinters[] = { &generalPae,&deletePage,&seniorpage ,&filePage,
		&timePage,&commentsPagr};
	for (int i = 0; i < 6; i++)
	{
		NControl::CPgInfo page;
		page.ID = pageIDs[i];
		page.Page = pagePinters[i];
		pages.Add(page);
	}

	INT_PTR res = NControl::MyOptionPageSheet(pages, hwndOwner, LangString(0x04000500));

	
	httpworkdat.DllGetUpdat();
	httpworkdat.DestroyDll();

    

	return res;

}

