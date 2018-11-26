// ContextMenu.cpp

#include "StdAfx.h"
#include <stdio.h>

#include "ContextMenu.h"
#include "Common/StringConvert.h"

#include "Windows/COM.h"
#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/Memory.h"
#include "Windows/Menu.h"
#include "Windows/Process.h"
#include "Windows/Shell.h"
#include "Drapdrop.h"

#include "../Common/ArchiveName.h"
#include "../Common/CompressCall.h"
#include "../Common/ExtractingFilePath.h"
#include "../Common/ZipRegistry.h"
#include "../Common/IsArchiveSFX.h"

#include "../FileManager/FormatUtils.h"
#include "../FileManager/ProgramLocation.h"

#ifdef LANG
#include "../FileManager/LangUtils.h"
#endif

#include "ContextMenuFlags.h"
#include "MyMessages.h"

#include "resource.h"

#include "../FileManager/HttpUpdat.h"

#include "Windows/Registry.h"
#include "Windows/Synchronization.h"

using namespace NWindows;
using namespace NRegistry;


#ifndef UNDER_CE
#define EMAIL_SUPPORT 1
#endif

extern LONG g_DllRefCount;
    
extern HINSTANCE g_hInstance;

static UINT g_ContextMenuIndex = 20;//右键菜单ID起始位置在系统给出的最初位置的第20位开始

CZipContextMenu::CZipContextMenu()  { InterlockedIncrement(&g_DllRefCount); }
CZipContextMenu::~CZipContextMenu() { InterlockedDecrement(&g_DllRefCount); }

static NSynchronization::CCriticalSection g_CS;
static const TCHAR *kCuPrefix = TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR") TEXT(STRING_PATH_SEPARATOR);
static const TCHAR *kKeyName = TEXT("Theme");
static const WCHAR *kThemeName = L"ThemeName";
static CSysString GetKeyPath(const CSysString &path) { return kCuPrefix + path; }

static LONG OpenMainKey(CKey &key, LPCTSTR keyName)
{
	return key.Open(HKEY_CURRENT_USER, GetKeyPath(keyName), KEY_READ);
}

HRESULT CZipContextMenu::GetFileNames(LPDATAOBJECT dataObject, UStringVector &fileNames)
{
  #ifndef UNDER_CE
  fileNames.Clear();
  if (dataObject == NULL)
    return E_FAIL;
  FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  NCOM::CStgMedium stgMedium;
  HRESULT result = dataObject->GetData(&fmte, &stgMedium);
  if (result != S_OK)
    return result;
  stgMedium._mustBeReleased = true;

  NShell::CDrop drop(false);
  NMemory::CGlobalLock globalLock(stgMedium->hGlobal);
  drop.Attach((HDROP)globalLock.GetPointer());
  drop.QueryFileNames(fileNames);
  #endif
  return S_OK;
}

// IShellExtInit

STDMETHODIMP CZipContextMenu::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT dataObject, HKEY /* hkeyProgID */)
{
  CMyComPtr<IDropTarget> dropTarget;
  _dropMode = false;
  _dropPath.Empty();
  if (pidlFolder != 0)
  {
    #ifndef UNDER_CE
    if (NShell::GetPathFromIDList(pidlFolder, _dropPath))
    {
      NFile::NName::NormalizeDirPathPrefix(_dropPath);
      _dropMode = !_dropPath.IsEmpty();
    }
    else
    #endif
      _dropPath.Empty();
  }

/*  RegisterDragDrop((HWND)GetModuleHandleW(L"explorer.exe"),dropTarget);*/

  return GetFileNames(dataObject, _fileNames);
}

STDMETHODIMP CZipContextMenu::InitContextMenu(const wchar_t * /* folder */,
    const wchar_t **names, UINT32 numFiles)
{
  _fileNames.Clear();
  for (UINT32 i = 0; i < numFiles; i++)
    _fileNames.Add(names[i]);
  _dropMode = false;
  return S_OK;
}


/////////////////////////////
// IContextMenu

static LPCWSTR kMainVerb = L"SevenZip";

struct CContextMenuCommand
{
  UINT32 flag;
  CZipContextMenu::ECommandInternalID CommandInternalID;
  LPCWSTR Verb;
  UINT ResourceID;
  UINT ResourceHelpID;
  UINT32 LangID;
};

