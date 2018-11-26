// Windows/Shell.cpp

#include "StdAfx.h"

#include "Common/MyCom.h"
#ifndef _UNICODE
#include "Common/StringConvert.h"
#endif

#include "Windows/COM.h"
#include "Windows/Shell.h"

#ifndef _UNICODE
extern bool g_IsNT;
#endif

namespace NWindows {
namespace NShell {

#ifndef UNDER_CE

// SHGetMalloc is unsupported in Windows Mobile?

void CItemIDList::Free()
{
  if (m_Object == NULL)
    return;
  CMyComPtr<IMalloc> shellMalloc;
  if (::SHGetMalloc(&shellMalloc) != NOERROR)
    throw 41099;
  shellMalloc->Free(m_Object);
  m_Object = NULL;
}


/////////////////////////////
// CDrop

void CDrop::Attach(HDROP object)
{
  Free();
  m_Object = object;
  m_Assigned = true;
}

void CDrop::Free()
{
  if (m_MustBeFinished && m_Assigned)
    Finish();
  m_Assigned = false;
}

UINT CDrop::QueryCountOfFiles()
{
  return QueryFile(0xFFFFFFFF, (LPTSTR)NULL, 0);
}

UString CDrop::QueryFileName(UINT fileIndex)
{
  UString fileName;
  #ifndef _UNICODE
  if (!g_IsNT)
  {
    AString fileNameA;
    UINT bufferSize = QueryFile(fileIndex, (LPTSTR)NULL, 0);
    QueryFile(fileIndex, fileNameA.GetBuffer(bufferSize + 2), bufferSize + 1);
    fileNameA.ReleaseBuffer();
    fileName = GetUnicodeString(fileNameA);
  }
  else
  #endif
  {
    UINT bufferSize = QueryFile(fileIndex, (LPWSTR)NULL, 0);
    QueryFile(fileIndex, fileName.GetBuffer(bufferSize + 2), bufferSize + 1);
    fileName.ReleaseBuffer();
  }
  return fileName;
}

void CDrop::QueryFileNames(UStringVector &fileNames)
{
  fileNames.Clear();
  UINT numFiles = QueryCountOfFiles();
  fileNames.Reserve(numFiles);
  for (UINT i = 0; i < numFiles; i++)
    fileNames.Add(QueryFileName(i));
}


bool GetPathFromIDList(LPCITEMIDLIST itemIDList, CSysString &path)
{
  bool result = BOOLToBool(::SHGetPathFromIDList(itemIDList, path.GetBuffer(MAX_PATH * 2)));
  path.ReleaseBuffer();
  return result;
}

#endif

#ifdef UNDER_CE

bool BrowseForFolder(LPBROWSEINFO, CSysString)
{
  return false;
}

bool BrowseForFolder(HWND, LPCTSTR, UINT, LPCTSTR, CSysString &)
{
  return false;
}

bool BrowseForFolder(HWND owner, LPCTSTR title,
    LPCTSTR initialFolder, CSysString &resultPath)
{
  /*
  // SHBrowseForFolder doesn't work before CE 6.0 ?
  if (GetProcAddress(LoadLibrary(L"ceshell.dll", L"SHBrowseForFolder") == 0)
    MessageBoxW(0, L"no", L"", 0);
  else
    MessageBoxW(0, L"yes", L"", 0);
  */
  /*
  UString s = L"all files";
  s += L" (*.*)";
  return MyGetOpenFileName(owner, title, initialFolder, s, resultPath, true);
  */
  return false;
}

#else

bool BrowseForFolder(LPBROWSEINFO browseInfo, CSysString &resultPath)
{
  NWindows::NCOM::CComInitializer comInitializer;
  LPITEMIDLIST itemIDList = ::SHBrowseForFolder(browseInfo);
  if (itemIDList == NULL)
    return false;
  CItemIDList itemIDListHolder;
  itemIDListHolder.Attach(itemIDList);
  return GetPathFromIDList(itemIDList, resultPath);
}


int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /* lp */, LPARAM data)
{
  #ifndef UNDER_CE
  switch(uMsg)
  {
    case BFFM_INITIALIZED:
    {
      SendMessage(hwnd, BFFM_SETSELECTION, TRUE, data);
      break;
    }
    default:
      break;
  }
  #endif
  return 0;
}


bool BrowseForFolder(HWND owner, LPCTSTR title, UINT ulFlags,
    LPCTSTR initialFolder, CSysString &resultPath)
{
  CSysString displayName;
  BROWSEINFO browseInfo;
  browseInfo.hwndOwner = owner;
  browseInfo.pidlRoot = NULL;

  // there are Unicode/astring problems in WinCE SDK!!!
  #ifdef UNDER_CE
  browseInfo.pszDisplayName = (LPSTR)displayName.GetBuffer(MAX_PATH);
  browseInfo.lpszTitle = (LPCSTR)title;
  #else
  browseInfo.pszDisplayName = displayName.GetBuffer(MAX_PATH);
  browseInfo.lpszTitle = title;
  #endif
  browseInfo.ulFlags = ulFlags;
  browseInfo.lpfn = (initialFolder != NULL) ? BrowseCallbackProc : NULL;
  browseInfo.lParam = (LPARAM)initialFolder;
  return BrowseForFolder(&browseInfo, resultPath);
}

bool BrowseForFolder(HWND owner, LPCTSTR title,
    LPCTSTR initialFolder, CSysString &resultPath)
{
  return BrowseForFolder(owner, title,
      #ifndef UNDER_CE
      BIF_NEWDIALOGSTYLE |BIF_BROWSEINCLUDEFILES|
      #endif
       BIF_STATUSTEXT |BIF_BROWSEINCLUDEFILES , initialFolder, resultPath);
}

#ifndef _UNICODE

typedef BOOL (WINAPI * SHGetPathFromIDListWP)(LPCITEMIDLIST pidl, LPWSTR pszPath);

bool GetPathFromIDList(LPCITEMIDLIST itemIDList, UString &path)
{
  path.Empty();
  SHGetPathFromIDListWP shGetPathFromIDListW = (SHGetPathFromIDListWP)
    ::GetProcAddress(::GetModuleHandleW(L"shell32.dll"), "SHGetPathFromIDListW");
  if (shGetPathFromIDListW == 0)
    return false;
  bool result = BOOLToBool(shGetPathFromIDListW(itemIDList, path.GetBuffer(MAX_PATH * 2)));
  path.ReleaseBuffer();
  return result;
}

typedef LPITEMIDLIST (WINAPI * SHBrowseForFolderWP)(LPBROWSEINFOW lpbi);

bool BrowseForFolder(LPBROWSEINFOW browseInfo, UString &resultPath)
{
  NWindows::NCOM::CComInitializer comInitializer;
  SHBrowseForFolderWP shBrowseForFolderW = (SHBrowseForFolderWP)
    ::GetProcAddress(::GetModuleHandleW(L"shell32.dll"), "SHBrowseForFolderW");
  if (shBrowseForFolderW == 0)
    return false;
  LPITEMIDLIST itemIDList = shBrowseForFolderW(browseInfo);
  if (itemIDList == NULL)
    return false;
  CItemIDList itemIDListHolder;
  itemIDListHolder.Attach(itemIDList);
  return GetPathFromIDList(itemIDList, resultPath);
}


int CALLBACK BrowseCallbackProc2(HWND hwnd, UINT uMsg, LPARAM /* lp */, LPARAM data)
{
  switch(uMsg)
  {
    case BFFM_INITIALIZED:
    {
      SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, data);
      break;
    }
    default:
      break;
  }
  return 0;
}


