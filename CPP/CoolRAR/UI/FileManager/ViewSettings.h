// ViewSettings.h

#ifndef __VIEW_SETTINGS_H
#define __VIEW_SETTINGS_H

#include "Common/MyString.h"
#include "Common/Types.h"

struct CColumnInfo
{
  PROPID PropID;
  bool IsVisible;
  UInt32 Width;
};

inline bool operator==(const CColumnInfo &a1, const CColumnInfo &a2)
{
  return (a1.PropID == a2.PropID) &&
    (a1.IsVisible == a2.IsVisible) && (a1.Width == a2.Width);
}

inline bool operator!=(const CColumnInfo &a1, const CColumnInfo &a2)
{
  return !(a1 == a2);
}

struct CListViewInfo
{
  CObjectVector<CColumnInfo> Columns;
  PROPID SortID;
  bool Ascending;

  void Clear()
  {
    SortID = 0;
    Ascending = true;
    Columns.Clear();
  }

  int FindColumnWithID(PROPID propID) const
  {
    for (int i = 0; i < Columns.Size(); i++)
      if (Columns[i].PropID == propID)
        return i;
    return -1;
  }

  bool IsEqual(const CListViewInfo &info) const
  {
    if (Columns.Size() != info.Columns.Size() ||
        SortID != info.SortID ||
        Ascending != info.Ascending)
      return false;
    for (int i = 0; i < Columns.Size(); i++)
      if (Columns[i] != info.Columns[i])
        return false;
    return true;
  }
};

void SaveListViewInfo(const UString &id, const CListViewInfo &viewInfo);
void ReadListViewInfo(const UString &id, CListViewInfo &viewInfo);

void SaveWindowSize(const RECT &rect, bool maximized);
bool ReadWindowSize(RECT &rect, bool &maximized);

void SavePanelsInfo(UInt32 numPanels, UInt32 currentPanel, UInt32 splitterPos);
bool ReadPanelsInfo(UInt32 &numPanels, UInt32 &currentPanel, UInt32 &splitterPos);

void SaveToolbarsMask(UInt32 toolbarMask);
UInt32 ReadToolbarsMask();

void SaveTreeMoveSize(UInt32 x);
bool ReadTreeMoveSize(UInt32 &x);

void SavePanelPath(UInt32 panel, const UString &path);
bool ReadPanelPath(UInt32 panel, UString &path);
bool ReadPassWord(UString &defaultPassword);
void SavePassWord(UString defaultPassword);

struct CListMode
{
  UInt32 Panels[2];
  void Init() { Panels[0] = Panels[1] = 3; }
  CListMode() { Init(); }
};

struct CTreeMode
{
	UInt32 Panels[2];
	void Init() { Panels[0] = Panels[1] = 0; }
	CTreeMode() { Init(); }
};


void SaveListMode(const CListMode &listMode);
void ReadListMode(CListMode &listMode);


void SaveTreeMode(const CTreeMode &treeMode);
void ReadTreeMode(CTreeMode &treeMode);


void SaveFolderHistory(const UStringVector &folders);
void ReadFolderHistory(UStringVector &folders);

void SaveFastFolders(const UStringVector &folders);
void ReadFastFolders(UStringVector &folders);
void SaveFastFoldersSign(const UStringVector &folders);
void ReadFastFoldersSign(UStringVector &folders);

void SaveThemeTitle(const UStringVector &themeStr);
void ReadThemeTitle(UStringVector &themeStr);
void SaveThemeFolderName(const UStringVector &ThemeFolderName);
void ReadThemeFolderName(UStringVector &ThemeFolderName);

void SaveCopyHistory(const UStringVector &folders);
void ReadCopyHistory(UStringVector &folders);

void AddUniqueStringToHeadOfList(UStringVector &list, const UString &s);

void SaveKillVirSoftwarePath(const UString &path);
bool ReadPanelPath(UString &path);

void SaveContextMenu(UString path);
bool ReadContextMenu(UString &path);


void SaveUpdateTime(const UString &path);
bool ReadUpdateTime(UString &path);

void SaveIconPath(const UString &path);
bool ReadIconPath(UString &path);

bool ReadDeleteMode(UInt32 &deletemode);

#endif