static CContextMenuCommand g_Commands[] =
{
  {
    NContextMenuFlags::kOpen,
    CZipContextMenu::kOpen,
    L"Open",
    IDS_CONTEXT_OPEN,
    IDS_CONTEXT_OPEN_HELP,
    0x04000700
  },
  {
    NContextMenuFlags::kExtract,
    CZipContextMenu::kExtract,
    L"Extract",
    IDS_CONTEXT_EXTRACT,
    IDS_CONTEXT_EXTRACT_HELP,
    0x04000701
  },
  {
    NContextMenuFlags::kExtractHere,
    CZipContextMenu::kExtractHere,
    L"ExtractHere",
    IDS_CONTEXT_EXTRACT_HERE,
    IDS_CONTEXT_EXTRACT_HERE_HELP,
    0x04000702
  },
  {
    NContextMenuFlags::kExtractTo,
    CZipContextMenu::kExtractTo,
    L"ExtractTo",
    IDS_CONTEXT_EXTRACT_TO,
    IDS_CONTEXT_EXTRACT_TO_HELP,
    0x04000703
  },
  {
    NContextMenuFlags::kTest,
    CZipContextMenu::kTest,
    L"Test",
    IDS_CONTEXT_TEST,
    IDS_CONTEXT_TEST_HELP,
    0x04000704
  },
  {
	NContextMenuFlags::kMoreExtractHere,
	CZipContextMenu::kMoreExtractHere,
	L"MoreExtractHere",
	IDS_CONTEXT_MORE_EXTRACT_HERE,
	IDS_CONTEXT_MORE_EXTRACT_HERE_HELP,
	0x04000710
   },
   {
	NContextMenuFlags::kMoreExtractTo,
	CZipContextMenu::kMoreExtractTo,
	L"MoreExtractTo",
	IDS_CONTEXT_MORE_EXTRACT_TO,
	IDS_CONTEXT_MORE_EXTRACT_TO_HELP,
	0x04000711
	},
  {
    NContextMenuFlags::kCompress,
    CZipContextMenu::kCompress,
    L"Compress",
    IDS_CONTEXT_COMPRESS,
    IDS_CONTEXT_COMPRESS_HELP,
    0x04000705,
  },
  {
    NContextMenuFlags::kCompressEmail,
    CZipContextMenu::kCompressEmail,
    L"CompressEmail",
    IDS_CONTEXT_COMPRESS_EMAIL,
    IDS_CONTEXT_COMPRESS_EMAIL_HELP,
    0x04000706
  },
  {
    NContextMenuFlags::kCompressTo7z,
    CZipContextMenu::kCompressTo7z,
    L"CompressTo7z",
    IDS_CONTEXT_COMPRESS_TO,
    IDS_CONTEXT_COMPRESS_TO_HELP,
    0x04000707
  },
  {
    NContextMenuFlags::kCompressTo7zEmail,
    CZipContextMenu::kCompressTo7zEmail,
    L"CompressTo7zEmail",
    IDS_CONTEXT_COMPRESS_TO_EMAIL,
    IDS_CONTEXT_COMPRESS_TO_EMAIL_HELP,
    0x04000708
  },
  {
    NContextMenuFlags::kCompressToZip,
    CZipContextMenu::kCompressToZip,
    L"CompressToZip",
    IDS_CONTEXT_COMPRESS_TO,
    IDS_CONTEXT_COMPRESS_TO_HELP,
    0x04000709
  },
  {
    NContextMenuFlags::kCompressToZipEmail,
    CZipContextMenu::kCompressToZipEmail,
    L"CompressToZipEmail",
    IDS_CONTEXT_COMPRESS_TO_EMAIL,
    IDS_CONTEXT_COMPRESS_TO_EMAIL_HELP,
    0x04000708
  }
};

int FindCommand(CZipContextMenu::ECommandInternalID &id)
{
  for (int i = 0; i < sizeof(g_Commands) / sizeof(g_Commands[0]); i++)
    if (g_Commands[i].CommandInternalID == id)
      return i;
  return -1;
}

void CZipContextMenu::FillCommand(ECommandInternalID id, UString &mainString, CCommandMapItem &commandMapItem)
{
  int i = FindCommand(id);
  if (i < 0)
    return;
  const CContextMenuCommand &command = g_Commands[i];
  commandMapItem.CommandInternalID = command.CommandInternalID;
  commandMapItem.Verb = (UString)kMainVerb + (UString)command.Verb;
  commandMapItem.HelpString = LangString(command.ResourceHelpID, command.LangID + 1);
  mainString = LangString(command.ResourceID, command.LangID);
}

static bool MyInsertMenu(CMenu &menu, int pos, UINT id, const UString &s)
{
	CMenuItem menuItem;
	menuItem.fType = MFT_STRING;
	menuItem.fMask = MIIM_TYPE | MIIM_ID;
	menuItem.wID = id;
	menuItem.StringValue = s;
	return menu.InsertItem(pos, true, menuItem);
}

