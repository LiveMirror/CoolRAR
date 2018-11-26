// Panel.h

#ifndef __PANEL_H
#define __PANEL_H

#include "../../../../C/Alloc.h"

#include "Common/MyCom.h"

#include "Windows/DLL.h"
#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/Handle.h"
#include "Windows/Synchronization.h"

#include "Windows/Control/ComboBox.h"
#include "Windows/Control/Edit.h"
#include "Windows/Control/ListView.h"
#include "Windows/Control/TreeView.h"
#include "Windows/Control/ReBar.h"
#include "Windows/Control/Static.h"
#include "Windows/Control/StatusBar.h"
#include "Windows/Control/ToolBar.h"
#include "Windows/Control/Window2.h"

#include "../Common/ExtractMode.h"
#include "AppState.h"
#include "IFolder.h"
#include "MyCom2.h"
#include "ProgressDialog2.h"
#include "SysIconUtils.h"


const int kParentFolderID = 100;
const int kPluginMenuStartID = 1000;
const int kToolbarStartID = 2000;

const int kParentIndex = -1;

#ifdef UNDER_CE
#define ROOT_FS_FOLDER L"\\"
#else
#define ROOT_FS_FOLDER L"C:\\\\"
#endif



struct TreePathStruct
{
	UString FilePath;
	HTREEITEM Root;
};
typedef CObjectVector<TreePathStruct> TreePathStructVector;

struct DesktopPathStruct
{
	UString FilePath;
	UString FileName;
};
typedef CObjectVector<DesktopPathStruct> DesktopPathStructVector;

struct CPanelCallback
{
  virtual void OnTab() = 0;
  virtual void SetFocusToPath(int index) = 0;
  virtual void OnCopy(bool move, bool copyToSame) = 0;
  virtual void OnSetSameFolder() = 0;
  virtual void OnSetSubFolder() = 0;
  virtual void PanelWasFocused() = 0;
  virtual void DragBegin() = 0;
  virtual void DragEnd() = 0;
  virtual void RefreshTitle(bool always) = 0;
};

void PanelCopyItems();

struct CItemProperty
{
  UString Name;
  PROPID ID;
  VARTYPE Type;
  int Order;
  bool IsVisible;
  UInt32 Width;
};

inline bool operator<(const CItemProperty &a1, const CItemProperty &a2)
  { return (a1.Order < a2.Order); }

inline bool operator==(const CItemProperty &a1, const CItemProperty &a2)
  { return (a1.Order == a2.Order); }

class CItemProperties: public CObjectVector<CItemProperty>
{
public:
  int FindItemWithID(PROPID id)
  {
    for (int i = 0; i < Size(); i++)
      if ((*this)[i].ID == id)
        return i;
    return -1;
  }
};

struct CTempFileInfo
{
  UString ItemName;
  UString FolderPath;
  UString FilePath;
  NWindows::NFile::NFind::CFileInfoW FileInfo;
  bool NeedDelete;

  CTempFileInfo(): NeedDelete(false) {}
  void DeleteDirAndFile() const
  {
    if (NeedDelete)
    {
      NWindows::NFile::NDirectory::DeleteFileAlways(FilePath);
      NWindows::NFile::NDirectory::MyRemoveDirectory(FolderPath);
    }
  }
  bool WasChanged(const NWindows::NFile::NFind::CFileInfoW &newFileInfo) const
  {
    return newFileInfo.Size != FileInfo.Size ||
        CompareFileTime(&newFileInfo.MTime, &FileInfo.MTime) != 0;
  }
};

struct CFolderLink: public CTempFileInfo
{
  NWindows::NDLL::CLibrary Library;
  CMyComPtr<IFolderFolder> ParentFolder;
  bool UsePassword;
  UString Password;
  bool IsVirtual;

  UString VirtualPath;
  CFolderLink(): UsePassword(false), IsVirtual(false) {}

  bool WasChanged(const NWindows::NFile::NFind::CFileInfoW &newFileInfo) const
  {
    return IsVirtual || CTempFileInfo::WasChanged(newFileInfo);
  }

};

enum MyMessages
{
  kShiftSelectMessage = WM_USER + 1,
  kReLoadMessage,
  kSetFocusToListView,
  kOpenItemChanged,
  kRefreshStatusBar,
  kRefreshHeaderComboBox
};

