// ViewSettings.h

#include "StdAfx.h"
 
#include "Common/IntToString.h"
#include "Common/StringConvert.h"

#include "ViewSettings.h"
#include "Windows/Registry.h"
#include "Windows/Synchronization.h"

using namespace NWindows;
using namespace NRegistry;

#define REG_PATH_FM TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR")

static const TCHAR *kDeleteMode = TEXT("Extraction");
static const TCHAR *kCuPrefix = TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR") TEXT(STRING_PATH_SEPARATOR) TEXT("Extraction");

static const TCHAR *kCUBasePath = REG_PATH_FM;
static const TCHAR *kCulumnsKeyName = REG_PATH_FM TEXT(STRING_PATH_SEPARATOR) TEXT("Columns");
static const TCHAR *kVirusKillSoftwarePath = TEXT("VirusKill");

static const TCHAR *kPositionValueName = TEXT("Position");
static const TCHAR *kPanelsInfoValueName = TEXT("Panels");
static const TCHAR *kToolbars = TEXT("Toolbars");
static const TCHAR *kTreesize = TEXT("Treesize");

static const WCHAR *kPanelPathValueName = L"PanelPath";
static const WCHAR *kIconPath = L"IconPath";               //文件关联图标获取路径。
static const TCHAR *kListMode = TEXT("ListMode");
static const TCHAR *kTreeMode = TEXT("TreeMode");

static const TCHAR *kFolderHistoryValueName = TEXT("FolderHistory");
static const TCHAR *kFastFoldersValueName = TEXT("FolderShortcuts");
static const TCHAR *kCopyHistoryValueName = TEXT("CopyHistory");
static const TCHAR *kFastFoldersValueSign =	TEXT("FolderSign");//收藏夹注释
static const TCHAR *kThemeTitleValues = TEXT("ThemeTitle");//主题名称。
static const TCHAR *kThemeFolderName = TEXT("ThemeFolderName");//主题包文件名.
static const TCHAR *kRootKeyNmaeForContextMenu = TEXT("Menu");
static const TCHAR *kUpdateInfo           =TEXT("UpdateInfo"); //检查更新

static const TCHAR *kUpdateTime           =TEXT("UpdateTime"); //

static const UInt32 kColumnInfoSpecHeader = 12;
static const UInt32 kColumnHeaderSize = 12;

static const UInt32 kColumnInfoVersion = 1;

static NSynchronization::CCriticalSection g_CS;

class CTempOutBufferSpec
{
  CByteBuffer Buffer;
  UInt32 Size;
  UInt32 Pos;
public:
  operator const Byte *() const { return (const Byte *)Buffer; }
  void Init(UInt32 dataSize)
  {
    Buffer.SetCapacity(dataSize);
    Size = dataSize;
    Pos = 0;
  }
  void WriteByte(Byte value)
  {
    if (Pos >= Size)
      throw "overflow";
    ((Byte *)Buffer)[Pos++] = value;
  }
  void WriteUInt32(UInt32 value)
  {
    for (int i = 0; i < 4; i++)
    {
      WriteByte((Byte)value);
      value >>= 8;
    }
  }
  void WriteBool(bool value)
  {
    WriteUInt32(value ? 1 : 0);
  }
};

class CTempInBufferSpec
{
public:
  Byte *Buffer;
  UInt32 Size;
  UInt32 Pos;
  Byte ReadByte()
  {
    if (Pos >= Size)
      throw "overflow";
    return Buffer[Pos++];
  }
  UInt32 ReadUInt32()
  {
    UInt32 value = 0;
    for (int i = 0; i < 4; i++)
      value |= (((UInt32)ReadByte()) << (8 * i));
    return value;
  }
  bool ReadBool()
  {
    return (ReadUInt32() != 0);
  }
};

void SaveListViewInfo(const UString &id, const CListViewInfo &viewInfo)
{
  const CObjectVector<CColumnInfo> &columns = viewInfo.Columns;
  CTempOutBufferSpec buffer;
  UInt32 dataSize = kColumnHeaderSize + kColumnInfoSpecHeader * columns.Size();
  buffer.Init(dataSize);

  buffer.WriteUInt32(kColumnInfoVersion);
  buffer.WriteUInt32(viewInfo.SortID);
  buffer.WriteBool(viewInfo.Ascending);
  for(int i = 0; i < columns.Size(); i++)
  {
    const CColumnInfo &column = columns[i];
    buffer.WriteUInt32(column.PropID);
    buffer.WriteBool(column.IsVisible);
    buffer.WriteUInt32(column.Width);
  }
  {
    NSynchronization::CCriticalSectionLock lock(g_CS);
    CKey key;
    key.Create(HKEY_CURRENT_USER, kCulumnsKeyName);
    key.SetValue(GetSystemString(id), (const Byte *)buffer, dataSize);
  }
}

