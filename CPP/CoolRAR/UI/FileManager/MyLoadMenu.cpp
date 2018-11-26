// MyLoadMenu.cpp

#include "StdAfx.h"

#include "Windows/Menu.h"
#include "Windows/Control/Dialog.h"

#include "../../PropID.h"

#include "../Common/CompressCall.h"
#include "../Common/ZipRegistry.h"

#include "AboutDialog.h"
#include "App.h"
#include "HelpUtils.h"
#include "LangUtils.h"
#include "RegistryUtils.h"
#include "../Explorer/ContextMenuFlags.h"
#include "../Explorer/RegistryContextMenu.h"
#include "../Explorer/resource.h"
#include "ProgramLocation.h"
#include "resource.h"
#include "HttpUpdat.h"
#include "ThemeDialog.h"

using namespace NWindows;
using namespace NFile;
using namespace NFind;
using namespace NContextMenuFlags;

static const UINT kOpenBookmarkMenuID = 730;
static const UINT kSetBookmarkMenuID = 740;
static const UINT kFileLoadHistoryMenuID = 750;
static const int kDirDriverMenuID = 760;
static const UINT kSetThemeID=770;


extern HINSTANCE g_hInstance;
static LPCWSTR kFMHelpTopic = L"dialog/index.htm";
static LPCTSTR kHomePageURL = "http://www.CoolRAR.com/";
static LPCTSTR kFaceBookURL = "http://bbs.coolrar.com";
static LPCTSTR kThanksURL   = "http://bbs.coolrar.com/feedback/1";
static LPCTSTR kThemeURL    = "http://www.coolrar.com/download.html";

extern void OptionsDialog(HWND hwndOwner, HINSTANCE hInstance);

enum
{
  kMenuIndex_File = 0,
  kMenuIndex_Edit,
  kMenuIndex_View,
  kMenuIndex_Bookmarks,
  kMenuIndex_Options
};

static const UInt32 kTopMenuLangIDs[] =
{
  0x03000102,
  0x03000103,
  0x03000105,
  0x03000107,
  0x03000104,
  0x03000106
};

static const UInt32 kAddToFavoritesLangID = 0x03000710;
static const UInt32 kToolbarsLangID = 0x03000451;
static const UInt32 kChangeDirverLangID = 0x04000212;
static const UInt32 kInOrOutPanelEditLangID = 0x04000341;
static const UInt32 kFileListEditLangID = 0x04000342;
static const UInt32 kFileFloderTree = 0x04000344;
static const UInt32 kThemeLangID = 0x04000345;

static const CIDLangPair kIDLangPairs[] =
{
  // File
  { IDM_FILE_OPEN, 0x04000210 },
  { IDM_SAVEFILECOPY, 0x04000211 },
  { IDM_PASSWORDSET, 0x04000220 },
  { IDM_COPY, 0x04000221 },
  { IDM_COPYTO, 0x04000230 },
  { IDM_SELECT_ALL, 0x04000233 },
  { IDM_SELECTROW, 0x04000231 },
  { IDM_DESELECTROW, 0x04000232 },
  { IDM_INVERT_SELECTION, 0x04000332 },
  { IDCLOSE, 0x03000260 },

  // commed
  { IDM_ADDFILE, 0x04000320 },
  { IDM_TOFLODER, 0x04000321 },
  { IDM_TEST, 0x04000322 },

 
  { IDM_CHECKFILE, 0x04000323 },
  { IDM_DELETEFILE, 0x04000324 },
  { IDM_SETFILENAME, 0x04000325 },
  { IDM_PRINTFILE, 0x04000326 },
  { IDM_UNPRESSDIRECT, 0x04000327 },
  { IDM_ADDANNOTATION, 0x04000328 },

  //tools
  { IDM_GUIDE, 0x04000329 },
  { IDM_SEARCHVIRUS, 0x04000330 },
  { IDM_FORMCHANGE, 0x04000331 },
  { IDM_REPAIRFILE, 0x04000332 },
  { IDM_UNPRESSITSELF, 0x04000333 },
  { IDM_SEARCHFILE, 0x04000334 },
  { IDM_GETINFORMATION, 0x04000335 },
  { IDM_FILE_SPLIT, 0x04000336 },
  { IDM_FILE_COMBINE, 0x04000337 },
  { IDM_GETMD5INFO, 0x04000338 },
  { IDM_HARDTEST, 0x04000339 },

  //favorites
  { IDM_FAVORITESFLODER,0x04000357},
  { IDM_ARRANGEFAVORITE,0x04000358},

  //edit
  { IDM_SETUP, 0x04000340 },
  { IDM_NOTCENT, 0x04000443 },
  { IDM_MANAGE_THEME, 0x04000347 },
  { IDM_ACHIEVE_THEME, 0x04000348 },
  { IDM_CHECKDAILYA, 0x04000349 },
  { IDM_DELETEDAILY, 0x04000350 },
	
  //help
  { IDM_HELP_CONTENTS, 0x04000351 },
  { IDM_HELP_ADVANCE, 0x04000352 },
  { IDM_HELP_CHECKUPDATE, 0x04000353 },
  { IDM_HELP_THANKS, 0x04000354 },
  { IDM_HELP_HOMEPAGE, 0x04000355 },
  { IDM_ABOUT, 0x04000356 }
};