UString GetFolderPath(IFolderFolder * folder);

class CPanel;

class CMyListView: public NWindows::NControl::CListView
{
public:
  WNDPROC _origWindowProc;
  CPanel *_panel;
  LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
};

class CMyStatusBar: public NWindows::NControl::CStatusBar
{
public:
	WNDPROC _origWindowProc;
	CPanel *_panel;
	LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
};


class CMyTreeView: public NWindows::NControl::CTreeView
{
public:
	WNDPROC _origWindowProc;
	CPanel *_panel;
	LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
};

class CMyComboBoxEdit: public NWindows::NControl::CEdit
{
public:
  WNDPROC _origWindowProc;
  CPanel *_panel;
  LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
};

struct CSelectedState
{
  int FocusedItem;
  UString FocusedName;
  bool SelectFocused;
  UStringVector SelectedNames;
  CSelectedState(): FocusedItem(-1), SelectFocused(false) {}
};

#ifdef UNDER_CE
#define MY_NMLISTVIEW_NMITEMACTIVATE NMLISTVIEW
#else
#define MY_NMLISTVIEW_NMITEMACTIVATE NMITEMACTIVATE
#endif

class CPanel: public NWindows::NControl::CWindow2
{
  CExtToIconMap _extToIconMap;
  UINT _baseID;
  int _comboBoxID;
  UINT _statusBarID;
  CRecordVector<UInt32>	_Copyindices;  //���Ƶ��ļ���Ϣ
  UStringVector _Copynames;            //���Ƶ��ļ���
  bool _needExtract;                   //��Ҫ��ѹ����
  UINT _TreeViewID;//����ͼid

  CAppState *_appState;

  bool OnCommand(int code, int itemID, LPARAM lParam, LRESULT &result);
  LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
  virtual bool OnCreate(CREATESTRUCT *createStruct);
  virtual bool OnSize(WPARAM wParam, int xSize, int ySize);
  virtual void OnDestroy();
  virtual bool OnNotify(UINT controlID, LPNMHDR lParam, LRESULT &result);

  void AddComboBoxItem(const UString &name, int iconIndex, int indent, bool addToList);
 

  bool OnComboBoxCommand(UINT code, LPARAM param, LRESULT &result);
  
  #ifndef UNDER_CE
  
  LRESULT OnNotifyComboBoxEnter(const UString &s);
  bool OnNotifyComboBoxEndEdit(PNMCBEENDEDITW info, LRESULT &result);
  #ifndef _UNICODE
  bool OnNotifyComboBoxEndEdit(PNMCBEENDEDIT info, LRESULT &result);
  #endif

  #endif

  bool OnNotifyReBar(LPNMHDR lParam, LRESULT &result);
  bool OnNotifyComboBox(LPNMHDR lParam, LRESULT &result);
  void OnItemChanged(NMLISTVIEW *item);
  void OnNotifyActivateItems();
  bool OnNotifyList(LPNMHDR lParam, LRESULT &result);
  bool OnNotifyTree(LPNMHDR header);
  void OnDrag(LPNMLISTVIEW nmListView);
  bool OnKeyDown(LPNMLVKEYDOWN keyDownInfo, LRESULT &result);
  BOOL OnBeginLabelEdit(LV_DISPINFOW * lpnmh);
  BOOL OnEndLabelEdit(LV_DISPINFOW * lpnmh);
  void OnColumnClick(LPNMLISTVIEW info);
  bool OnCustomDraw(LPNMLVCUSTOMDRAW lplvcd, LRESULT &result);
  

public:
  HWND _mainWindow;
  CPanelCallback *_panelCallback;

  //��ȡ�ļ����Ŀ¼��¼��Ŀ����
  void GetItemMaxCount(int &index);
  //�ж��ļ������Ƿ�Ϊѹ���ļ�
  bool JudgeFileType(UString sysPath);
  //�жϵ�ǰѡ�����ļ��Ƿ���ѹ���ļ�
   bool FileCopyJudge();

  void CopyFiles();//�����ļ�
  void PlasterFiles();//ճ���ļ�
  void UpdateThemeIco(UString ImagePath,int Fouse);//ˢ��listview��ico����
  void UpdateStausIco(UString ImagePath,int Fouse);
  
  
  void SysIconsWereChanged() { _extToIconMap.Clear(); }