static const wchar_t *kArcExts[] =
{
  L"7z",
  L"bz2",
  L"gz",
  L"rar",
  L"zip"
};

static bool IsItArcExt(const UString &ext2)
{
  UString ext = ext2;
  ext.MakeLower();
  for (int i = 0; i < sizeof(kArcExts) / sizeof(kArcExts[0]); i++)
    if (ext.Compare(kArcExts[i]) == 0)
      return true;
  return false;
}

static UString GetSubFolderNameForExtract(const UString &archiveName)
{
  int dotPos = archiveName.ReverseFind(L'.');
  if (dotPos < 0)
    return archiveName + UString(L"~");
  const UString ext = archiveName.Mid(dotPos + 1);
  UString res = archiveName.Left(dotPos);
  res.TrimRight();
  dotPos = res.ReverseFind(L'.');
  if (dotPos > 0)
  {
    const UString ext2 = res.Mid(dotPos + 1);
    if (ext.CompareNoCase(L"rar") == 0 &&
        (ext2.CompareNoCase(L"part001") == 0 ||
         ext2.CompareNoCase(L"part01") == 0 ||
         ext2.CompareNoCase(L"part1") == 0) ||
        IsItArcExt(ext2) && ext.CompareNoCase(L"001") == 0)
      res = res.Left(dotPos);
    res.TrimRight();
  }
  return GetCorrectFullFsPath(res);
}

static UString GetReducedString(const UString &s)
{
  const int kMaxSize = 64;
  if (s.Length() < kMaxSize)
    return s;
  const int kFirstPartSize = kMaxSize / 2;
  return s.Left(kFirstPartSize) + UString(L" ... ") + s.Right(kMaxSize - kFirstPartSize);
}

static UString GetQuotedReducedString(const UString &s)
{
  UString s2 = GetReducedString(s);
  s2.Replace(L"&", L"&&");
  return GetQuotedString(s2);
}

static const char *kExtractExludeExtensions =
" ace arj arc"
" bz2 bzip2"
" cab cpio"
" deb dmg"
" exe"
" gzip gz"
" hfs"
" jar jre"
" iso"
" rar rle rpm"
" lzh lha lzma"
" mid"
" ntfs"
" split swm"
" tar taz tbz tbz2 tgz tpz wim"
" uue"
" xar xpi xz"
" z zip 7z"
" 001"
" isz img lzma86 split"
" ";
static bool FindExt(const char *p, const UString &name)
{
  int extPos = name.ReverseFind('.');
  if (extPos < 0)
    return false;
  UString ext = name.Mid(extPos + 1);
  ext.MakeLower();
  AString ext2 = UnicodeStringToMultiByte(ext);
  for (int i = 0; p[i] != 0;)
  {
    int j;
    for (j = i; p[j] != ' '; j++);
    if (ext2.Length() == j - i && memcmp(p + i, (const char *)ext2, ext2.Length()) == 0)
      return true;
    i = j + 1;
  }
  return false;
}

static bool DoNeedExtract(const UString &name)
{
  return FindExt(kExtractExludeExtensions, name);
}
//外部引用函数WCharToMByte().
bool WCharToMByte2(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,NULL,0,NULL,FALSE);
	if(dwSize < dwMinSize)
	{
		return false;
	}
	WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,lpszStr,dwSize,NULL,FALSE);
	return true;
}