void ReadListViewInfo(const UString &id, CListViewInfo &viewInfo)
{
  viewInfo.Clear();
  CObjectVector<CColumnInfo> &columns = viewInfo.Columns;
  CByteBuffer buffer;
  UInt32 size;
  {
    NSynchronization::CCriticalSectionLock lock(g_CS);
    CKey key;
    if (key.Open(HKEY_CURRENT_USER, kCulumnsKeyName, KEY_READ) != ERROR_SUCCESS)
      return;
    if (key.QueryValue(GetSystemString(id), buffer, size) != ERROR_SUCCESS)
      return;
  }
  if (size < kColumnHeaderSize)
    return;
  CTempInBufferSpec inBuffer;
  inBuffer.Size = size;
  inBuffer.Buffer = (Byte *)buffer;
  inBuffer.Pos = 0;


  UInt32 version = inBuffer.ReadUInt32();
  if (version != kColumnInfoVersion)
    return;
  viewInfo.SortID = inBuffer.ReadUInt32();
  viewInfo.Ascending = inBuffer.ReadBool();

  size -= kColumnHeaderSize;
  if (size % kColumnInfoSpecHeader != 0)
    return;
  int numItems = size / kColumnInfoSpecHeader;
  columns.Reserve(numItems);
  for(int i = 0; i < numItems; i++)
  {
    CColumnInfo columnInfo;
    columnInfo.PropID = inBuffer.ReadUInt32();
    columnInfo.IsVisible = inBuffer.ReadBool();
    columnInfo.Width = inBuffer.ReadUInt32();
    columns.Add(columnInfo);
  }
}

static const UInt32 kWindowPositionHeaderSize = 5 * 4;
static const UInt32 kPanelsInfoHeaderSize = 3 * 4;


void SaveWindowSize(const RECT &rect, bool maximized)
{
  CSysString keyName = kCUBasePath;
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  key.Create(HKEY_CURRENT_USER, keyName);
  CTempOutBufferSpec buffer;
  buffer.Init(kWindowPositionHeaderSize);
  buffer.WriteUInt32(rect.left);
  buffer.WriteUInt32(rect.top);
  buffer.WriteUInt32(rect.right);
  buffer.WriteUInt32(rect.bottom);
  buffer.WriteBool(maximized);
  key.SetValue(kPositionValueName, (const Byte *)buffer, kWindowPositionHeaderSize);
}

bool ReadWindowSize(RECT &rect, bool &maximized)
{
  CSysString keyName = kCUBasePath;
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, keyName, KEY_READ) != ERROR_SUCCESS)
    return false;
  CByteBuffer buffer;
  UInt32 size;
  if (key.QueryValue(kPositionValueName, buffer, size) != ERROR_SUCCESS)
    return false;
  if (size != kWindowPositionHeaderSize)
    return false;
  CTempInBufferSpec inBuffer;
  inBuffer.Size = size;
  inBuffer.Buffer = (Byte *)buffer;
  inBuffer.Pos = 0;
  rect.left = inBuffer.ReadUInt32();
  rect.top = inBuffer.ReadUInt32();
  rect.right = inBuffer.ReadUInt32();
  rect.bottom = inBuffer.ReadUInt32();
  maximized = inBuffer.ReadBool();
  return true;
}

void SavePanelsInfo(UInt32 numPanels, UInt32 currentPanel, UInt32 splitterPos)
{
  CSysString keyName = kCUBasePath;
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  key.Create(HKEY_CURRENT_USER, keyName);

  CTempOutBufferSpec buffer;
  buffer.Init(kPanelsInfoHeaderSize);
  buffer.WriteUInt32(numPanels);
  buffer.WriteUInt32(currentPanel);
  buffer.WriteUInt32(splitterPos);
  key.SetValue(kPanelsInfoValueName, (const Byte *)buffer, kPanelsInfoHeaderSize);
}