  void DeleteItems(bool toRecycleBin);
  void DeleteItemsInternal(CRecordVector<UInt32> &indices);
  void CreateFolder();
  void CreateFile();

private:

  void ChangeWindowSize(int xSize, int ySize);
  void ChangTLSzie(int x);
  void InitColumns();
  void InsertColumn(int index);

  void SetFocusedSelectedItem(int index, bool select);
  void RefreshListCtrl(const UString &focusedName, int focusedPos, bool selectFocused,
      const UStringVector &selectedNames);

  void OnShiftSelectMessage();
  void OnArrowWithShift();

  void OnInsert();
public:
  void UpdateSelection();
  void SelectSpec(bool selectMode);
  void SelectByType(bool selectMode);
  void SelectAll(bool selectMode);
  void InvertSelection();
private:

  LRESULT SetItemText(LVITEMW &item);


public:
  NWindows::NControl::CReBar _headerReBar;
  NWindows::NControl::CToolBar _headerToolBar;
 
  NWindows::NControl::
    #ifdef UNDER_CE
    CComboBox
    #else
    CComboBoxEx
    #endif
    _headerComboBox;
  UStringVector ComboBoxPaths;
  CMyComboBoxEdit _comboBoxEdit;
  CMyListView _listView;
  CMyTreeView _treeView;//����ͼ

  TreePathStructVector treePathVector;

  DesktopPathStructVector DesktopPathVector;

  bool changTreeSize; //չ����ʽ

  void InitTreeList();//��ʼ������ͼ

  void InitTree();

  bool treeMove; 
  POINT oldPison;//��ǰ����ͼ�ֱ߽�����
  RECT oldRect;
  void OnInvertTracker(RECT rect);
  int _treeviewTop,_treeviewBottom;	//�����б�Ķ�������͵ײ�����
  UString GetTreeItem(HTREEITEM hItem);
	
  bool OnTreeClick(HTREEITEM hItem = NULL);

  HTREEITEM GetTreeItemHt(UString FilePath);

  void ShowTreeFile();

  HRESULT ParseDesktopPath(UString & DesktopfilePath);


  bool AddTree(UString Filepath,HTREEITEM  ParentRoot,bool Layers,bool PartBool);

  bool AddDriveTree(UString driveName,UString path,bool needchild,HTREEITEM  ParentRoot);
  CMyStatusBar _statusBar;
  bool _lastFocusedIsList;

  DWORD _exStyle;
  bool _showDots;
  bool _showRealFileIcons;
  bool _enableItemChangeNotify;
  bool _mySelectMode;
  bool _isInThePressFile;//�жϵ�ǰ�Ƿ���ѹ���ļ�Ŀ¼��
  CBoolVector _selectedStatusVector;

  CSelectedState _selectedState;

  HWND GetParent();

  UInt32 GetRealIndex(const LVITEMW &item) const
  {
    return (UInt32)item.lParam;
  }
  int GetRealItemIndex(int indexInListView) const
  {
    LPARAM param;
    if (!_listView.GetItemParam(indexInListView, param))
      throw 1;
    return (int)param;
  }

  UInt32 _ListViewMode;
  UInt32 _TreeViewMode;
  int _xSize;

  bool _flatMode;
  bool _flatModeForDisk;
  bool _flatModeForArc;

  bool _dontShowMode;


  UString _currentFolderPrefix;
  
  CObjectVector<CFolderLink> _parentFolders;
  NWindows::NDLL::CLibrary _library;
  CMyComPtr<IFolderFolder> _folder;

  UStringVector _fastFolders;


  void GetSelectedNames(UStringVector &selectedNames);
  void SaveSelectedState(CSelectedState &s);
  void RefreshListCtrl(const CSelectedState &s);
  void RefreshListCtrlSaveFocused();

  UString GetItemName(int itemIndex) const;
  UString GetItemPrefix(int itemIndex) const;
  UString GetItemRelPath(int itemIndex) const;
  UString GetItemFullPath(int itemIndex) const;
  bool IsItemFolder(int itemIndex) const;
  UInt64 GetItemSize(int itemIndex) const;

  ////////////////////////
  // PanelFolderChange.cpp

  void SetToRootFolder();
  HRESULT BindToPath(const UString &fullPath, bool &archiveIsOpened, bool &encrypted); // can be prefix
  HRESULT BindToPathAndRefresh(const UString &path);
  void OpenDrivesFolder();
  
