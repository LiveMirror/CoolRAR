// Windows/Clipboard.cpp

#include "StdAfx.h"

#ifdef UNDER_CE
#include <winuserm.h>
#endif

#include "Windows/Clipboard.h"
#include "Windows/Defs.h"
#include "Windows/Memory.h"
#include "Windows/Shell.h"
#include "Windows/Memory.h"

#include "Common/StringConvert.h"

namespace NWindows {

bool CClipboard::Open(HWND wndNewOwner)
{
  m_Open = BOOLToBool(::OpenClipboard(wndNewOwner));
  return m_Open;
}

CClipboard::~CClipboard()
{
  Close();
}

bool CClipboard::Close()
{
  if (!m_Open)
    return true;
  m_Open = !BOOLToBool(CloseClipboard());
  return !m_Open;
}

bool ClipboardIsFormatAvailableHDROP()
{
  return BOOLToBool(IsClipboardFormatAvailable(CF_HDROP));
}

static bool ClipboardSetData(UINT uFormat, const void *data, size_t size)
{
  NMemory::CGlobal global;
  if (!global.Alloc(GMEM_DDESHARE | GMEM_MOVEABLE, size))
    return false;
  {
    NMemory::CGlobalLock globalLock(global);
    LPVOID p = globalLock.GetPointer();
    if (p == NULL)
      return false;
    memcpy(p, data, size);
  }
  if (::SetClipboardData(uFormat, global) == NULL)
    return false;
  global.Detach();
  return true;
}

bool ClipboardSetText(HWND owner, const UString &s)
{
  CClipboard clipboard;
  if (!clipboard.Open(owner))
    return false;
  if (!::EmptyClipboard())
    return false;

  bool res;
  res = ClipboardSetData(CF_UNICODETEXT, (const wchar_t *)s, (s.Length() + 1) * sizeof(wchar_t));
  #ifndef _UNICODE
  AString a;
  a = UnicodeStringToMultiByte(s, CP_ACP);
  res |=  ClipboardSetData(CF_TEXT, (const char *)a, (a.Length() + 1) * sizeof(char));
  a = UnicodeStringToMultiByte(s, CP_OEMCP);
  res |=  ClipboardSetData(CF_OEMTEXT, (const char *)a, (a.Length() + 1) * sizeof(char));
  #endif
  return res;
}
 
}
