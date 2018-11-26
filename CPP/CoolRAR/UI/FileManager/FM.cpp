// FM.cpp

#include "StdAfx.h"

#include "../../../../C/Alloc.h"

#include "Windows/Error.h"
#include "Windows/MemoryLock.h"
#include "Windows/NtCheck.h"

#ifndef UNDER_CE
#include "Windows/Security.h"
#endif

#include "../GUI/ExtractRes.h"

#include "resource.h"

#include "App.h"
#include "FormatUtils.h"
#include "LangUtils.h"
#include "MyLoadMenu.h"
#include "Panel.h"
#include "RegistryUtils.h"
#include "StringUtils.h"
#include "ViewSettings.h"
#include "HttpUpdat.h"
#include "../Common/ArchiveCommandLine.h"
#include "ExceptionHandler.h"


using namespace NWindows;
using namespace NFile;
using namespace NFind;

#define MAX_LOADSTRING 100

#define MENU_HEIGHT 26

HINSTANCE g_hInstance;
HWND g_HWND;
bool _isXP;
bool _needReopen;
HttpUpdat httpworkdat;
bool g_OpenArchive = false;
static UString g_MainPath;
static bool g_Maximized = false;

#ifndef UNDER_CE
DWORD g_ComCtl32Version;
#endif

bool g_IsSmallScreen = false;

bool g_LVN_ITEMACTIVATE_Support = true;
// LVN_ITEMACTIVATE replaces both NM_DBLCLK & NM_RETURN
// Windows 2000
// NT/98 + IE 3 (g_ComCtl32Version >= 4.70)


const int kNumDefaultPanels = 1;

const int kSplitterWidth = 4;
int kSplitterRateMax = 1 << 16;
int kPanelSizeMin = 120;

// bool OnMenuCommand(HWND hWnd, int id);

class CSplitterPos
{
  int _ratio; // 10000 is max
  int _pos;
  int _fullWidth;
  void SetRatioFromPos(HWND hWnd)
    { _ratio = (_pos + kSplitterWidth / 2) * kSplitterRateMax /
        MyMax(GetWidth(hWnd), 1); }
public:
  int GetPos() const
    { return _pos; }
  int GetWidth(HWND hWnd) const
  {
    RECT rect;
    ::GetClientRect(hWnd, &rect);
    return rect.right;
  }
  void SetRatio(HWND hWnd, int aRatio)
  {
    _ratio = aRatio;
    SetPosFromRatio(hWnd);
  }
  void SetPosPure(HWND hWnd, int pos)
  {
    int posMax = GetWidth(hWnd) - kSplitterWidth;
    if (posMax < kPanelSizeMin * 2)
      pos = posMax / 2;
    else
    {
      if (pos > posMax - kPanelSizeMin)
        pos = posMax - kPanelSizeMin;
      else if (pos < kPanelSizeMin)
        pos = kPanelSizeMin;
    }
    _pos = pos;
  }
  void SetPos(HWND hWnd, int pos)
  {
    _fullWidth = GetWidth(hWnd);
    SetPosPure(hWnd, pos);
    SetRatioFromPos(hWnd);
  }
  void SetPosFromRatio(HWND hWnd)
  {
    int fullWidth = GetWidth(hWnd);
    if (_fullWidth != fullWidth && fullWidth != 0)
    {
      _fullWidth = fullWidth;
      SetPosPure(hWnd, GetWidth(hWnd) * _ratio / kSplitterRateMax - kSplitterWidth / 2);
    }
  }
};

static bool g_CanChangeSplitter = false;
static UINT32 g_SplitterPos = 0;
static CSplitterPos g_Splitter;
static bool g_PanelsInfoDefined = false;

static int g_StartCaptureMousePos;
static int g_StartCaptureSplitterPos;

CApp g_App;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

const wchar_t *kWindowClass = L"FM";

#ifdef UNDER_CE
#define WS_OVERLAPPEDWINDOW ( \
  WS_OVERLAPPED   | \
  WS_CAPTION      | \
  WS_SYSMENU      | \
  WS_THICKFRAME   | \
  WS_MINIMIZEBOX  | \
  WS_MAXIMIZEBOX)