  void GoToItem(UString name);//��λ�����������ļ�
  void SetBookmark(int index);
  void SetBookmark(UString string);
  void SetBookmark(const UStringVector &setstring);
  void GetBookmark(UStringVector &getstring);
  void SetSign(UString string,UString sign);
  void SetSign(const UStringVector &setSign);
  void GetSign(UStringVector &getSign);

  void OpenBookmark(int index);

  void SetTheme(int index);
  void SetTheme(const UStringVector &setstring);
  void SetTheme(const UString &string);
  void GetTheme(UStringVector &getstring);
  void OpenTheme(int index);
  void SetThemeFolderName(UString Name);
  void SetThemeFolderName(const UStringVector &FolderName);
  void GetThemeFolderName(UStringVector &getFolderName);
  
  void LoadFullPath();
  void LoadFullPathAndShow();
  void FoldersHistory();
  void OpenHistoryFile(int index);
  void OpenDirDriver(UString selectdrive);
  
  void GetOldFoldersHistory(int &index,UString &foldername);
  void FromChange();
  void FromChangeRealDo(UString karchivepath,UString kfrom,UString karchivename);
  void PressItself();
  void OpenParentFolder();
  void CloseOpenFolders();
  void OpenRootFolder();
  void GetLastFileName(UString & );
 



  HRESULT Create(HWND mainWindow, HWND parentWindow,
      UINT id,
      const UString &currentFolderPrefix,
      CPanelCallback *panelCallback,
      CAppState *appState, bool &archiveIsOpened, bool &encrypted);
  void SetFocusToList();
  void SetFocusToLastRememberedItem();


  void ReadListViewInfo();
  void SaveListViewInfo();

  void PanelInfoOut(UString strFileName);//�����ļ�
  void PanelInfoReadIn();

  CPanel() :
      _exStyle(0),
      _showDots(true),
      _showRealFileIcons(false),
      _needSaveInfo(false),
      _startGroupSelect(0),
      _selectionIsDefined(false),
      _ListViewMode(3),
	  _TreeViewMode(1),
      _flatMode(false),
      _flatModeForDisk(false),
      _flatModeForArc(false),
      _xSize(300),
      _mySelectMode(false),
      _enableItemChangeNotify(true),
      _dontShowMode(false)
  {}

  void SetExtendedStyle()
  {
    if (_listView != 0)
      _listView.SetExtendedListViewStyle(_exStyle);
  }


  bool _needSaveInfo;
  UString _typeIDString;
  CListViewInfo _listViewInfo;
  CItemProperties _properties;
  CItemProperties _visibleProperties;
  
  PROPID _sortID;
  bool _ascending;

  void Release();
  ~CPanel();
  void OnLeftClick(MY_NMLISTVIEW_NMITEMACTIVATE *itemActivate);
  bool OnRightClick(MY_NMLISTVIEW_NMITEMACTIVATE *itemActivate, LRESULT &result);
  void ShowColumnsContextMenu(int x, int y);

  void OnTimer();
  void OnReload();
  bool OnContextMenu(HANDLE windowHandle, int xPos, int yPos);
  bool CreatMenuByStatusBar(int xPos, int yPos);

  CMyComPtr<IContextMenu> _sevenZipContextMenu;
  CMyComPtr<IContextMenu> _systemContextMenu;
  HRESULT CreateShellContextMenu(
      const CRecordVector<UInt32> &operatedIndices,
      CMyComPtr<IContextMenu> &systemContextMenu);
  void CreateSystemMenu(HMENU menu,
      const CRecordVector<UInt32> &operatedIndices,
      CMyComPtr<IContextMenu> &systemContextMenu);
  void CreateSevenZipMenu(HMENU menu,
      const CRecordVector<UInt32> &operatedIndices,
      CMyComPtr<IContextMenu> &sevenZipContextMenu);
  void CreateFileMenu(HMENU menu,
      CMyComPtr<IContextMenu> &sevenZipContextMenu,
      CMyComPtr<IContextMenu> &systemContextMenu,
      bool programMenu);
  void CreateFileMenu(HMENU menu);
  bool InvokePluginCommand(int id);
  bool InvokePluginCommand(int id, IContextMenu *sevenZipContextMenu,
      IContextMenu *systemContextMenu);

  void InvokeSystemCommand(const char *command);
  void Properties();
  void EditCut();
  void EditCopy();
  void EditPaste();