bool ReadPanelsInfo(UInt32 &numPanels, UInt32 &currentPanel, UInt32 &splitterPos)
{
  CSysString keyName = kCUBasePath;
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, keyName, KEY_READ) != ERROR_SUCCESS)
    return false;
  CByteBuffer buffer;
  UInt32 size;
  if (key.QueryValue(kPanelsInfoValueName, buffer, size) != ERROR_SUCCESS)
    return false;
  if (size != kPanelsInfoHeaderSize)
    return false;
  CTempInBufferSpec inBuffer;
  inBuffer.Size = size;
  inBuffer.Buffer = (Byte *)buffer;
  inBuffer.Pos = 0;
  numPanels = inBuffer.ReadUInt32();
  currentPanel = inBuffer.ReadUInt32();
  splitterPos = inBuffer.ReadUInt32();
  return true;
}

void SaveToolbarsMask(UInt32 toolbarMask)
{
  
  CKey key;
  key.Create(HKEY_CURRENT_USER, kCUBasePath);
  key.SetValue(kToolbars, toolbarMask);
}

static const UInt32 kDefaultToolbarMask = ((UInt32)1 << 31) | 8 | 4 | 1;

UInt32 ReadToolbarsMask()
{
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) != ERROR_SUCCESS)
    return kDefaultToolbarMask;
  UInt32 mask;
  if (key.QueryValue(kToolbars, mask) != ERROR_SUCCESS)
    return kDefaultToolbarMask;
  return mask;
}

//记录树视图位置相对初始位置的偏移量
void SaveTreeMoveSize(UInt32 x)
{

	CKey key;
	key.Create(HKEY_CURRENT_USER, kCUBasePath);
	key.SetValue(kTreesize, x);
}
bool ReadTreeMoveSize(UInt32 &x)
{
	CKey key;
	if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) != ERROR_SUCCESS)
		return false;
	if (key.QueryValue(kTreesize, x) != ERROR_SUCCESS)
		return false;
	return true;
}

static UString GetPanelPathName(UInt32 panelIndex)
{
  WCHAR panelString[16];
  ConvertUInt32ToString(panelIndex, panelString);
  return UString(kPanelPathValueName) + panelString;
}


void SavePassWord(UString defaultPassword)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	key.Create(HKEY_CURRENT_USER, kCUBasePath);
	key.SetValue(L"passwordSet", defaultPassword);
}
bool ReadPassWord(UString &defaultPassword)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) != ERROR_SUCCESS)
		return false;
	key.QueryValue(L"passwordSet", defaultPassword);
	return true;
}
void SavePanelPath(UInt32 panel, const UString &path)
{
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  key.Create(HKEY_CURRENT_USER, kCUBasePath);
  key.SetValue(GetPanelPathName(panel), path);
}

bool ReadPanelPath(UInt32 panel, UString &path)
{
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) != ERROR_SUCCESS)
    return false;
  return (key.QueryValue(GetPanelPathName(panel), path) == ERROR_SUCCESS);
}

void SaveListMode(const CListMode &listMode)
{
  CKey key;
  key.Create(HKEY_CURRENT_USER, kCUBasePath);
  UInt32 t = 0;
  for (int i = 0; i < 2; i++)
    t |= ((listMode.Panels[i]) & 0xFF) << (i * 8);
  key.SetValue(kListMode, t);
}

void SaveTreeMode(const CTreeMode &TreeMode)
{
	CKey key;
	key.Create(HKEY_CURRENT_USER, kCUBasePath);
	UInt32 t = 0;
	for (int i = 0; i < 2; i++)
		t |= ((TreeMode.Panels[i]) & 0xFF) << (i * 8);
	key.SetValue(kTreeMode, t);
}

void ReadListMode(CListMode &listMode)
{
  CKey key;
  listMode.Init();
  if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) != ERROR_SUCCESS)
    return;
  UInt32 t;
  if (key.QueryValue(kListMode, t) != ERROR_SUCCESS)
    return;
  for (int i = 0; i < 2; i++)
  {
    listMode.Panels[i] = (t & 0xFF);
    t >>= 8;
  }
}

void ReadTreeMode(CTreeMode &treeMode)
{
	CKey key;
	treeMode.Init();
	if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) != ERROR_SUCCESS)
		return;
	UInt32 t;
	if (key.QueryValue(kTreeMode, t) != ERROR_SUCCESS)
		return;
	for (int i = 0; i < 2; i++)
	{
		treeMode.Panels[i] = (t & 0xFF);
		t >>= 8;
	}
}


