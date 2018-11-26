// RegistryAssociations.cpp

#include "StdAfx.h"

#include "RegistryAssociations.h"

#include "Common/IntToString.h"
#include "Common/StringConvert.h"
#include "Common/StringToInt.h"

#include "Windows/Registry.h"
#include "Windows/Synchronization.h"

#include "StringUtils.h"

using namespace NWindows;
using namespace NRegistry;

namespace NRegistryAssociations {
  
static NSynchronization::CCriticalSection g_CriticalSection;

#define REG_PATH_FM TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR")



///////////////////////////////////
// External

static const TCHAR *kShellNewKeyName = TEXT("ShellNew");
static const TCHAR *kShellNewDataValueName = TEXT("Data");
  
static const TCHAR *kDefaultIconKeyName = TEXT("DefaultIcon");
static const TCHAR *kShellKeyName = TEXT("shell");
static const TCHAR *kOpenKeyName = TEXT("open");
static const TCHAR *kCommandKeyName = TEXT("command");
static const TCHAR *k7zipPrefix = TEXT("CoolRAR.");
static const TCHAR *kShellexKeyName = TEXT("shellex");
static const TCHAR *kDropHandlerKeyName = TEXT("DropHandler");
static const TCHAR *kContextMenuHandlers = TEXT("ContextMenuHandlers");
static const TCHAR *kPropertySheetHandlers = TEXT("PropertySheetHandlers");
static const TCHAR *kDropkExtensionCLSID = TEXT("{23170F69-40C1-278A-1000-000100020001}");


static CSysString GetExtensionKeyName(const CSysString &extension)
{
  return CSysString(TEXT(".")) + extension;
}

static CSysString GetExtProgramKeyName(const CSysString &extension)
{
  return CSysString(k7zipPrefix) + extension;
}

static bool CheckShellExtensionInfo2(const CSysString &extension,
    CSysString programKeyName, UString &iconPath, int &iconIndex)
{
  iconIndex = -1;
  iconPath.Empty();
  NSynchronization::CCriticalSectionLock lock(g_CriticalSection);
  CKey extKey;
  if (extKey.Open(HKEY_CLASSES_ROOT, GetExtensionKeyName(extension), KEY_READ) != ERROR_SUCCESS)
    return false;
  if (extKey.QueryValue(NULL, programKeyName) != ERROR_SUCCESS)
    return false;
  UString s = GetUnicodeString(k7zipPrefix);
  if (s.CompareNoCase(GetUnicodeString(programKeyName.Left(s.Length()))) != 0)
    return false;
  CKey iconKey;
  if (extKey.Open(HKEY_CLASSES_ROOT, programKeyName + CSysString(TEXT(CHAR_PATH_SEPARATOR)) + kDefaultIconKeyName, KEY_READ) != ERROR_SUCCESS)
    return false;
  UString value;
  if (extKey.QueryValue(NULL, value) == ERROR_SUCCESS)
  {
    int pos = value.ReverseFind(L',');
    iconPath = value;
    if (pos >= 0)
    {
      const wchar_t *end;
      UInt64 index = ConvertStringToUInt64((const wchar_t *)value + pos + 1, &end);
      if (*end == 0)
      {
        iconIndex = (int)index;
        iconPath = value.Left(pos);
      }
    }
  }
  return true;
}

bool CheckShellExtensionInfo(const CSysString &extension, UString &iconPath, int &iconIndex)
{
  NSynchronization::CCriticalSectionLock lock(g_CriticalSection);
  CSysString programKeyName;
  if (!CheckShellExtensionInfo2(extension, programKeyName, iconPath, iconIndex))
    return false;
  CKey extProgKey;
  return (extProgKey.Open(HKEY_CLASSES_ROOT, programKeyName, KEY_READ) == ERROR_SUCCESS);
}

static void DeleteShellExtensionKey(const CSysString &extension)
{
  NSynchronization::CCriticalSectionLock lock(g_CriticalSection);
  CKey rootKey;
  rootKey.Attach(HKEY_CLASSES_ROOT);
  rootKey.RecurseDeleteKey(GetExtensionKeyName(extension));
  rootKey.Detach();
}

static void DeleteShellExtensionProgramKey(const CSysString &extension)
{
  NSynchronization::CCriticalSectionLock lock(g_CriticalSection);
  CKey rootKey;
  rootKey.Attach(HKEY_CLASSES_ROOT);
  rootKey.RecurseDeleteKey(GetExtProgramKeyName(extension));
  rootKey.Detach();
}

void DeleteShellExtensionInfo(const CSysString &extension)
{
  CSysString programKeyName;
  UString iconPath;
  int iconIndex;
  if (CheckShellExtensionInfo2(extension, programKeyName, iconPath, iconIndex))
    DeleteShellExtensionKey(extension);
  DeleteShellExtensionProgramKey(extension);
}

void AddShellExtensionInfo(const CSysString &extension,
    const UString &programTitle,
    const UString &programOpenCommand,
    const UString &iconPath, int iconIndex,
    const void *shellNewData, int shellNewDataSize)
{
  DeleteShellExtensionKey(extension);
  DeleteShellExtensionProgramKey(extension);
  NSynchronization::CCriticalSectionLock lock(g_CriticalSection);
  CSysString programKeyName;
  {
    CSysString ext = extension;
    if (iconIndex < 0)
      ext = TEXT("*");
    programKeyName = GetExtProgramKeyName(ext);
  }
  {
    CKey extKey;
    extKey.Create(HKEY_CLASSES_ROOT, GetExtensionKeyName(extension));
    extKey.SetValue(NULL, programKeyName);
    if (shellNewData != NULL)
    {
      CKey shellNewKey;
      shellNewKey.Create(extKey, kShellNewKeyName);
      shellNewKey.SetValue(kShellNewDataValueName, shellNewData, shellNewDataSize);
    }
  }
  CKey programKey;
  programKey.Create(HKEY_CLASSES_ROOT, programKeyName);
  programKey.SetValue(NULL, programTitle);
  {
    CKey iconKey;
    iconKey.Create(programKey, kDefaultIconKeyName);
    UString iconPathFull = iconPath;
    if (iconIndex < 0)
      iconIndex = 0;
    {
      iconPathFull += L",";
      wchar_t s[16];
      ConvertUInt32ToString(iconIndex, s);
      iconPathFull += s;
    }
    iconKey.SetValue(NULL, iconPathFull);
  }

  CKey shellKey;
  shellKey.Create(programKey, kShellKeyName);
  shellKey.SetValue(NULL, TEXT(""));

  CKey openKey;
  openKey.Create(shellKey, kOpenKeyName);
  openKey.SetValue(NULL, TEXT(""));
  
  CKey commandKey;
  commandKey.Create(openKey, kCommandKeyName);

  commandKey.SetValue(NULL, programOpenCommand);
  CSysString ArchiveType = extension;

  CKey shellexKey;
  shellexKey.Create(programKey,kShellexKeyName);
	
  CKey ContextMuen;
  ContextMuen.Create(shellexKey,kContextMenuHandlers);
  CKey CmClsid;
  CmClsid.Create(ContextMuen,kDropkExtensionCLSID);

  CKey Pskey;
  Pskey.Create(shellexKey,kPropertySheetHandlers);
  CKey PsClisd;
  PsClisd.Create(Pskey,kDropkExtensionCLSID);

  if (ArchiveType == TEXT("zip")||ArchiveType==TEXT("7z")||ArchiveType ==TEXT("rar")
	  || ArchiveType == TEXT("tar") ||ArchiveType == TEXT("jar")||ArchiveType == TEXT("xpi"))
  {
	  CKey openShellexKey;
	  openShellexKey.Create(shellexKey,kDropHandlerKeyName);
	  openShellexKey.SetValue(NULL,kDropkExtensionCLSID);
  }
}

///////////////////////////

}