UString ThemeRegBmp()
{
	UString ThemeValue,ThemesBagName;
	CKey key;
	if (OpenMainKey(key,kKeyName) == ERROR_SUCCESS)
	{
		if (key.QueryValue(kThemeName,ThemesBagName) == ERROR_SUCCESS)
		{
			if (ThemesBagName == L"")
			{
				ThemeValue=L"";
			}
			else
			{
				ThemeValue=ThemesBagName;
			}
		}
		else
		{
			ThemeValue=L"";
		}
	}
	else
	{
		ThemeValue = L"";
	}
	return ThemeValue;
}
bool IsThereHaveOtherTheme(HBITMAP &bitmap)
{
	UString themevalue = ThemeRegBmp();
	UString path;
	if (themevalue != L"")
	{
		UString Pathtemp;
		WCHAR szPath[MAX_PATH];
		if(SUCCEEDED(SHGetFolderPathW(NULL, 
			CSIDL_APPDATA |CSIDL_FLAG_CREATE, 
			NULL, 
			0, 
			szPath))) 
		{
			Pathtemp = szPath;
		}
		else
			return false;
		Pathtemp=Pathtemp+WSTRING_PATH_SEPARATOR+L"CoolRAR\\Themes";
		path=Pathtemp+WSTRING_PATH_SEPARATOR+themevalue;
		path+=L"\\context.bmp";
		bitmap = (HBITMAP)LoadImageW(g_hInstance,path,
			IMAGE_BITMAP,
			0,	
			0,	
			LR_LOADFROMFILE|LR_CREATEDIBSECTION
			);
		return true;
	}
	return false;
}
STDMETHODIMP CZipContextMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu,
      UINT commandIDFirst, UINT commandIDLast, UINT flags)
{
  UString IsMoreArc;

  LoadLangOneTime();
  HBITMAP hbitmap;
  if(!IsThereHaveOtherTheme(hbitmap))
  {
	  UString path;
	  GetProgramFolderPath(path);
	  path+=L"icon\\context.bmp";
	  TCHAR bitmappath[4096];
	  memset(bitmappath,0,4096*sizeof(TCHAR));
	  WCharToMByte2(path.GetBuffer(path.Length()),bitmappath,sizeof(bitmappath)/sizeof(bitmappath[4096]));
	  hbitmap =(HBITMAP)LoadImageA(g_hInstance,bitmappath,
		  IMAGE_BITMAP,
		  0,	
		  0,	
		  LR_LOADFROMFILE|LR_CREATEDIBSECTION
		  );
  }
  if (_fileNames.Size() == 0)
    return E_FAIL;
  UINT currentCommandID = commandIDFirst + g_ContextMenuIndex;
  if ((flags & 0x000F) != CMF_NORMAL  &&
      (flags & CMF_VERBSONLY) == 0 &&
      (flags & CMF_EXPLORE) == 0)
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, currentCommandID);

  _commandMap.Clear();

  CMenu popupMenu;
  CMenuDestroyer menuDestroyer;

  CContextMenuInfo ci;
  ci.Load();
  MENUITEMINFO menuItem;
  UINT subIndex = indexMenu;
  if (ci.Cascaded)
  {
    CCommandMapItem commandMapItem;
    if (!popupMenu.CreatePopup())
      return E_FAIL;
    menuDestroyer.Attach(popupMenu);
    commandMapItem.CommandInternalID = kCommandNULL;
    commandMapItem.Verb = kMainVerb;
    commandMapItem.HelpString = LangString(IDS_CONTEXT_CAPTION_HELP, 0x02000102);
    _commandMap.Add(commandMapItem);
    
    menuItem.wID = currentCommandID++;
    subIndex = 0;
  }
  else
  {
    popupMenu.Attach(hMenu);
  }

  UInt32 contextMenuFlags = ci.Flags;

  UString mainString;
  UString dllpath;
  GetProgramFolderPath(dllpath);
  if (_fileNames.Size() == 1 && currentCommandID + 6 <= commandIDLast)
  {
    const UString &fileName = _fileNames.Front();
    UString folderPrefix;
    NFile::NDirectory::GetOnlyDirPrefix(fileName, folderPrefix);
   
    NFile::NFind::CFileInfoW fileInfo;
    if (!fileInfo.Find(fileName))
      return E_FAIL;

	
    if (!fileInfo.IsDir()                                 //非文件夹
		&& DoNeedExtract(fileInfo.Name)					  //后缀名正确						
		&& IsArchiveSFX(_fileNames[0],dllpath) == S_OK )	  //具有压缩性质	
    {
      if ((contextMenuFlags & NContextMenuFlags::kOpen) != 0)
      {
        CCommandMapItem commandMapItem;
        FillCommand(kOpen, mainString, commandMapItem);
        MyInsertMenu(popupMenu, subIndex, currentCommandID++, mainString);
		::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
        _commandMap.Add(commandMapItem);
      }
    }
  }

  if (_fileNames.Size() > 0 && currentCommandID + 10 <= commandIDLast)
  {
    bool needExtract = false;
	bool needCompress = false;
    for(int i = 0; i < _fileNames.Size(); i++)
    {
      NFile::NFind::CFileInfoW fileInfo;
      if (!fileInfo.Find(_fileNames[i]))
        return E_FAIL;

		 bool MoreArc;
	     bool ininin = fileInfo.IsArchived();
		 MoreArc=ininin;
		 bool on=fileInfo.IsCompressed();

      if (!fileInfo.IsDir() && DoNeedExtract(fileInfo.Name)) //根据文件后缀名和是否具有文件夹性质来确定菜单项
        needExtract = true;
	  else needCompress = true;
	 
	  int extPos = fileInfo.Name.ReverseFind('.');
	  if (extPos > 0)
	  {
		  UString ext = fileInfo.Name.Mid(extPos + 1);
		  ext.MakeLower();

		  if(_fileNames.Size() == 1 
			  &&ext == L"exe")   //检测是否是后缀名为EXE的可执行文件
		  {
			  needCompress = true;
			  if( IsArchiveSFX(_fileNames[0],dllpath) == S_OK)//检测是否为自解压文件
				needExtract = true;
			  else needExtract = false;
			  
		  }
	  }
	  UString Isexe = fileInfo.Name.Mid(extPos + 1);
	  Isexe.MakeLower();

	  if (MoreArc ==true && DoNeedExtract(fileInfo.Name) && Isexe != L"exe")
	  {
		  IsMoreArc +=L"a";
	  }
	 
    }

	if (_fileNames.Size() >1)
	{
		needExtract = false;
		needCompress =true;
		if( IsMoreArc.Length() == _fileNames.Size())
		{
			const UString &fileName = _fileNames.Front();
			UString folderPrefix;
			NFile::NDirectory::GetOnlyDirPrefix(fileName, folderPrefix);
			NFile::NFind::CFileInfoW fileInfo;
			if (!fileInfo.Find(fileName))
				return E_FAIL;
			//MoreExtract Here
			if ((contextMenuFlags & NContextMenuFlags::kMoreExtractHere) != 0)
			{
				CCommandMapItem commandMapItem;
				FillCommand(kMoreExtractHere, mainString, commandMapItem);
				MyInsertMenu(popupMenu, subIndex, currentCommandID++, mainString);
				::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
				if (_dropMode)
					commandMapItem.Folder = _dropPath;
				else
					commandMapItem.Folder = folderPrefix;
				_commandMap.Add(commandMapItem);
			}

			// MoreExtract To
			if ((contextMenuFlags & NContextMenuFlags::kMoreExtractTo) != 0)
			{
				CCommandMapItem commandMapItem;
				UString s;
				FillCommand(kMoreExtractTo, s, commandMapItem);
				UString folder;
				if (_fileNames.Size() == 1)
					folder = GetSubFolderNameForExtract(fileInfo.Name);
				else
					folder = L'*';
				if (_dropMode)
					commandMapItem.Folder = _dropPath;
				else
					commandMapItem.Folder = folderPrefix;
				commandMapItem.Folder += folder;
				MyInsertMenu(popupMenu, subIndex, currentCommandID++, s);
				::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);

				_commandMap.Add(commandMapItem);
			}
		}
	}
    const UString &fileName = _fileNames.Front();
    if (needExtract)
    {
      UString folderPrefix;
      NFile::NDirectory::GetOnlyDirPrefix(fileName, folderPrefix);
      NFile::NFind::CFileInfoW fileInfo;
      if (!fileInfo.Find(fileName))
        return E_FAIL;
      // Extract
      if ((contextMenuFlags & NContextMenuFlags::kExtract) != 0)
      {
        CCommandMapItem commandMapItem;
        FillCommand(kExtract, mainString, commandMapItem);
        if (_dropMode)
          commandMapItem.Folder = _dropPath;
        else
          commandMapItem.Folder = folderPrefix;
        commandMapItem.Folder += GetSubFolderNameForExtract(fileInfo.Name) + UString(WCHAR_PATH_SEPARATOR);
        MyInsertMenu(popupMenu, subIndex, currentCommandID++, mainString);
		::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);

        _commandMap.Add(commandMapItem);
      }

      // Extract Here
      if ((contextMenuFlags & NContextMenuFlags::kExtractHere) != 0)
      {
        CCommandMapItem commandMapItem;
        FillCommand(kExtractHere, mainString, commandMapItem);
        MyInsertMenu(popupMenu, subIndex, currentCommandID++, mainString);
		::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
        if (_dropMode)
          commandMapItem.Folder = _dropPath;
        else
          commandMapItem.Folder = folderPrefix;
        _commandMap.Add(commandMapItem);
      }

      // Extract To
      if ((contextMenuFlags & NContextMenuFlags::kExtractTo) != 0)
      {
        CCommandMapItem commandMapItem;
        UString s;
        FillCommand(kExtractTo, s, commandMapItem);
        UString folder;
        if (_fileNames.Size() == 1)
          folder = GetSubFolderNameForExtract(fileInfo.Name);
        else
          folder = L'*';
        if (_dropMode)
          commandMapItem.Folder = _dropPath;
        else
          commandMapItem.Folder = folderPrefix;
        commandMapItem.Folder += folder;
        s = MyFormatNew(s, GetQuotedReducedString(folder + UString(WCHAR_PATH_SEPARATOR)));
        MyInsertMenu(popupMenu, subIndex, currentCommandID++, s);
		::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
		
        _commandMap.Add(commandMapItem);
      }

      // Test
      if ((contextMenuFlags & NContextMenuFlags::kTest) != 0)
      {
        CCommandMapItem commandMapItem;
        FillCommand(kTest, mainString, commandMapItem);
        MyInsertMenu(popupMenu, subIndex, currentCommandID++, mainString);
		::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
        _commandMap.Add(commandMapItem);
      }
    }
    UString archiveName = CreateArchiveName(fileName, _fileNames.Size() > 1, false);
    UString archiveName7z = archiveName + L".7z";
    UString archiveNameZip = archiveName + L".zip";
    UString archivePathPrefix;
    NFile::NDirectory::GetOnlyDirPrefix(fileName, archivePathPrefix);

    // Compress
	if (needCompress)
	{
		if ((contextMenuFlags & NContextMenuFlags::kCompress) != 0)
		{
			CCommandMapItem commandMapItem;
			if (_dropMode)
				commandMapItem.Folder = _dropPath;
			else
				commandMapItem.Folder = archivePathPrefix;
			commandMapItem.Archive = archiveName;
			FillCommand(kCompress, mainString, commandMapItem);
			MyInsertMenu(popupMenu, subIndex, currentCommandID++, mainString);
			::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
		
			_commandMap.Add(commandMapItem);
		}
	
    #ifdef EMAIL_SUPPORT
    // CompressEmail
    if ((contextMenuFlags & NContextMenuFlags::kCompressEmail) != 0 && !_dropMode)
    {
      CCommandMapItem commandMapItem;
      commandMapItem.Archive = archiveName;
      FillCommand(kCompressEmail, mainString, commandMapItem);
      MyInsertMenu(popupMenu, subIndex, currentCommandID++, mainString);
	  ::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
	  
      _commandMap.Add(commandMapItem);
    }
    #endif

    // CompressTo7z
    if (contextMenuFlags & NContextMenuFlags::kCompressTo7z)
    {
      CCommandMapItem commandMapItem;
      UString s;
      FillCommand(kCompressTo7z, s, commandMapItem);
      if (_dropMode)
        commandMapItem.Folder = _dropPath;
      else
        commandMapItem.Folder = archivePathPrefix;
      commandMapItem.Archive = archiveName7z;
      commandMapItem.ArchiveType = L"7z";
      s = MyFormatNew(s, GetQuotedReducedString(archiveName7z));
      MyInsertMenu(popupMenu, subIndex, currentCommandID++, s);
	  ::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
	  
      _commandMap.Add(commandMapItem);
    }

    #ifdef EMAIL_SUPPORT
    // CompressTo7zEmail
    if ((contextMenuFlags & NContextMenuFlags::kCompressTo7zEmail) != 0  && !_dropMode)
    {
      CCommandMapItem commandMapItem;
      UString s;
      FillCommand(kCompressTo7zEmail, s, commandMapItem);
      commandMapItem.Archive = archiveName7z;
      commandMapItem.ArchiveType = L"7z";
      s = MyFormatNew(s, GetQuotedReducedString(archiveName7z));
      MyInsertMenu(popupMenu, subIndex, currentCommandID++, s);
	  ::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
	  
      _commandMap.Add(commandMapItem);
    }
    #endif

    // CompressToZip
    if (contextMenuFlags & NContextMenuFlags::kCompressToZip)
    {
      CCommandMapItem commandMapItem;
      UString s;
      FillCommand(kCompressToZip, s, commandMapItem);
      if (_dropMode)
        commandMapItem.Folder = _dropPath;
      else
        commandMapItem.Folder = archivePathPrefix;
      commandMapItem.Archive = archiveNameZip;
      commandMapItem.ArchiveType = L"zip";
      s = MyFormatNew(s, GetQuotedReducedString(archiveNameZip));
      MyInsertMenu(popupMenu, subIndex, currentCommandID++, s);
	  ::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
	  
      _commandMap.Add(commandMapItem);
    }

    #ifdef EMAIL_SUPPORT
    // CompressToZipEmail
    if ((contextMenuFlags & NContextMenuFlags::kCompressToZipEmail) != 0  && !_dropMode)
    {
      CCommandMapItem commandMapItem;
      UString s;
      FillCommand(kCompressToZipEmail, s, commandMapItem);
      commandMapItem.Archive = archiveNameZip;
      commandMapItem.ArchiveType = L"zip";
      s = MyFormatNew(s, GetQuotedReducedString(archiveNameZip));
      MyInsertMenu(popupMenu, subIndex, currentCommandID++, s);
	  ::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
	  
      _commandMap.Add(commandMapItem);
    }
    #endif
  }
  }
  if (ci.Cascaded)
  {
    CMenuItem menuItem;
    menuItem.fType = MFT_STRING;
    menuItem.fMask = MIIM_SUBMENU | MIIM_TYPE | MIIM_ID| MIIM_ID|MIIM_CHECKMARKS;;
    menuItem.wID = currentCommandID++;
    menuItem.hSubMenu = popupMenu.Detach();
    menuDestroyer.Disable();
    menuItem.StringValue = LangString(IDS_CONTEXT_POPUP_CAPTION, 0x02000101);
    CMenu menu;
    menu.Attach(hMenu);
    menu.InsertItem(indexMenu, true, menuItem);
    ::SetMenuItemBitmaps(popupMenu,subIndex++,MF_BYPOSITION,hbitmap,hbitmap);
	
	
  }


  
 return MAKE_HRESULT(SEVERITY_SUCCESS, 0, currentCommandID - commandIDFirst);

}