static void SaveStringList(LPCTSTR valueName, const UStringVector &folders)
{
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  key.Create(HKEY_CURRENT_USER, kCUBasePath);
  key.SetValue_Strings(valueName, folders);
}

static void ReadStringList(LPCTSTR valueName, UStringVector &folders)
{
  folders.Clear();
  NSynchronization::CCriticalSectionLock lock(g_CS);
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) == ERROR_SUCCESS)
    key.GetValue_Strings(valueName, folders);
}


void SaveFolderHistory(const UStringVector &folders)
  { SaveStringList(kFolderHistoryValueName, folders); }
void ReadFolderHistory(UStringVector &folders)
  { ReadStringList(kFolderHistoryValueName, folders); }

void SaveFastFolders(const UStringVector &folders)
  { SaveStringList(kFastFoldersValueName, folders); }
void ReadFastFolders(UStringVector &folders)
  { ReadStringList(kFastFoldersValueName, folders); }

void SaveFastFoldersSign(const UStringVector &folders)//存取收藏夹注释
{SaveStringList(kFastFoldersValueSign, folders);    }
void ReadFastFoldersSign(UStringVector &folders)
{ReadStringList(kFastFoldersValueSign, folders);    }

void SaveThemeTitle(const UStringVector &themeStr)//主题名称。
{
	SaveStringList(kThemeTitleValues,themeStr);
}
void ReadThemeTitle(UStringVector &themeStr)
{
	ReadStringList(kThemeTitleValues,themeStr);
}
void SaveThemeFolderName(const UStringVector &ThemeFolderName)//主题包文件夹名.
{
	SaveStringList(kThemeFolderName,ThemeFolderName);
}
void ReadThemeFolderName(UStringVector &ThemeFolderName)
{
	ReadStringList(kThemeFolderName,ThemeFolderName);
}

void SaveCopyHistory(const UStringVector &folders)
  { SaveStringList(kCopyHistoryValueName, folders); }
void ReadCopyHistory(UStringVector &folders)
  { ReadStringList(kCopyHistoryValueName, folders); }

void AddUniqueStringToHeadOfList(UStringVector &list, const UString &s)
{
  for (int i = 0; i < list.Size();)
    if (s.CompareNoCase(list[i]) == 0)
      list.Delete(i);
    else
      i++;
  list.Insert(0, s);
}
void SaveKillVirSoftwarePath(const UString &path)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	key.Create(HKEY_CURRENT_USER, kVirusKillSoftwarePath );
	key.SetValue(L"VirKillSoftware", path);
}

void SaveUpdateTime(const UString &path)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	key.Create(HKEY_CURRENT_USER, kUpdateTime );
	key.SetValue(L"UpdateTime", path);
}

bool ReadUpdateTime(UString &path)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	if (key.Open(HKEY_CURRENT_USER, kUpdateTime , KEY_READ) != ERROR_SUCCESS)
		return false;
	return (key.QueryValue(L"UpdateTime", path) == ERROR_SUCCESS);
}


bool ReadContextMenu(UString &path)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	if (key.Open(HKEY_CURRENT_USER, kRootKeyNmaeForContextMenu , KEY_READ) != ERROR_SUCCESS)
		return false;
	return (key.QueryValue(L"Menu", path) == ERROR_SUCCESS);
}

void SaveContextMenu(UString path)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	key.Create(HKEY_CURRENT_USER, kRootKeyNmaeForContextMenu );
	key.SetValue(L"Menu", path);

}


bool ReadPanelPath(UString &path)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	if (key.Open(HKEY_CURRENT_USER, kVirusKillSoftwarePath , KEY_READ) != ERROR_SUCCESS)
		return false;
	return (key.QueryValue(L"VirKillSoftware", path) == ERROR_SUCCESS);
}
void SaveIconPath(const UString &path)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	key.Create(HKEY_CURRENT_USER, kCUBasePath);
	key.SetValue(kIconPath, path);
}

bool ReadIconPath(UString &path)
{
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) != ERROR_SUCCESS)
		return false;
	return (key.QueryValue(kIconPath, path) == ERROR_SUCCESS);
}
bool ReadDeleteMode(UInt32 &deletemode)
{
	deletemode =1;//读取注册表中删除源文件的信息
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	if (key.Open(HKEY_CURRENT_USER, kCuPrefix, KEY_READ) != ERROR_SUCCESS)
		return false;
	return (key.QueryValue(kDeleteMode, deletemode) == ERROR_SUCCESS);
}