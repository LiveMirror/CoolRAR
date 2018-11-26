// RegistryContextMenu.cpp

#include "StdAfx.h"

#include "Windows/Registry.h"
#include "Windows/Synchronization.h"

#include "RegistryContextMenu.h"

using namespace NWindows;
using namespace NRegistry;

namespace NZipRootRegistry {
  
#ifndef UNDER_CE

static NSynchronization::CCriticalSection g_CS;
  
static const TCHAR *kContextMenuKeyName  = TEXT("\\shellex\\ContextMenuHandlers\\CoolRAR");
static const TCHAR *kDragDropMenuKeyName = TEXT("\\shellex\\DragDropHandlers\\CoolRAR");
//×¢²á×ÀÃæÍÏ·Å¡£
// static const TCHAR *kDragDropHandershell = TEXT("\\CoolRAR\\shell\\open\\command");
// static const TCHAR *kDragDropHandershellexCM = TEXT("\\CoolRAR\\shellex\\ContextMenuHandlers\\{23170F69-40C1-278A-1000-000100020001}");
// static const TCHAR *kDragDropHandershellexPS = TEXT("\\CoolRAR\\shellex\\PropertySheetHandlers\\{23170F69-40C1-278A-1000-000100020001}");
// static const TCHAR *kDefaultIcon = TEXT("\\CoolRAR\\DefaultIcon");
//static const TCHAR *kcommand = TEXT("command");
//static const TCHAR *kDropHandler = TEXT("\\CoolRAR\\shellex\\DropHandler");

static const TCHAR *kExtensionCLSID = TEXT("{23170F69-40C1-278A-1000-000100020001}");

static const TCHAR *kRootKeyNameForFile = TEXT("*");
static const TCHAR *kRootKeyNameForFolder = TEXT("Folder");
static const TCHAR *kRootKeyNameForDirectory = TEXT("Directory");
static const TCHAR *kRootKeyNameForDrive = TEXT("Drive");



static CSysString GetFullContextMenuKeyName(const CSysString &keyName)
  { return (keyName + kContextMenuKeyName); }

static CSysString GetFullDragDropMenuKeyName(const CSysString &keyName)
  { return (keyName + kDragDropMenuKeyName); }

static bool CheckHandlerCommon(const CSysString &keyName)
{
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  if (key.Open(HKEY_CLASSES_ROOT, keyName, KEY_READ) != ERROR_SUCCESS)
    return false;
  CSysString value;
  if (key.QueryValue(NULL, value) != ERROR_SUCCESS)
    return false;
  value.MakeUpper();
  return (value.Compare(kExtensionCLSID) == 0);
}

bool CheckContextMenuHandler()
{
  return
    CheckHandlerCommon(GetFullContextMenuKeyName(kRootKeyNameForDirectory))  &&
    CheckHandlerCommon(GetFullContextMenuKeyName(kRootKeyNameForFile)) &&
    CheckHandlerCommon(GetFullDragDropMenuKeyName(kRootKeyNameForDirectory))  &&
    CheckHandlerCommon(GetFullDragDropMenuKeyName(kRootKeyNameForDrive));
}
bool CheckDragDropHandler()
{
	return
		CheckHandlerCommon(GetFullDragDropMenuKeyName(kRootKeyNameForFolder));
}
static void DeleteContextMenuHandlerCommon(const CSysString &keyName)
{
  CKey rootKey;
  rootKey.Attach(HKEY_CLASSES_ROOT);
  rootKey.RecurseDeleteKey(GetFullContextMenuKeyName(keyName));
  rootKey.Detach();
}

static void DeleteDragDropMenuHandlerCommon(const CSysString &keyName)
{
  CKey rootKey;
  rootKey.Attach(HKEY_CLASSES_ROOT);
  rootKey.RecurseDeleteKey(GetFullDragDropMenuKeyName(keyName));
  rootKey.Detach();
}

void DeleteContextMenuHandler()
{
  DeleteContextMenuHandlerCommon(kRootKeyNameForFile);
  DeleteContextMenuHandlerCommon(kRootKeyNameForFolder);
  DeleteContextMenuHandlerCommon(kRootKeyNameForDirectory);
  DeleteContextMenuHandlerCommon(kRootKeyNameForDrive);
  DeleteDragDropMenuHandlerCommon(kRootKeyNameForFile);
  DeleteDragDropMenuHandlerCommon(kRootKeyNameForFolder);
  DeleteDragDropMenuHandlerCommon(kRootKeyNameForDirectory);
  DeleteDragDropMenuHandlerCommon(kRootKeyNameForDrive);
}

static void AddContextMenuHandlerCommon(const CSysString &keyName)
{
  DeleteContextMenuHandlerCommon(keyName);
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  key.Create(HKEY_CLASSES_ROOT, GetFullContextMenuKeyName(keyName));
  key.SetValue(NULL, kExtensionCLSID);
}

static void AddDragDropMenuHandlerCommon(const CSysString &keyName)
{
  DeleteDragDropMenuHandlerCommon(keyName);
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  key.Create(HKEY_CLASSES_ROOT, GetFullDragDropMenuKeyName(keyName));
  key.SetValue(NULL, kExtensionCLSID);
}
// static void AddDropHandlerDefaultIcon()
// {
// 	NSynchronization::CCriticalSectionLock lock(g_CS);
// 	CKey key;
// 	key.Create(HKEY_CLASSES_ROOT,kDefaultIcon);
// 	key.SetValue(NULL,"C:\Program Files\CoolRAR\CoolRAR.exe,0");
// }
// static void AddDropHandlershell()
// {
// 	NSynchronization::CCriticalSectionLock lock(g_CS);
// 	CKey key;
// 	key.Create(HKEY_CLASSES_ROOT,kDragDropHandershell);
// 	key.SetValue(NULL,"C:\Program Files\CoolRAR\CoolRAR.exe");
// }
// static void AddDropHandlershellex()
// {
// 	NSynchronization::CCriticalSectionLock lock(g_CS);
// 	CKey key,keyps,keyDH;
// 	key.Create(HKEY_CLASSES_ROOT,kDragDropHandershellexCM);
// 	//key.SetValue(NULL,NULL);
// 	keyps.Create(HKEY_CLASSES_ROOT,kDragDropHandershellexPS);
// 	//keyps.SetValue(NULL,NULL);
// 	keyDH.Create(HKEY_CLASSES_ROOT,kDropHandler);
// 	keyDH.SetValue(NULL,kExtensionCLSID);
// }
void AddContextMenuHandler()
{
  AddContextMenuHandlerCommon(kRootKeyNameForFile);
  AddContextMenuHandlerCommon(kRootKeyNameForDirectory);
	
  AddDragDropMenuHandlerCommon(kRootKeyNameForDirectory);
  AddDragDropMenuHandlerCommon(kRootKeyNameForDrive);
  AddDragDropMenuHandlerCommon(kRootKeyNameForFolder);
}

// void AddDropHandler()
// {
// 	AddDropHandlerDefaultIcon();
// 	AddDropHandlershell();
// 	AddDropHandlershellex();
// 
// }

#endif

}