int CZipContextMenu::FindVerb(const UString &verb)
{
  for(int i = 0; i < _commandMap.Size(); i++)
    if (_commandMap[i].Verb.Compare(verb) == 0)
      return i;
  return -1;
}

static UString Get7zFmPath()
{
  UString path;
  GetProgramFolderPath(path);
  return path + L"CoolRAR.exe";
}

STDMETHODIMP CZipContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO commandInfo)
{
  HttpUpdat updatDll;
  int commandOffset;

  // It's fix for bug: crashing in XP. See example in MSDN: "Creating Context Menu Handlers".

  #ifndef UNDER_CE
  if (commandInfo->cbSize == sizeof(CMINVOKECOMMANDINFOEX) &&
      (commandInfo->fMask & CMIC_MASK_UNICODE) != 0)
  {
    LPCMINVOKECOMMANDINFOEX commandInfoEx = (LPCMINVOKECOMMANDINFOEX)commandInfo;
    if (HIWORD(commandInfoEx->lpVerbW) == 0)
      commandOffset = LOWORD(commandInfo->lpVerb);
    else
      commandOffset = FindVerb(commandInfoEx->lpVerbW);
  }
  else
  #endif
    if (HIWORD(commandInfo->lpVerb) == 0)
      commandOffset = LOWORD(commandInfo->lpVerb);
    else
      commandOffset = FindVerb(GetUnicodeString(commandInfo->lpVerb));
	commandOffset = commandOffset - g_ContextMenuIndex;
  if (commandOffset < 0 || commandOffset >= _commandMap.Size())
    return E_FAIL;

  const CCommandMapItem commandMapItem = _commandMap[commandOffset];
  ECommandInternalID commandInternalID = commandMapItem.CommandInternalID;

  try
  {
    switch(commandInternalID)
    {
      case kOpen:
      {

		  UString path;
		  GetProgramFolderPath(path);	  
		  updatDll.DllGetUpdat();
		  updatDll.DestroyDll();
		  UString params = GetQuotedString(_fileNames[0]);
		  MyCreateProcess(Get7zFmPath(), params);
        break;
      }
      case kExtract:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  ExtractArchives(_fileNames, commandMapItem.Folder, (commandInternalID == kExtract));
			  break;
		  }
      case kExtractHere:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  ExtractArchives(_fileNames, commandMapItem.Folder, (commandInternalID == kExtract));
			  break;
		  }
      case kExtractTo:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  ExtractArchives(_fileNames, commandMapItem.Folder, (commandInternalID == kExtract));
			  break;
		  }
	  case kMoreExtractHere:
		  {
			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  ExtractArchives(_fileNames, commandMapItem.Folder, !(commandInternalID == kMoreExtractHere));
			  break;
		  }
	  case kMoreExtractTo:
		  {
			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  ExtractArchives(_fileNames,commandMapItem.Folder,!(commandInternalID ==kMoreExtractTo));
			  break;
		  }
	  case kTest:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  TestArchives(_fileNames);
			  break;
		  }
      case kCompress:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  bool email =
				  (commandInternalID == kCompressEmail) ||
				  (commandInternalID == kCompressTo7zEmail) ||
				  (commandInternalID == kCompressToZipEmail);
			  bool showDialog =
				  (commandInternalID == kCompress) ||
				  (commandInternalID == kCompressEmail);
			  CompressFiles(commandMapItem.Folder,
				  commandMapItem.Archive, commandMapItem.ArchiveType,
				  _fileNames, email, showDialog, false);
			  break;
		  }
      case kCompressEmail:
		  {
			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  bool email =
				  (commandInternalID == kCompressEmail) ||
				  (commandInternalID == kCompressTo7zEmail) ||
				  (commandInternalID == kCompressToZipEmail);
			  bool showDialog =
				  (commandInternalID == kCompress) ||
				  (commandInternalID == kCompressEmail);
			  CompressFiles(commandMapItem.Folder,
				  commandMapItem.Archive, commandMapItem.ArchiveType,
				  _fileNames, email, showDialog, false);
			  break;
		  }
      case kCompressTo7z:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  bool email =
				  (commandInternalID == kCompressEmail) ||
				  (commandInternalID == kCompressTo7zEmail) ||
				  (commandInternalID == kCompressToZipEmail);
			  bool showDialog =
				  (commandInternalID == kCompress) ||
				  (commandInternalID == kCompressEmail);
			  CompressFiles(commandMapItem.Folder,
				  commandMapItem.Archive, commandMapItem.ArchiveType,
				  _fileNames, email, showDialog, false);
			  break;
		  }
      case kCompressTo7zEmail:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();

			  bool email =
				  (commandInternalID == kCompressEmail) ||
				  (commandInternalID == kCompressTo7zEmail) ||
				  (commandInternalID == kCompressToZipEmail);
			  bool showDialog =
				  (commandInternalID == kCompress) ||
				  (commandInternalID == kCompressEmail);
			  CompressFiles(commandMapItem.Folder,
				  commandMapItem.Archive, commandMapItem.ArchiveType,
				  _fileNames, email, showDialog, false);
			  break;
		  }
      case kCompressToZip:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  bool email =
				  (commandInternalID == kCompressEmail) ||
				  (commandInternalID == kCompressTo7zEmail) ||
				  (commandInternalID == kCompressToZipEmail);
			  bool showDialog =
				  (commandInternalID == kCompress) ||
				  (commandInternalID == kCompressEmail);
			  CompressFiles(commandMapItem.Folder,
				  commandMapItem.Archive, commandMapItem.ArchiveType,
				  _fileNames, email, showDialog, false);
			  break;
		  }
      case kCompressToZipEmail:
		  {

			  UString path;
			  GetProgramFolderPath(path);
			  updatDll.DllGetUpdat();
			  updatDll.DestroyDll();
			  bool email =
				  (commandInternalID == kCompressEmail) ||
				  (commandInternalID == kCompressTo7zEmail) ||
				  (commandInternalID == kCompressToZipEmail);
			  bool showDialog =
				  (commandInternalID == kCompress) ||
				  (commandInternalID == kCompressEmail);
			  CompressFiles(commandMapItem.Folder,
				  commandMapItem.Archive, commandMapItem.ArchiveType,
				  _fileNames, email, showDialog, false);
			  break;
		  }
    }
  }
  catch(...)
  {
    ::MessageBoxW(0, L"Error", L"CoolRAR", MB_ICONERROR);
  }
  return S_OK;
}