#endif

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  CWindow wnd;

  g_hInstance = hInstance;

  ReloadLangSmart();

  UString title = LangString(IDS_APP_TITLE, 0x03000000);


  WNDCLASSW wc;

  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC) WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
  
  wc.hCursor = ::LoadCursor(0, IDC_SIZEWE);
  wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);

  wc.lpszMenuName =
    #ifdef UNDER_CE
    0
    #else
    MAKEINTRESOURCEW(IDM_MENU)
    #endif
    ;

  wc.lpszClassName = kWindowClass;

  MyRegisterClass(&wc);


  DWORD style = WS_OVERLAPPEDWINDOW;
  
  RECT rect;
  bool maximized = false;
  int x , y, xSize, ySize;
  x = y = xSize = ySize = CW_USEDEFAULT;
  bool windowPosIsRead = ReadWindowSize(rect, maximized);

  if (windowPosIsRead)
  {
    
    xSize = rect.right - rect.left;
    ySize = rect.bottom - rect.top;
  }
  else
  {
	  xSize = 1024;
	  ySize = 540;
	  
	  if(GetSystemMetrics(SM_CXSCREEN) < xSize) 
	  {
		  xSize = 600;
		  ySize = 280;
	  }
	 
	  
  }

  UINT32 numPanels, currentPanel;
  g_PanelsInfoDefined = ReadPanelsInfo(numPanels, currentPanel, g_SplitterPos);
  if (g_PanelsInfoDefined)
  {
    if (numPanels < 1 || numPanels > 2)
      numPanels = kNumDefaultPanels;
    if (currentPanel >= 2)
      currentPanel = 0;
  }
  else
  {
    numPanels = kNumDefaultPanels;
    currentPanel = 0;
  }
  g_App.NumPanels = numPanels;
  g_App.LastFocusedPanel = currentPanel;

  if (!wnd.Create(kWindowClass, title, style,
    x, y, xSize, ySize, NULL, NULL, hInstance, NULL))
    return FALSE;

  if (nCmdShow == SW_SHOWNORMAL ||
      nCmdShow == SW_SHOW
      #ifndef UNDER_CE
      || nCmdShow == SW_SHOWDEFAULT
      #endif
      )
  {
    if (maximized)
      nCmdShow = SW_SHOWMAXIMIZED;
    else
      nCmdShow = SW_SHOWNORMAL;
  }

  if (nCmdShow == SW_SHOWMAXIMIZED)
    g_Maximized = true;

  #ifndef UNDER_CE
  WINDOWPLACEMENT placement;
  placement.length = sizeof(placement);
  if (wnd.GetPlacement(&placement))
  {
    if (windowPosIsRead)
      placement.rcNormalPosition = rect;
    placement.showCmd = nCmdShow;
    wnd.SetPlacement(&placement);
  }
  else
  #endif
    wnd.Show(nCmdShow);

  return TRUE;
}


#ifndef UNDER_CE
static DWORD GetDllVersion(LPCTSTR lpszDllName)
{
  HINSTANCE hinstDll;
  DWORD dwVersion = 0;
  hinstDll = LoadLibrary(lpszDllName);
  if (hinstDll)
  {
    DLLGETVERSIONPROC pDllGetVersion;
    pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
    
    /*Because some DLLs might not implement this function, you
    must test for it explicitly. Depending on the particular
    DLL, the lack of a DllGetVersion function can be a useful
    indicator of the version.
    */
    if (pDllGetVersion)
    {
      DLLVERSIONINFO dvi;
      HRESULT hr;
      
      ZeroMemory(&dvi, sizeof(dvi));
      dvi.cbSize = sizeof(dvi);
      
      hr = (*pDllGetVersion)(&dvi);
      
      if (SUCCEEDED(hr))
      {
        dwVersion = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
      }
    }
    FreeLibrary(hinstDll);
  }
  return dwVersion;
}
#endif

bool IsLargePageSupported()
{
  #ifdef _WIN64
  return true;
  #else
  OSVERSIONINFO versionInfo;
  versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
  if (!::GetVersionEx(&versionInfo))
    return false;
  if (versionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT || versionInfo.dwMajorVersion < 5)
    return false;
  if (versionInfo.dwMajorVersion > 5)
    return true;
  if (versionInfo.dwMinorVersion < 1)
    return false;
  if (versionInfo.dwMinorVersion > 1)
    return true;
  return false;
  #endif
}

#ifndef UNDER_CE
static void SetMemoryLock()
{
  if (!IsLargePageSupported())
    return;
    NSecurity::AddLockMemoryPrivilege();

  if (ReadLockMemoryEnable())
    NSecurity::EnableLockMemoryPrivilege();
}
#endif


#define NT_CHECK_FAIL_ACTION MessageBoxW(0, L"Unsupported Windows version", L"CoolRAR", MB_ICONERROR); return 1;
extern bool WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize);

int DoWinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */,
#ifdef UNDER_CE
LPWSTR
#else
 LPSTR