  int _startGroupSelect;

  bool _selectionIsDefined;
  bool _selectMark;
  bool _deleteEnable;//�Ƿ����ʹ��ɾ����ݼ�
  int _prevFocusedItem;
  int _upArrangeIconIndex;
  int _downArrangeIconIndex;
 
  void SortItemsWithPropID(PROPID propID);
  //�ж��б�����˳�������
  bool ItemsListRuleJudge(PROPID propID);
  //�жϵ�ǰ·���Ƿ�Ϊѹ��·��
  bool NowPathJudge();
  //�жϵ�ǰѹ���ļ��Ƿ�ΪRAR�ļ�
  bool NowAcrhvieJudge();
  bool IsExeFile();
  void GetSelectedItemsIndices(CRecordVector<UInt32> &indices) const;
  void GetOperatedItemIndices(CRecordVector<UInt32> &indices) const;
  void GetAllItemIndices(CRecordVector<UInt32> &indices) const;
  void GetOperatedIndicesSmart(CRecordVector<UInt32> &indices) const;
  void KillSelection();

  UString GetFolderTypeID() const;
  bool IsRootFolder() const;
  bool IsFSFolder() const;
  bool IsFSDrivesFolder() const;
  bool IsFsOrDrivesFolder() const { return IsFSFolder() || IsFSDrivesFolder(); }
  bool IsDeviceDrivesPrefix() const { return _currentFolderPrefix == L"\\\\.\\"; }
  bool IsFsOrPureDrivesFolder() const { return IsFSFolder() || (IsFSDrivesFolder() && !IsDeviceDrivesPrefix()); }

  UString GetFsPath() const;
  UString GetDriveOrNetworkPrefix() const;

  bool DoesItSupportOperations() const;

  bool _processTimer;
  bool _processNotify;

  class CDisableTimerProcessing
  {
    bool _processTimerMem;
    bool _processNotifyMem;

    CPanel &_panel;
    public:

    CDisableTimerProcessing(CPanel &panel): _panel(panel)
    {
      Disable();
    }
    void Disable()
    {
      _processTimerMem = _panel._processTimer;
      _processNotifyMem = _panel._processNotify;
      _panel._processTimer = false;
      _panel._processNotify = false;
    }
    void Restore()
    {
      _panel._processTimer = _processTimerMem;
      _panel._processNotify = _processNotifyMem;
    }
    ~CDisableTimerProcessing()
    {
      Restore();
    }
    CDisableTimerProcessing& operator=(const CDisableTimerProcessing &) {; }
  };

  
  void RefreshListCtrl();

  void MessageBoxInfo(LPCWSTR message, LPCWSTR caption);
  void MessageBox(LPCWSTR message);
  void MessageBox(LPCWSTR message, LPCWSTR caption);
  void MessageBoxMyError(LPCWSTR message);
  void MessageBoxError(HRESULT errorCode, LPCWSTR caption);
  void MessageBoxError(HRESULT errorCode);
  void MessageBoxLastError(LPCWSTR caption);
  void MessageBoxLastError();

  void MessageBoxErrorForUpdate(HRESULT errorCode, UINT resourceID, UInt32 langID);

  void MessageBoxErrorLang(UINT resourceID, UInt32 langID);

  void OpenFocusedItemAsInternal();
  void OpenSelectedItems(bool internal);
 void SearchFilename();
 void SeaVirus();

  void OpenFolderExternal(int index);

  void OpenFolder(int index);
  void OpenRealeaseFile();
  void GetNowPath(UString &nowPath);
  HRESULT OpenParentArchiveFolder();
  HRESULT OpenItemAsArchive(IInStream *inStream,
      const CTempFileInfo &tempFileInfo,
      const UString &virtualFilePath,
      bool &encrypted);
  HRESULT JudgeOpenItemAsArchive(IInStream *inStream,
	  const CTempFileInfo &tempFileInfo,
	  const UString &virtualFilePath,
	  bool &encrypted);
  HRESULT OpenItemAsArchive(const UString &name, bool &encrypted);
  HRESULT OpenItemAsArchive(const UString &name,const UString &dirname, bool &encrypted);
  HRESULT OpenArchiveByFind(const UString &name,const UString &dirname,bool &enercypted); //FIND�е���

  HRESULT OpenItemAsArchive(int index);
  void OpenItemInArchive(int index, bool tryInternal, bool tryExternal,
      bool editMode);
  HRESULT OnOpenItemChanged(const UString &folderPath, const UString &itemName, bool usePassword, const UString &password);
  LRESULT OnOpenItemChanged(LPARAM lParam);

  void OpenItem(int index, bool tryInternal, bool tryExternal);
 
  void EditItem();
  void EditItem(int index);
  void PrintFile(); //��ӡ�ļ�

  void RenameFile();
  void ChangeComment();

  void SetListViewMode(UInt32 index);
  bool ListViewModeJudge();
  bool TreeViewModeJudge();
  void ChangTreeViewMode();
  UInt32 GetListViewMode() const { return _ListViewMode; }
  UInt32 GetTreeViewMode() const { return _TreeViewMode;}
  PROPID GetSortID() const { return _sortID; }

  void ChangeFlatMode();
  bool GetFlatMode() const { return _flatMode; }

  void RefreshStatusBar();
  void OnRefreshStatusBar();

  void AddToArchive();

  void GetFilePaths(const CRecordVector<UInt32> &indices, UStringVector &paths);
  //ֱ�ӽ�ѹ����
  void ExtractDirectly();
  void ExtractTheme(UStringVector paths,UString PathName);//��ѹ�������
  void ExtractArchives(); 
  //��ѹ���ļ���ѡ���Խ�ѹ��	
  void ExtractArchivesFileByChoose();
  void TestArchives();

  HRESULT CopyTo(const CRecordVector<UInt32> &indices, const UString &folder,
      bool moveMode, bool showErrorMessages, UStringVector *messages,
      bool &usePassword, UString &password);

  HRESULT CopyTo(const CRecordVector<UInt32> &indices, const UString &folder,
      bool moveMode, bool showErrorMessages, UStringVector *messages)
  {
    bool usePassword = false;
    UString password;
    if (_parentFolders.Size() > 0)
    {
      const CFolderLink &fl = _parentFolders.Back();
      usePassword = fl.UsePassword;
      password = fl.Password;
    }
    return CopyTo(indices, folder, moveMode, showErrorMessages, messages, usePassword, password);
  }

  HRESULT CopyTo(const CRecordVector<UInt32> &indices, const UString &folder,
	  bool moveMode, bool showErrorMessages, UStringVector *messages,
	  bool &usePassword, UString &password,NExtract::NOverwriteMode::EEnum overwritemode);
  HRESULT CopyTo(const CRecordVector<UInt32> &indices, const UString &folder,
	  bool moveMode, bool showErrorMessages, UStringVector *messages,NExtract::NOverwriteMode::EEnum overwritemode)
  {
	  bool usePassword = false;
	  UString password;
	  if (_parentFolders.Size() > 0)
	  {
		  const CFolderLink &fl = _parentFolders.Back();
		  usePassword = fl.UsePassword;
		  password = fl.Password;
	  }
	  return CopyTo(indices, folder, moveMode, showErrorMessages, messages, usePassword, password,overwritemode);
  }

  HRESULT CopyFrom(const UString &folderPrefix, const UStringVector &filePaths,
      bool showErrorMessages, UStringVector *messages);

  void CopyFromNoAsk(const UStringVector &filePaths);
  void CopyFromAsk(const UStringVector &filePaths);

  // empty folderPath means create new Archive to path of first fileName.
  void DropObject(IDataObject * dataObject, const UString &folderPath);

  // empty folderPath means create new Archive to path of first fileName.
  void CompressDropFiles(const UStringVector &fileNames, const UString &folderPath);

  void RefreshTitle(bool always = false) { _panelCallback->RefreshTitle(always);  }
  void RefreshTitleAlways() { RefreshTitle(true);  }

  UString GetItemsInfoString(const CRecordVector<UInt32> &indices);
  //��ȡ����������
  void GetDriverName(UString &s,int number);
  //������
  void CheckUpdate();
  int GetDriverNumber();
};

class CMyBuffer
{
  void *_data;
public:
  CMyBuffer(): _data(0) {}
  operator void *() { return _data; }
  bool Allocate(size_t size)
  {
    if (_data != 0)
      return false;
    _data = ::MidAlloc(size);
    return _data != 0;
  }
  ~CMyBuffer() { ::MidFree(_data); }
};

#endif