static void MyCopyString(void *dest, const wchar_t *src, bool writeInUnicode)
{
  if (writeInUnicode)
  {
    MyStringCopy((wchar_t *)dest, src);
  }
  else
    MyStringCopy((char *)dest, (const char *)GetAnsiString(src));
}

STDMETHODIMP CZipContextMenu::GetCommandString(UINT_PTR commandOffset, UINT uType,
    UINT * /* pwReserved */ , LPSTR pszName, UINT /* cchMax */)
{
  int cmdOffset = (int)commandOffset;
  switch(uType)
  {
    #ifdef UNDER_CE
    case GCS_VALIDATE:
    #else
    case GCS_VALIDATEA:
    case GCS_VALIDATEW:
    #endif
      if (cmdOffset < 0 || cmdOffset >= _commandMap.Size())
        return S_FALSE;
      else
        return S_OK;
  }
  if (cmdOffset < 0 || cmdOffset >= _commandMap.Size())
    return E_FAIL;
  #ifdef UNDER_CE
  if (uType == GCS_HELPTEXT)
  #else
  if (uType == GCS_HELPTEXTA || uType == GCS_HELPTEXTW)
  #endif
  {
    MyCopyString(pszName, _commandMap[cmdOffset].HelpString,
      #ifdef UNDER_CE
      true
      #else
      uType == GCS_HELPTEXTW
      #endif
      );
    return NO_ERROR;
  }
  #ifdef UNDER_CE
  if (uType == GCS_VERB)
  #else
  if (uType == GCS_VERBA || uType == GCS_VERBW)
  #endif
  {
    MyCopyString(pszName, _commandMap[cmdOffset].Verb,
      #ifdef UNDER_CE
      true
      #else
      uType == GCS_VERBW
      #endif
      );
    return NO_ERROR;
  }
  return E_FAIL;
}