#endif
/* lpCmdLine */, int nCmdShow)
{

#ifdef _WIN32

	MSG msg;

	NT_CHECK
		SetLargePageSize();

#endif

	InitCommonControls();
	UStringVector commandStrings;
	NCommandLineParser::SplitCommandLine(GetCommandLineW(), commandStrings);

#ifndef UNDER_CE
	if (commandStrings.Size() > 0)
		commandStrings.Delete(0);
#endif
		_isXP = true;
		_needReopen = false;	
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (::GetVersionEx(&vi) && vi.dwMajorVersion  != 5)//检查系统版本
		{
			_needReopen = true;
			_isXP = false;
		}
		if(commandStrings.Size() != 0)
			_needReopen = false;
	

#ifndef UNDER_CE
	g_ComCtl32Version = ::GetDllVersion(TEXT("comctl32.dll"));
	g_LVN_ITEMACTIVATE_Support = (g_ComCtl32Version >= MAKELONG(71, 4));
#endif

	g_IsSmallScreen = !NWindows::NControl::IsDialogSizeOK(200, 200);

	// OleInitialize is required for drag and drop.
#ifndef UNDER_CE
	OleInitialize(NULL);
#endif
	// Maybe needs CoInitializeEx also ?

	UString commandsString;

#ifdef UNDER_CE
	commandsString = GetCommandLineW();
#else
	UString programString;
	SplitStringToTwoStrings(GetCommandLineW(), programString, commandsString);
#endif

	commandsString.Trim();
	UString paramString, tailString;
	SplitStringToTwoStrings(commandsString, paramString, tailString);
	paramString.Trim();

	if (!paramString.IsEmpty())
	{
		g_MainPath = paramString;

	}



#ifndef UNDER_CE
	SetMemoryLock();
#endif


	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

#ifndef _UNICODE
	if (g_IsNT)
	{
		HACCEL hAccels = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDR_ACCELERATOR1));
		while (GetMessageW(&msg, NULL, 0, 0))
		{
			if (TranslateAcceleratorW(g_HWND, hAccels, &msg) == 0)
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
	}
	else
#endif
	{
		HACCEL hAccels = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (TranslateAccelerator(g_HWND, hAccels, &msg) == 0)
			{

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}


	g_HWND = 0;
#ifndef UNDER_CE
	OleUninitialize();
#endif

	return (int)msg.wParam;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
    #ifdef UNDER_CE
    LPWSTR
    #else
    LPSTR
    #endif
    /* lpCmdLine */, int nCmdShow)
{
	int ret = 0;
	__try
	{
		ret = DoWinMain(hInstance, NULL, NULL, nCmdShow);
	}
	__except(RecordExceptionInfo(GetExceptionInformation(), "WinMain"))
	{
		// Do nothing here - RecordExceptionInfo() has already done
		// everything that is needed. Actually this code won't even
		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
		// the __except clause.
	}

	return ret;
}

static void SaveWindowInfo(HWND aWnd)
{
  #ifdef UNDER_CE
  RECT rect;
  if (!::GetWindowRect(aWnd, &rect))
    return;
  SaveWindowSize(rect, g_Maximized);
  #else
  WINDOWPLACEMENT placement;
  placement.length = sizeof(placement);
  if (!::GetWindowPlacement(aWnd, &placement))
    return;
  SaveWindowSize(placement.rcNormalPosition, BOOLToBool(::IsZoomed(aWnd)));
  #endif
  SavePanelsInfo(g_App.NumPanels, g_App.LastFocusedPanel, g_Splitter.GetPos());
}