struct CContextMenuItem
{
	int ControlID;
	UInt32 LangID;
	UInt32 Flag;
};
static CContextMenuItem kMenuItems[] =
{
	{ IDS_CONTEXT_OPEN, 0x02000103, kOpen},
	{ IDS_CONTEXT_EXTRACT, 0x02000105, kExtract},
	{ IDS_CONTEXT_EXTRACT_HERE, 0x0200010B, kExtractHere },
	{ IDS_CONTEXT_EXTRACT_TO, 0x0200010D, kExtractTo },

	{ IDS_CONTEXT_TEST, 0x02000109, kTest},

	{ IDS_CONTEXT_MORE_EXTRACT_HERE, 0x02000115, kMoreExtractHere },
	{ IDS_CONTEXT_MORE_EXTRACT_TO, 0x02000116, kMoreExtractTo },

	{ IDS_CONTEXT_COMPRESS, 0x02000107, kCompress },
	{ IDS_CONTEXT_COMPRESS_TO, 0x0200010F, kCompressTo7z },
	{ IDS_CONTEXT_COMPRESS_TO, 0x0200010F, kCompressToZip }

#ifndef UNDER_CE
	,
	{ IDS_CONTEXT_COMPRESS_EMAIL, 0x02000111, kCompressEmail },
	{ IDS_CONTEXT_COMPRESS_TO_EMAIL, 0x02000113, kCompressTo7zEmail },
	{ IDS_CONTEXT_COMPRESS_TO_EMAIL, 0x02000113, kCompressToZipEmail }
#endif
};

const int kNumMenuItems = sizeof(kMenuItems) / sizeof(kMenuItems[0]);

static int FindLangItem(int ControlID)
{
  for (int i = 0; i < sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]); i++)
    if (kIDLangPairs[i].ControlID == ControlID)
      return i;
  return -1;
}

static int GetSortControlID(PROPID propID)
{
  switch(propID)
  {
    case kpidName: return IDM_VIEW_ARANGE_BY_NAME;
    case kpidExtension: return IDM_VIEW_ARANGE_BY_TYPE;
    case kpidMTime: return IDM_VIEW_ARANGE_BY_DATE;
    case kpidSize: return IDM_VIEW_ARANGE_BY_SIZE;
    case kpidNoProperty: return IDM_VIEW_ARANGE_NO_SORT;
  }
  return -1;
}



static inline UINT Get_fMaskForString() { return MIIM_TYPE; }
static inline UINT Get_fMaskForFTypeAndString() { return MIIM_TYPE; }

static void MyChangeMenu(HMENU menuLoc, int level, int menuIndex)
{
  CMenu menu;
  menu.Attach(menuLoc);
  for (int i = 0;; i++)
  {
    CMenuItem item;
    item.fMask = Get_fMaskForString() | MIIM_SUBMENU | MIIM_ID;
    item.fType = MFT_STRING;
    if (!menu.GetItem(i, true, item))
      break;
    {
      UString newString;
      if (item.hSubMenu)
      {
        UInt32 langID = 0;
        if (level == 1 && menuIndex == kMenuIndex_File)
          langID = kChangeDirverLangID;
		else if(level == 1 && menuIndex == kMenuIndex_Options && i == 1)
		  langID =kInOrOutPanelEditLangID;
		else if(level == 1 && menuIndex == kMenuIndex_Options && i == 3)
			langID =kFileListEditLangID;	
		else if(level == 1 && menuIndex == kMenuIndex_Options && i == 4)
			langID =kFileFloderTree;
		else if(level == 1 && menuIndex == kMenuIndex_Options && i == 5)
			langID =kThemeLangID;
        else
        {
          MyChangeMenu(item.hSubMenu, level + 1, i);
          if (level == 0 && i < sizeof(kTopMenuLangIDs) / sizeof(kTopMenuLangIDs[0]))
            langID = kTopMenuLangIDs[i];
          else
            continue;
        }
        newString = LangString(langID);
        if (newString.IsEmpty())
          continue;
      }
      else
      {
        int langPos = FindLangItem(item.wID);
        if (langPos < 0)
          continue;
        newString = LangString(kIDLangPairs[langPos].LangID);
        if (newString.IsEmpty())
          continue;
        UString shorcutString = item.StringValue;
        int tabPos = shorcutString.ReverseFind(wchar_t('\t'));
        if (tabPos >= 0)
          newString += shorcutString.Mid(tabPos);
      }
      {
        item.StringValue = newString;
        item.fMask = Get_fMaskForString();
        item.fType = MFT_STRING;
        menu.SetItem(i, true, item);
      }
    }
  }
}

static CMenu g_FileMenu;

struct CFileMenuDestroyer
{
  ~CFileMenuDestroyer() { if ((HMENU)g_FileMenu != 0) g_FileMenu.Destroy(); }
} g_FileMenuDestroyer;


static void CopyMenu(HMENU srcMenuSpec, HMENU destMenuSpec)
{
  CMenu srcMenu;
  srcMenu.Attach(srcMenuSpec);
  CMenu destMenu;
  destMenu.Attach(destMenuSpec);
  int startPos = 0;
  for (int i = 0;; i++)
  {
    CMenuItem item;
    item.fMask = MIIM_STATE | MIIM_ID | Get_fMaskForFTypeAndString();
    item.fType = MFT_STRING;
    if (srcMenu.GetItem(i, true, item))
    {
      if (destMenu.InsertItem(startPos, true, item))
        startPos++;
    }
    else
      break;
  }
}