static bool BrowseForFolder(HWND owner, LPCWSTR title, UINT ulFlags,
    LPCWSTR initialFolder, UString &resultPath)
{
  UString displayName;
  BROWSEINFOW browseInfo;
  browseInfo.hwndOwner = owner;
  browseInfo.pidlRoot = NULL;
  browseInfo.pszDisplayName = displayName.GetBuffer(MAX_PATH);
  browseInfo.lpszTitle = title;
  browseInfo.ulFlags = ulFlags;
  browseInfo.lpfn = (initialFolder != NULL) ? BrowseCallbackProc2 : NULL;
  browseInfo.lParam = (LPARAM)initialFolder;
  return BrowseForFolder(&browseInfo, resultPath);
}

bool BrowseForFolder(HWND owner, LPCWSTR title, LPCWSTR initialFolder, UString &resultPath)
{
  if (g_IsNT)
    return BrowseForFolder(owner, title,
      BIF_NEWDIALOGSTYLE  |BIF_BROWSEINCLUDEFILES 
      , initialFolder, resultPath);
  CSysString s;
  bool res = BrowseForFolder(owner, GetSystemString(title),
      BIF_NEWDIALOGSTYLE  |BIF_BROWSEINCLUDEFILES 
      , GetSystemString(initialFolder), s);
  resultPath = GetUnicodeString(s);
  return res;
}

#endif

#endif

}}