static void ExecuteCommand(UINT commandID)
{
  CPanel::CDisableTimerProcessing disableTimerProcessing1(g_App.Panels[0]);
  CPanel::CDisableTimerProcessing disableTimerProcessing2(g_App.Panels[1]);

  switch (commandID)
  {
    case kAddCommand: 
		g_App.AddToArchive(); 
		break;
    case kExtractCommand: 
		g_App.ExtractArchives();
		break;
    case kTestCommand:
		g_App.TestArchives();
		break;
  }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent;
  switch (message)
  {
    case WM_COMMAND:
      wmId    = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      if ((HWND) lParam != NULL && wmEvent != 0)
        break;
      if (wmId >= kToolbarStartID)
      {
        ExecuteCommand(wmId);
        return 0;
      }
      if (OnMenuCommand(hWnd, wmId))
        return 0;
      break;
    case WM_INITMENUPOPUP:
      OnMenuActivating(hWnd, HMENU(wParam), LOWORD(lParam));
      break;

   

    case WM_CREATE:
    {
      g_HWND = hWnd;
      if (g_PanelsInfoDefined)
        g_Splitter.SetPos(hWnd, g_SplitterPos);
      else
      {
        g_Splitter.SetRatio(hWnd, kSplitterRateMax / 2);
        g_SplitterPos = g_Splitter.GetPos();
      }

      RECT rect;
      ::GetClientRect(hWnd, &rect);
      int xSize = rect.right;
      int xSizes[2];
      xSizes[0] = g_Splitter.GetPos();
      xSizes[1] = xSize - kSplitterWidth - xSizes[0];
      if (xSizes[1] < 0)
        xSizes[1] = 0;

      g_App.CreateDragTarget();
      bool archiveIsOpened;
      bool encrypted;
      bool needOpenFile = false;
      if (!g_MainPath.IsEmpty() /* && g_OpenArchive */)
      {
        if (NFile::NFind::DoesFileExist(g_MainPath))
          needOpenFile = true;
      }
      HRESULT res = g_App.Create(hWnd, g_MainPath, xSizes, archiveIsOpened, encrypted);

      if (res == E_ABORT)
      {
        return -1;
      }
      if (needOpenFile && !archiveIsOpened || res != S_OK)
      {
        UString message = L"Error";
        if (res == S_FALSE || res == S_OK)
        {
          if (encrypted)
            message = MyFormatNew(IDS_CANT_OPEN_ENCRYPTED_ARCHIVE, 0x0200060A, g_MainPath);
          else
            message = MyFormatNew(IDS_CANT_OPEN_ARCHIVE, 0x02000609, g_MainPath);
        }
        else
        {
          if (res != S_OK)
          {
            if (res == E_OUTOFMEMORY)
              message = LangString(IDS_MEM_ERROR, 0x0200060B);
            else
              if (!NError::MyFormatMessage(res, message))
                message = L"Error";
          }
        }
        MessageBoxW(0, message, L"CoolRAR", MB_ICONERROR);
        return -1;
      }
      RegisterDragDrop(hWnd, g_App._dropTarget);

      break;
    }
    case WM_DESTROY:
    {
      RevokeDragDrop(hWnd);
      g_App._dropTarget.Release();

      g_App.Save();
      g_App.Release();
      SaveWindowInfo(hWnd);
      PostQuitMessage(0);
      break;
    }
    case WM_LBUTTONDOWN:
      g_StartCaptureMousePos = LOWORD(lParam);
      g_StartCaptureSplitterPos = g_Splitter.GetPos();
      ::SetCapture(hWnd);
	  
      break;
    case WM_LBUTTONUP:
    {
      ::ReleaseCapture();
      break;
    }
    case WM_MOUSEMOVE:
    {
      if ((wParam & MK_LBUTTON) != 0 && ::GetCapture() == hWnd)
      {
        g_Splitter.SetPos(hWnd, g_StartCaptureSplitterPos +
            (short)LOWORD(lParam) - g_StartCaptureMousePos);
        g_App.MoveSubWindows();
      }

      break;

    }

    case WM_SIZE:
    {
      if (g_CanChangeSplitter)
        g_Splitter.SetPosFromRatio(hWnd);
      else
      {
        g_Splitter.SetPos(hWnd, g_SplitterPos );
        g_CanChangeSplitter = true;
      }
     
      g_Maximized = (wParam == SIZE_MAXIMIZED) || (wParam == SIZE_MAXSHOW);

      g_App.MoveSubWindows();
      
      return 0;
      break;
    }
    case WM_SETFOCUS:
      g_App.SetFocusToLastItem();
      break;
    
    case WM_NOTIFY:
    {
      g_App.OnNotify((int)wParam, (LPNMHDR)lParam);
      break;
    }
  }
  #ifndef _UNICODE
  if (g_IsNT)
    return DefWindowProcW(hWnd, message, wParam, lParam);
  else
  #endif
    return DefWindowProc(hWnd, message, wParam, lParam);

}

static int Window_GetRealHeight(NWindows::CWindow &w)
{
  RECT rect;
  w.GetWindowRect(&rect);
  int res = rect.bottom - rect.top;
  #ifndef UNDER_CE
  WINDOWPLACEMENT placement;
  if (w.GetPlacement(&placement))
    res += placement.rcNormalPosition.top;
  #endif
  return res;
}

void CApp::MoveSubWindows()
{
  HWND hWnd = _window;
  RECT rect;
  if (hWnd == 0)
    return;
  ::GetClientRect(hWnd, &rect);
  int xSize = rect.right;
  if (xSize == 0)
    return;
  int headerSize = 0;
  #ifdef UNDER_CE
  _commandBar.AutoSize();
  {
    _commandBar.Show(true); // maybe we need it for
    headerSize += _commandBar.Height();
  }
  #endif
  if (_toolBar)
  {
    _toolBar.AutoSize();
    #ifdef UNDER_CE
    int h2 = Window_GetRealHeight(_toolBar);
    _toolBar.Move(0, headerSize, xSize, h2);
    #endif
    headerSize += Window_GetRealHeight(_toolBar);
  }
  int ySize = MyMax((int)(rect.bottom - headerSize), 0);
  
  if (NumPanels > 1)
  {
    Panels[0].Move(0, headerSize, g_Splitter.GetPos(), ySize);
    int xWidth1 = g_Splitter.GetPos() + kSplitterWidth;
    Panels[1].Move(xWidth1, headerSize, xSize - xWidth1, ySize);
  }
  else
  {
    Panels[LastFocusedPanel].Move(0, headerSize, xSize, ySize);
  }
}