void GetDriverRealName(UString &s,int number)
{

	GetDriveInfo(s);

}
void GetDriverName(UString &s,int number )
{
	UStringVector driveStrings;
	MyGetLogicalDriveStrings(driveStrings);

	s = driveStrings[number];	
	

}
int GetDriverNumber()
{
	UStringVector driveStrings;
	MyGetLogicalDriveStrings(driveStrings);
	return driveStrings.Size();
}

void MyLoadMenu()
{
  HMENU baseMenu;

  #ifdef UNDER_CE

  HMENU oldMenu = g_App._commandBar.GetMenu(0);
  if (oldMenu)
    ::DestroyMenu(oldMenu);
  BOOL b = g_App._commandBar.InsertMenubar(g_hInstance, IDM_MENU, 0);
  baseMenu = g_App._commandBar.GetMenu(0);
  if (!g_LangID.IsEmpty())
    MyChangeMenu(baseMenu, 0, 0);
  g_App._commandBar.DrawMenuBar(0);
 
  #else

  HWND hWnd = g_HWND;
  HMENU oldMenu = ::GetMenu(hWnd);
  ::SetMenu(hWnd, ::LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_MENU)));
  ::DestroyMenu(oldMenu);
  baseMenu = ::GetMenu(hWnd);
  if (!g_LangID.IsEmpty())
    MyChangeMenu(baseMenu, 0, 0);
  ::DrawMenuBar(hWnd);

  #endif

  if ((HMENU)g_FileMenu != 0)
    g_FileMenu.Destroy();
    g_FileMenu.CreatePopup();

	HMENU FILEMENU;
	FILEMENU =::LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_FILEMENU));
	g_FileMenu.Attach(FILEMENU);

	
 
}


void OnMenuActivating(HWND /* hWnd */, HMENU hMenu, int position)
{
  HMENU mainMenu =
    #ifdef UNDER_CE
    g_App._commandBar.GetMenu(0);
    #else
    ::GetMenu(g_HWND)
    #endif
    ;
 if (::GetSubMenu(mainMenu, position) != hMenu)
  return;


  if (position == kMenuIndex_File)
  {
    CMenu menu;
    menu.Attach(hMenu);
	//如果当前选定的文件不是压缩文件则保存压缩文件副本按钮隐藏
	if ( g_App.PressFileJudge()&& !g_App.ChangeToolBarState())
	{
		::EnableMenuItem(menu,IDM_SAVEFILECOPY,MF_ENABLED);
	}
	else
	{
		::EnableMenuItem(menu,IDM_SAVEFILECOPY, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	} 
	CMenu subMenu;
	//将子菜单链接到第三项
	subMenu.Attach(menu.GetSubMenu(2));
	subMenu.RemoveAllItems();
	//写入子项内容
	int i,number;
	number =GetDriverNumber();
	for (i = 0; i <number; i++)
	{
		
		
		UString s;
		GetDriverName(s,i);//获取驱动器盘符名称

		DWORD attrib = FILE_ATTRIBUTE_DIRECTORY;
		int iconIndex ;//系统图标
		GetRealIconIndex(s, attrib,iconIndex);

		HBITMAP hbitmap;



		hbitmap = ::LoadBitmap(NULL, MAKEINTRESOURCE(iconIndex ));

	

		GetDriverRealName(s,i);//传入盘符名称以及序号，获取完整磁盘名。
		subMenu.AppendItem(MF_STRING, kDirDriverMenuID + i, s);

		
		::SetMenuItemBitmaps(subMenu,i,MF_BYPOSITION,hbitmap,hbitmap);

	}
	
    menu.RemoveAllItemsFrom(15);


	
	//逐条判断是否为压缩文件
	int judge= 0;
	//judge用来记录在历史浏览记录中当前所选的条目数
	UString r,s;
	{
	for(int i= 1;i< 5;i++)
	{
		
		
	     //获取历史浏览记录
 		 g_App.GetOldFileName(judge,r);

		 if( !r.IsEmpty() )
		 {
			 s += (wchar_t)(L'0' + i);
			 s +=L"   ";
			 s += r;
			 s += L"....";	 	  
			 //输出到历史浏览目录	
			 menu.AppendItem(MF_STRING, kFileLoadHistoryMenuID + i , s);
			 r.Empty();
			 s.Empty();
			
		 }
		 else
		 {
			 r.Empty();
			 s.Empty();
		 }

	   
    }
	}
	
  }
 
  else if (position == kMenuIndex_Edit)
  {
	  CMenu menu;
	  menu.Attach(hMenu);
	  //如果当前路径不是压缩文件则隐藏
	  if ( !g_App.GetFocusedPanel().NowAcrhvieJudge())//如果当前路径不是压缩文件路径
	  {
		  ::EnableMenuItem(menu,IDM_ADDFILE,MF_ENABLED);
	  }
	  else
	  {
		  ::EnableMenuItem(menu,IDM_ADDFILE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  } 
	  if(g_App.GetFocusedPanel().NowPathJudge())
	  {
		::EnableMenuItem(menu,IDM_ADDANNOTATION, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  }
	  else
	  {
		::EnableMenuItem(menu,IDM_ADDANNOTATION, MF_ENABLED);
	  }
	  if (g_App.GetFocusedPanel().NowPathJudge() && g_App.GetFocusedPanel().NowAcrhvieJudge())
	  {
		  ::EnableMenuItem(menu,IDM_DELETEFILE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
		  ::EnableMenuItem(menu,IDM_SETFILENAME, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
		  ::EnableMenuItem(menu,IDM_PROTECTFILE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
		  ::EnableMenuItem(menu,IDM_LOCKFILE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  }
	  else
	  {
		  ::EnableMenuItem(menu,IDM_DELETEFILE,MF_ENABLED);
		  ::EnableMenuItem(menu,IDM_SETFILENAME,MF_ENABLED);
		  ::EnableMenuItem(menu,IDM_PROTECTFILE,MF_ENABLED);
		  ::EnableMenuItem(menu,IDM_LOCKFILE,MF_ENABLED);
	  }

  }
  else if(position == kMenuIndex_View)
  {
	  CMenu menu;
	  menu.Attach(hMenu);
	  //如果当前路径不是压缩文件则按钮隐藏
	  if ( !g_App.GetFocusedPanel().NowPathJudge() )//如果当前路径不是压缩文件路径
	  {
		  ::EnableMenuItem(menu,IDM_UNPRESSITSELF,MF_ENABLED);
		  ::EnableMenuItem(menu,IDM_FILE_SPLIT, MF_ENABLED);
		  ::EnableMenuItem(menu,IDM_FILE_COMBINE, MF_ENABLED);

	  }
	  else
	  {
		  ::EnableMenuItem(menu,IDM_FILE_SPLIT, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
		  ::EnableMenuItem(menu,IDM_FILE_COMBINE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
		  if (g_App.GetFocusedPanel().IsExeFile())//如果在自解压压缩文件内则禁用转化为自解压选项
		  {
			::EnableMenuItem(menu,IDM_UNPRESSITSELF, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
		  }  
	  } 
  }

  else if (position == kMenuIndex_Options)
  {
 

	  CMenu menu;
	  menu.Attach(hMenu);

	  CMenu subMenu;
	  //将导入导出子菜单链接到第二项
	  subMenu.Attach(menu.GetSubMenu(1));
	  subMenu.RemoveAllItems();
	  UString inout;
	  inout =LangString(0x04000359);
	  subMenu.AppendItem(MF_STRING, IDM_INOUT_FILEIN, inout);
	  inout.Empty();
	  inout =LangString(0x04000360);
	  subMenu.AppendItem(MF_STRING, IDM_INOUT_FILEOUT, inout);

	   
	  CMenu subMenufile;
	  //将文件列表子菜单链接到第三项
	  subMenufile.Attach(menu.GetSubMenu(3));
	  //判断是否可选中分层文件夹按钮
	  if ( g_App.CentJudge() )
	  {
		  ::EnableMenuItem(subMenufile,IDM_NOTCENT,MF_ENABLED);
	  }
	  else
	  {
		  ::EnableMenuItem(subMenufile,IDM_NOTCENT, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  }
	   
	  UString filelist;
	  UString fileinfo;
	  filelist =LangString(0x04000361);
	  fileinfo =LangString(0x04000362);
	  if (fileinfo.IsEmpty())
	  {
		  filelist =L"列表查看(&W)";
		  fileinfo =L"详细资料(&L)";
	  }
	  
	  subMenufile.AppendItem(MF_STRING, IDM_FILELIST_INFO, fileinfo);
	  subMenufile.AppendItem(MF_STRING, IDM_FILELIST_LIST, filelist);

	  subMenufile.CheckItemByID(IDM_FILELIST_LIST, g_App.FileListJudge());
	  subMenufile.CheckItemByID(IDM_FILELIST_INFO, !g_App.FileListJudge());
	  subMenufile.RemoveAllItemsFrom(4);
	  
	  CMenu subMenutree;
	  //将文件夹树子菜单链接到第四项
	  subMenutree.Attach(menu.GetSubMenu(4));
	  subMenutree.RemoveAllItems();
	  UString driveFloder;
	  UString PressFloder;
	  driveFloder =LangString(0x04000363);
	  PressFloder =LangString(0x04000364);
	  if(PressFloder.IsEmpty())
	  {
		  driveFloder =L"显示磁盘文件夹(&D)\tCtrl+T";
		  PressFloder =L"显示压缩文件夹(&A)\tCtrl+T";
	  }
	  subMenutree.AppendItem(MF_STRING, IDM_FLODERLIST_DRIVE, driveFloder);
	  subMenutree.AppendItem(MF_STRING, IDM_FLODERLIST_PRESS, PressFloder);

	  //加载选项的选中状态
	  subMenutree.CheckItemByID(IDM_FLODERLIST_PRESS, g_App.FileTreeJudge());
	  subMenutree.CheckItemByID(IDM_FLODERLIST_DRIVE, g_App.FileTreeJudge());
	

	  //加入主题包至子菜单中。
	  CMenu subMenuTheme;
	  subMenuTheme.Attach(menu.GetSubMenu(5));
	  subMenuTheme.RemoveAllItemsFrom(3);
	  int maxcount = g_App.AppState.ThemeTitle.GetStringSize();
	  for (int i = 0; i < maxcount; i++)
	  {
		  
		  UString path = g_App.AppState.ThemeTitle.GetString(i);
		  subMenuTheme.AppendItem(MF_STRING, kSetThemeID + i, path);
	  }
	
	  subMenuTheme.CheckItemByID(kSetThemeID+g_App.ThemeIsSelect(),true);

	  //加载所有选项菜单的项目
 	  menu.RemoveAllItemsFrom(9);

  }
  else if (position == kMenuIndex_Bookmarks)
  {
    CMenu menu;
    menu.Attach(hMenu);
	menu.RemoveAllItemsFrom(2);
	int maxcount = g_App.AppState.FastFolders.GetStringSize(); //得到收藏夹内文件名数量
    for (int i = 0; i < maxcount; i++)
    {
		
      UString path = g_App.AppState.FastFolders.GetString(i);//得到收藏夹内文件完整路径
	  UString sign;
	  g_App.AppState.FastFolders.GetSign(path,sign);		//得到收藏夹内当前文件注释
	  if(sign.IsEmpty())
	  {
		  const int kMaxSize = 100;
		  const int kFirstPartSize = kMaxSize / 2;
		  if (path.Length() > kMaxSize)
		  {
			  path = path.Left(kFirstPartSize) + UString(L" ... ") +
				  path.Right(kMaxSize - kFirstPartSize);
		  }
		  UString s;
		  s = L"&";
		  s+= (wchar_t)(L'1' + i);
		  s+= L"   ";
		  s+= path;
		  menu.AppendItem(MF_STRING, kOpenBookmarkMenuID + i, s);
	  }
	  else
	  {
		  UString s;

		  s = L"&";
		  s+= (wchar_t)(L'1' + i);
		  s+= L"   ";
		  s+=sign;
		 menu.AppendItem(MF_STRING, kOpenBookmarkMenuID + i, s);
	  }
    }
  }
}


void LoadDriveMenu(HMENU hMenu)
{


	//改变驱动器子菜单
	CMenu subMenuDriveName;
	subMenuDriveName.Attach(hMenu);
	subMenuDriveName.RemoveAllItems();
	//写入子项内容
	int i,number;
	number =GetDriverNumber();
	for (i = 0; i <number; i++)
	{
		UString s;
		GetDriverName(s,i);//获取驱动器盘符名称
		GetDriverRealName(s,i);//传入盘符名称以及序号，获取完整磁盘名。
		subMenuDriveName.AppendItem(MF_STRING, kDirDriverMenuID + i, s);
	}

}


void LoadFileMenu(HMENU hMenu, int startPos, bool programMenu,
    bool isFsFolder, int numItems, bool allAreFiles)
{
	HMENU FILEMENU;
	FILEMENU =::LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_FILEMENU));
	g_FileMenu.Attach(FILEMENU);
	CMenu destMenu;
  destMenu.Attach(hMenu);

  UString diffPath;
  ReadRegDiff(diffPath);
  
  int numRealItems = startPos;
  for (int i = 0;; i++)
  {
    CMenuItem item;
	item.fMask = MIIM_STATE | MIIM_ID | Get_fMaskForFTypeAndString()|MIIM_SUBMENU;
    item.fType = MFT_STRING;
    if (!g_FileMenu.GetItem(i, true, item))
      break;
    {
      if (!programMenu && item.wID == IDCLOSE)
        continue;

      if (item.wID == IDM_FILE_DIFF && diffPath.IsEmpty())
        continue;

      bool isOneFsFile = (isFsFolder && numItems == 1 && allAreFiles);
      bool disable = ((item.wID == IDM_FILE_SPLIT || item.wID == IDM_FILE_COMBINE) && !isOneFsFile);

      bool isBigScreen = NControl::IsDialogSizeOK(40, 200);

      if (!isBigScreen && (disable || item.IsSeparator()))
        continue;
		
      if (destMenu.InsertItem(startPos, true, item))
        startPos++;
	
      if (disable)
        destMenu.EnableItem(startPos - 1, MF_BYPOSITION | MF_GRAYED);

      if (!item.IsSeparator())
        numRealItems = startPos;
    }
  }
	
  //开始加载查看方式子菜单
  CMenu subMenufile;
  //将文件列表子菜单链接到第三项
  subMenufile.Attach(destMenu.GetSubMenu(16));
  subMenufile.RemoveAllItems();
  //判断是否可选中分层文件夹按钮
  UString filelist;
  UString fileinfo;
  filelist =LangString(0x04000361);
  fileinfo =LangString(0x04000362);
  if (fileinfo.IsEmpty())
  {
	  filelist =L"列表查看(&W)";
	  fileinfo =L"详细资料(&L)";
  }
  subMenufile.AppendItem(MF_STRING, IDM_FILELIST_INFO, fileinfo);
  subMenufile.AppendItem(MF_STRING, IDM_FILELIST_LIST, filelist);
  subMenufile.CheckItemByID(IDM_FILELIST_LIST, g_App.FileListJudge());
  subMenufile.CheckItemByID(IDM_FILELIST_INFO, !g_App.FileListJudge());
  subMenufile.RemoveAllItemsFrom(2);
	
  //加入文件夹排列方式子菜单
  CMenu subMenulistmode;
  subMenulistmode.Attach(destMenu.GetSubMenu(17));
  subMenulistmode.RemoveAllItems();
  UString listByname,listBysize,listBytype,listBytime,listBynoMode;
  listByname =LangString(0x04000365) + L"(&N)";
  listBysize =LangString(0x04000366) + L"(&S)";
  listBytype =LangString(0x04000367) + L"(&T)";
  listBytime =LangString(0x04000368) + L"(&M)";
  listBynoMode =LangString(0x04000369) + L"(&O)";
  subMenulistmode.AppendItem(MF_STRING, IDM_VIEW_ARANGE_BY_NAME, listByname);
  subMenulistmode.AppendItem(MF_STRING, IDM_VIEW_ARANGE_BY_SIZE, listBysize);
  subMenulistmode.AppendItem(MF_STRING, IDM_VIEW_ARANGE_BY_TYPE, listBytype);
  subMenulistmode.AppendItem(MF_STRING, IDM_VIEW_ARANGE_BY_DATE, listBytime);
  subMenulistmode.AppendItem(MF_STRING, IDM_VIEW_ARANGE_NO_SORT, listBynoMode);
  //判断当前选项是否选中，选中则在选项前显示勾
  subMenulistmode.CheckItemByID(IDM_VIEW_ARANGE_BY_NAME,g_App.FileListRuleJudge(kpidName));
  subMenulistmode.CheckItemByID(IDM_VIEW_ARANGE_BY_SIZE,g_App.FileListRuleJudge(kpidSize));
  subMenulistmode.CheckItemByID(IDM_VIEW_ARANGE_BY_TYPE,g_App.FileListRuleJudge(kpidAttrib));
  subMenulistmode.CheckItemByID(IDM_VIEW_ARANGE_BY_DATE,g_App.FileListRuleJudge(kpidMTime));
  subMenulistmode.CheckItemByID(IDM_VIEW_ARANGE_NO_SORT,g_App.FileListRuleJudge(kpidNoProperty));
  subMenulistmode.RemoveAllItemsFrom(5);
 

  //改变驱动器子菜单
  CMenu subMenuDriveName;
  //将子菜单链接到第三项
  subMenuDriveName.Attach(destMenu.GetSubMenu(18));
  subMenuDriveName.RemoveAllItems();
  subMenuDriveName.RemoveAllItems();
  //写入子项内容
  int number;
  number =GetDriverNumber();
  for (int j = 0; j <number; j++)
  {
	  UString s;
	  GetDriverName(s,j);//获取驱动器盘符名称
	  GetDriverRealName(s,j);//传入盘符名称以及序号，获取完整磁盘名。
	  subMenuDriveName.AppendItem(MF_STRING, kDirDriverMenuID + j, s);
  }

  CMenu menu;
  menu.Attach(destMenu);
  //如果当前路径不是压缩文件则隐藏
  if ( !g_App.GetFocusedPanel().NowAcrhvieJudge() )//如果当前路径不是压缩文件路径
  {
	  ::EnableMenuItem(menu,IDM_ADDFILE,MF_ENABLED);
  }
  else
  {
	  ::EnableMenuItem(menu,IDM_ADDFILE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
  } 

  if (g_App.GetFocusedPanel().NowPathJudge() && g_App.GetFocusedPanel().NowAcrhvieJudge())
  {
	  ::EnableMenuItem(menu,IDM_DELETEFILE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  ::EnableMenuItem(menu,IDM_SETFILENAME, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  ::EnableMenuItem(menu,IDM_ADDANNOTATION, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  ::EnableMenuItem(menu,IDM_PROTECTFILE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  ::EnableMenuItem(menu,IDM_LOCKFILE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
  }
  else
  {
	  ::EnableMenuItem(menu,IDM_DELETEFILE,MF_ENABLED);
	  ::EnableMenuItem(menu,IDM_SETFILENAME,MF_ENABLED);
	  ::EnableMenuItem(menu,IDM_ADDANNOTATION,MF_ENABLED);
	  ::EnableMenuItem(menu,IDM_PROTECTFILE,MF_ENABLED);
	  ::EnableMenuItem(menu,IDM_LOCKFILE,MF_ENABLED);
  }
  if ( !g_App.GetFocusedPanel().NowPathJudge() )//如果当前路径不是压缩文件路径
  {
	  ::EnableMenuItem(menu,IDM_FORMCHANGE,MF_ENABLED);
	  ::EnableMenuItem(menu,IDM_UNPRESSITSELF,MF_ENABLED);

  }
  else
  {
	  ::EnableMenuItem(menu,IDM_FORMCHANGE, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
	  ::EnableMenuItem(menu,IDM_UNPRESSITSELF, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
  } 
  	
  destMenu.RemoveAllItemsFrom(numRealItems);
   ::DestroyMenu(FILEMENU);
}

bool ContextMenuCommand(int id)
{
	switch(id)
	{
		case IDM_SELECT_ALL:
			g_App.SelectAll(true);
			g_App.RefreshStatusBar();
			break;
		case IDM_ADDFILE:
			g_App.AddFile();
			break;
		case IDM_TOFLODER:
			g_App.ToFlord();
			break;
		case IDM_TEST:
			g_App.Test();
			break;	
		case IDM_CHECKFILE:
			g_App.CheckFile();
			break;
		case IDM_DELETEFILE:
			g_App.MyDeleteFile();
			break;
		case IDM_REPAIRFILE:
			g_App.RepairFile();
			break;
		case IDM_UNPRESSDIRECT:
			g_App.UnpressDirect();
			break;
		case IDM_GETINFORMATION:
			g_App.GetInformation();
			break;
		case IDM_PASSWORDSET:
			g_App.PasswordSet();
			break;
		case IDM_FAVORITESFLODER:
			g_App.Favorites();
			break;
		case IDM_CREATE_FOLDER:
			g_App.CreateFolder();
			break;
		case IDM_SETFILENAME:
			g_App.SetFileName();
			break;
		case IDM_FILELIST_LIST:
		case IDM_FILELIST_INFO:
			int index;
			index =id -IDM_FILELIST_LIST;
			index =index +2;
			if(index < 4)
			{
				g_App.SetListViewMode(index);
			}		
			break;
		case IDM_VIEW_ARANGE_BY_NAME:
		{
			g_App.SortItemsWithPropID(kpidName);
			break;
		}
		case IDM_VIEW_ARANGE_BY_TYPE:
		{
			g_App.SortItemsWithPropID(kpidAttrib);
			break;
		}
		case IDM_VIEW_ARANGE_BY_DATE:
		{
			g_App.SortItemsWithPropID(kpidMTime);
			break;
		}
		case IDM_VIEW_ARANGE_BY_SIZE:
		{
			g_App.SortItemsWithPropID(kpidSize);
			break;
		}
		case IDM_VIEW_ARANGE_NO_SORT:
		{
			g_App.SortItemsWithPropID(kpidNoProperty);
			break;
		}
		break;

		default:
		if(id >= kDirDriverMenuID&& id < kDirDriverMenuID +GetDriverNumber() )
		{
			UString selectdrive;
			int driver;
			driver = id - kDirDriverMenuID;
			GetDriverName(selectdrive,driver);
			g_App.OpenSelectDrive(selectdrive);
			return true;
		}
		return false;
	}
    return true;
}
bool ExecuteFileCommand(int id)
{
	if (id >= kPluginMenuStartID)
	{
		g_App.GetFocusedPanel().InvokePluginCommand(id);
		g_App.GetFocusedPanel()._sevenZipContextMenu.Release();
		g_App.GetFocusedPanel()._systemContextMenu.Release();
		return true;
	}

	switch (id)
	{
		// File

	case IDM_FILE_OPEN:
		g_App.OpenItem();
		break;
	case IDM_SAVEFILECOPY:
		g_App.SaveFileCopy();
		break;
	case IDM_PASSWORDSET:
		g_App.PasswordSet();
		break;
	case IDM_COPY:
		g_App.CopyFiletoClipBoard();
		break;
	case IDM_COPYTO:
		g_App.MoveFromClipBoard();
		break;
	case IDM_SELECT_ALL:
		g_App.SelectAll(true);
		g_App.RefreshStatusBar();
		break;	
	case IDM_SELECTROW:
		g_App.SelectRow();
		break;
	case IDM_DESELECTROW:
		g_App.DeSelectRow();
		break;
	case IDM_INVERT_SELECTION:
		g_App.InvertSelection();
		g_App.RefreshStatusBar();
		break;
	default:
		return false;
	}
	return true;
}

bool OnMenuCommand(HWND hWnd, int id)
{
	if (ExecuteFileCommand(id))
		return true;

	switch (id)
	{
		// File
	case IDCLOSE:
		SendMessage(hWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWnd);
		SendMessage (hWnd, WM_CLOSE, 0, 0);
		break;

		// Command
	case IDM_ADDFILE:
		g_App.AddFile();
		break;
	case IDM_TOFLODER:
		g_App.ToFlord();
		break;
	case IDM_TEST:
		g_App.Test();
		break;
	case IDM_CHECKFILE_LAGEBUTTON:
		g_App.CheckFile();
		break;
	case IDM_CHECKFILE:
		g_App.CheckFile();
		break;
	case IDM_DELETEFILE_LAGEBUTTON:
		g_App.MyDeleteFile();
		break;
	case IDM_DELETEFILE:
		g_App.MyDeleteFile();
		break;
	case IDM_SETFILENAME:
		g_App.SetFileName();
		break;
	case IDM_PRINTFILE:
		g_App.PrintFile();
		break;
	case IDM_UNPRESSDIRECT:
		g_App.UnpressDirect();
		break;
	case IDM_ADDANNOTATION:
		g_App.AddAnnotation();
		break;

	case IDM_DESELECT_ALL:
		g_App.SelectAll(false);
		g_App.RefreshStatusBar();
		break;
	


		//View
	case IDM_SETUP:
		OptionsDialog(hWnd, g_hInstance);
		break;
	case IDM_INOUT_FILEIN:
		g_App.Filein();
		break;
	case IDM_INOUT_FILEOUT:
		g_App.FileOut();
		break;
	case IDM_FILELIST_LIST:
	case IDM_FILELIST_INFO:
		int index;
		index =id -IDM_FILELIST_LIST;
		index =index +2;
		if(index < 4)
		{
		   g_App.SetListViewMode(index);
		}
		break;
	case IDM_FLODERLIST_DRIVE:
    case IDM_FLODERLIST_PRESS:
		g_App.ChangTreeView();
		break;
	case IDM_MANAGE_THEME:
		g_App.ManageTheme();
		break;
	case IDM_ACHIEVE_THEME:
		::ShellExecute(NULL,"open",kThemeURL,NULL,NULL,SW_SHOWNORMAL);
		break;
	case IDM_CHECKDAILYA:
		g_App.CheckDaily();
		break;
	case IDM_DELETEDAILY:
		g_App.DeleteDaily();
		break;
	
	
	case IDM_VIEW_LARGE_ICONS:
	case IDM_VIEW_SMALL_ICONS:
	case IDM_VIEW_LIST:
	case IDM_VIEW_DETAILS:
		{
			UINT index = id - IDM_VIEW_LARGE_ICONS;
			if (index < 4)
			{
				g_App.SetListViewMode(index);
				
			}
			break;
		}
	case IDM_VIEW_ARANGE_BY_NAME:
		{
			g_App.SortItemsWithPropID(kpidName);
			break;
		}
	case IDM_VIEW_ARANGE_BY_TYPE:
		{
			g_App.SortItemsWithPropID(kpidExtension);
			break;
		}
	case IDM_VIEW_ARANGE_BY_DATE:
		{
			g_App.SortItemsWithPropID(kpidMTime);
			break;
		}
	case IDM_VIEW_ARANGE_BY_SIZE:
		{
			g_App.SortItemsWithPropID(kpidSize);
			break;
		}
	case IDM_VIEW_ARANGE_NO_SORT:
		{
			g_App.SortItemsWithPropID(kpidNoProperty);
			break;
		}

	case IDM_OPEN_ROOT_FOLDER:
		g_App.OpenRootFolder();
		break;
	case IDM_OPEN_PARENT_FOLDER:
		g_App.OpenParentFolder();
		break;
	case IDM_FOLDERS_HISTORY:
		g_App.FoldersHistory();
		break;
	case IDM_VIEW_REFRESH:
		g_App.RefreshView();
		break;
	case IDM_VIEW_FLAT_VIEW:
		g_App.ChangeFlatMode();
		break;
	case IDM_VIEW_TWO_PANELS:
		g_App.SwitchOnOffOnePanel();
		break;
	case IDM_VIEW_STANDARD_TOOLBAR:
		g_App.SwitchStandardToolbar();
		break;
	case IDM_VIEW_ARCHIVE_TOOLBAR:
		g_App.SwitchArchiveToolbar();
		break;
	case IDM_VIEW_TOOLBARS_SHOW_BUTTONS_TEXT:
		g_App.SwitchButtonsLables();
		break;
	case IDM_VIEW_TOOLBARS_LARGE_BUTTONS:
		g_App.SwitchLargeButtons();
		break;
	

		// Tools
	case IDM_GUIDE_LAGEBUTTON:
		g_App.Guide();
		break;
	case IDM_GUIDE:
		g_App.Guide();
		break;
	case IDM_SEARCHVIRUS:
		g_App.SearchVirus();
		break;
	case IDM_FORMCHANGE:
		g_App.FormChange();
		break;
	case IDM_REPAIRFILE_LAGEBUTTON:
		g_App.RepairFile();
		break;
	case IDM_REPAIRFILE:
		g_App.RepairFile();
		break;
	case IDM_UNPRESSITSELF:
		g_App.UnPressItself();
		break;
	case IDM_SEARCHFILE_LAGEBUTTON:
		g_App.SearchFilename();
		break;
	case IDM_SEARCHFILE:
		g_App.SearchFilename();
		break;
	case IDM_GETINFORMATION_LAGEBUTTON:
		g_App.GetInformation();
		break;
	case IDM_GETINFORMATION:
		g_App.GetInformation();
		break;
	case IDM_FILE_SPLIT:
		g_App.Split();
		break;
	case IDM_FILE_COMBINE:
		g_App.Combine();
		break;
	case IDM_GETMD5INFO:
		g_App.GetMD5();
		break;

	case IDM_HARDTEST:
		{
			CPanel::CDisableTimerProcessing disableTimerProcessing1(g_App.Panels[0]);
			CPanel::CDisableTimerProcessing disableTimerProcessing2(g_App.Panels[1]);
			Benchmark();
			break;
		}
		// Help
	case IDM_HELP_CONTENTS:
		ShowHelpWindow(NULL, kFMHelpTopic);
		break;
	case IDM_HELP_ADVANCE:
		::ShellExecute(NULL, "open", kFaceBookURL, NULL, NULL, SW_SHOWNORMAL);
		break;
	case IDM_HELP_CHECKUPDATE:
		g_App.CheckUpdateInfo();
		break;
	case IDM_HELP_THANKS:
		::ShellExecute(NULL,"open",kThanksURL,NULL,NULL,SW_SHOWNORMAL);
		break;
	case IDM_HELP_HOMEPAGE:
		::ShellExecute(NULL,"open",kHomePageURL,NULL,NULL,SW_SHOWNORMAL);
		break;
		//收藏夹
	case IDM_FAVORITESFLODER:
		g_App.Favorites();
		break;
	case IDM_ARRANGEFAVORITE:
		g_App.ArrangeFavorite();
		break;
	case IDM_ABOUT:
		{
			CAboutDialog dialog;
			dialog.Create(hWnd);
			break;
		}
	default:
		{	
			if(id >= kDirDriverMenuID&& id < kDirDriverMenuID +GetDriverNumber() )
			{
				UString selectdrive;
				int driver;
				driver = id - kDirDriverMenuID;
				GetDriverName(selectdrive,driver);
				g_App.OpenSelectDrive(selectdrive);
				return true;
			}
			if(id >kFileLoadHistoryMenuID&& id <=kFileLoadHistoryMenuID +4)
			{
				int selectindex =id - kFileLoadHistoryMenuID;
				g_App.OpenSelectItem(selectindex);
				return true;
			}
			if (id >= kOpenBookmarkMenuID && id <= kOpenBookmarkMenuID + 9)
			{
				g_App.OpenBookmark(id - kOpenBookmarkMenuID);
				return true;
			}
			if (id >=kSetThemeID &&id <= kSetThemeID + 9)
			{
				CThemeDialog themeDialog;
				themeDialog.MeunTheme(id - kSetThemeID);
				return true;
			}
			else if (id >= kSetBookmarkMenuID && id <= kSetBookmarkMenuID + 9)
			{
				g_App.SetBookmark(id - kSetBookmarkMenuID);
				return true;
			}
			return false;
		}
	}
	return true;
}


