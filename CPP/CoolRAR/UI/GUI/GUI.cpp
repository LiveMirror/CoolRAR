// GUI.cpp

#include "StdAfx.h"

#include "Common/MyInitGuid.h"
#include "../FileManager/resourceGui.h"
#include "../../../../C/Alloc.h"

#include "Common/DynamicBuffer.h"
#include "Common/StringConvert.h"
#include "Common/CommandLineParser.h"
#include "Common/MyException.h"
#include "Common/StringConvert.h"

#include "Windows/Error.h"
#include "Windows/NtCheck.h"
#ifdef _WIN32
#include "Windows/MemoryLock.h"
#endif

#include "../Common/ArchiveCommandLine.h"
#include "../Common/ExitCode.h"

#include "../FileManager/StringUtils.h"

#include "BenchmarkDialog.h"
#include "ExtractGUI.h"
#include "UpdateGUI.h"

#include "ExtractRes.h"
#include "Windows/Registry.h"

#include "../Common/ZipRegistry.h"

#include "Windows/Window.h"
#include "Windows/DLL.h"
#include "Common/UTFConvert.h"
#include "../FileManager/ProgramLocation.h"
#include "../FileManager/ExceptionHandler.h"

using namespace NWindows;
using namespace NRegistry;
HINSTANCE g_hInstance;
enum DeleteMode
{
	deleteNull = 1,
	deleteByAsk,
	deleteNoAsk
};

#ifndef _UNICODE
#endif

#ifdef UNDER_CE
bool g_LVN_ITEMACTIVATE_Support = true;
#endif

static NSynchronization::CCriticalSection g_CS;
static const TCHAR *kKeyName = TEXT("Extraction");
static const TCHAR *kDeleteMode = TEXT("DeleteSourceFile");
static const TCHAR *kCuPrefix = TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("CoolRAR") TEXT(STRING_PATH_SEPARATOR);

static void ErrorMessage(LPCWSTR message)
{
  MessageBoxW(NULL, message, L"CoolRAR", MB_ICONERROR | MB_OK);
}

static void ErrorLangMessage(UINT resourceID, UInt32 langID)
{
  ErrorMessage(LangString(resourceID, langID));
}

static const char *kNoFormats = "CoolRAR cannot find the code that works with archives.";
static const wchar_t *kDescriptionFileName = L"descript.ion";

static int ShowMemErrorMessage()
{
  ErrorLangMessage(IDS_MEM_ERROR, 0x0200060B);
  return NExitCode::kMemoryError;
}

static int ShowSysErrorMessage(DWORD errorCode)
{
  if (errorCode == E_OUTOFMEMORY)
    return ShowMemErrorMessage();
  ErrorMessage(HResultToMessage(errorCode));
  return NExitCode::kFatalError;
}
//////////////注册表操作函数///////////////////
static CSysString GetKeyPath(const CSysString &path) { return kCuPrefix + path; }
static LONG OpenMainKey(CKey &key, LPCTSTR keyName)
{
	return key.Open(HKEY_CURRENT_USER, GetKeyPath(keyName), KEY_READ);
}

static LONG CreateMainKey(CKey &key, LPCTSTR keyName)
{
	return key.Create(HKEY_CURRENT_USER, GetKeyPath(keyName));
}
static int GetExtractDeleteMode()
{
	int deletemode = 1;
	NSynchronization::CCriticalSectionLock lock(g_CS);
	CKey key;
	if (OpenMainKey(key, kKeyName) == ERROR_SUCCESS)
	{   
		UInt32 deletevalue;
		if( key.QueryValue(kDeleteMode,deletevalue) != ERROR_SUCCESS )
		{
			deletemode =1;
		}
		else
			deletemode = deletevalue;	
	}
	return deletemode;
}

static bool IsAscii(const UString &testString)
{
	for (int i = 0; i < testString.Length(); i++)
		if (testString[i] >= 0x80)
			return false;
	return true;
}

#ifndef _UNICODE
typedef int (WINAPI * SHFileOperationWP)(LPSHFILEOPSTRUCTW lpFileOp);
#endif
static void MyDeleteFile(const UString path,bool toRecycleBin,HWND parentHwnd,bool toAsk)
{
	CDynamicBuffer<WCHAR> buffer;
	size_t size = 0;
	int maxLen = 0;
	
		// L"\\\\?\\") doesn't work here.
		if (path.Length() > maxLen)
			maxLen = path.Length();
		buffer.EnsureCapacity(size + path.Length() + 1);
		memmove(((WCHAR *)buffer) + size, (const WCHAR *)path, (path.Length() + 1) * sizeof(WCHAR));
		size += path.Length() + 1;
	
	buffer.EnsureCapacity(size + 1);
	((WCHAR *)buffer)[size] = 0;
	if (maxLen >= MAX_PATH)
	{
		if (toRecycleBin)
		{
			return;
		}
		
	}
	else
	{
		SHFILEOPSTRUCTW fo;
		fo.hwnd = parentHwnd;
		fo.wFunc = FO_DELETE;
		fo.pFrom = (const WCHAR *)buffer;
		fo.pTo = 0;
		fo.fFlags = 0;
		if (toAsk)
			fo.fFlags |= FOF_ALLOWUNDO;
		else
			 fo.fFlags |= FOF_NOCONFIRMATION;
		fo.fAnyOperationsAborted = FALSE;
		fo.hNameMappings = 0;
		fo.lpszProgressTitle = 0;
		int res;
#ifdef _UNICODE
		res = ::SHFileOperationW(&fo);
#else
		SHFileOperationWP shFileOperationW = (SHFileOperationWP)
			::GetProcAddress(::GetModuleHandleW(L"shell32.dll"), "SHFileOperationW");
		if (shFileOperationW == 0)
			return;
		res = shFileOperationW(&fo);
#endif
	}
}
static int Main2()
{
  UStringVector sourceFileName;//用以存储源文件名称		
  UStringVector commandStrings;
  NCommandLineParser::SplitCommandLine(GetCommandLineW(), commandStrings);	
  #ifndef UNDER_CE
   if (commandStrings.Size() > 0)
    commandStrings.Delete(0);
  #endif
  if (commandStrings.Size() == 0)
  {
    MessageBoxW(0, LangString(0x07000049), L"CoolRAR", 0);
    return 0;
  }

  CArchiveCommandLineOptions options;
  CArchiveCommandLineParser parser;

  parser.Parse1(commandStrings, options);
  parser.Parse2(options);

  options.UpdateOptions.Sourcefiledelete = false;	
  #if defined(_WIN32) && defined(_7ZIP_LARGE_PAGES)
  if (options.LargePages)
    NSecurity::EnableLockMemoryPrivilege();
  #endif
   
  CCodecs *codecs = new CCodecs;
  CMyComPtr<IUnknown> compressCodecsInfo = codecs;
  HRESULT result = codecs->Load();
  if (result != S_OK)
    throw CSystemException(result);
  
  bool isExtractGroupCommand = options.Command.IsFromExtractGroup();
  if (codecs->Formats.Size() == 0 &&
        (isExtractGroupCommand ||
        options.Command.IsFromUpdateGroup()))
    throw kNoFormats;

  CIntVector formatIndices;
 

  if (!codecs->FindFormatForArchiveType(options.ArcType, formatIndices))
  {
    ErrorLangMessage(IDS_UNSUPPORTED_ARCHIVE_TYPE, 0x0200060D);
    return NExitCode::kFatalError;
  }
 
  if (options.Command.CommandType == NCommandType::kBenchmark)
  {
    HRESULT res;
    #ifdef EXTERNAL_CODECS
    CObjectVector<CCodecInfoEx> externalCodecs;
    res = LoadExternalCodecs(codecs, externalCodecs);
    if (res != S_OK)
      throw CSystemException(res);
    #endif
    res = Benchmark(
      #ifdef EXTERNAL_CODECS
      codecs, &externalCodecs,
      #endif
      options.NumThreads, options.DictionarySize);
    if (res != S_OK)
      throw CSystemException(res);
  }
  else if (isExtractGroupCommand)
  {
    CExtractCallbackImp *ecs = new CExtractCallbackImp;
    CMyComPtr<IFolderArchiveExtractCallback> extractCallback = ecs;

    #ifndef _NO_CRYPTO
    ecs->PasswordIsDefined = options.PasswordEnabled;
    ecs->Password = options.Password;
    #endif

    ecs->Init();
	
    CExtractOptions eo;
    eo.StdOutMode = options.StdOutMode;
    eo.OutputDir = options.OutputDir;
    eo.YesToAll = options.YesToAll;
    eo.OverwriteMode = options.OverwriteMode;
    eo.PathMode = options.Command.GetPathMode();
    eo.TestMode = options.Command.IsTestMode();
    eo.CalcCrc = options.CalcCrc;
    #if !defined(_7ZIP_ST) && !defined(_SFX)
    eo.Properties = options.ExtractProperties;
    #endif

    bool messageWasDisplayed = false;
    HRESULT result = ExtractGUI(codecs, formatIndices,
          options.ArchivePathsSorted,
          options.ArchivePathsFullSorted,
          options.WildcardCensor.Pairs.Front().Head,
          eo, options.ShowDialog, messageWasDisplayed, ecs);
	if (result == S_OK && eo.TestMode != true)
	{
		int deletemode =deleteNull;
			deletemode = GetExtractDeleteMode();//获取注册表中删除源文件的信息
			if (deletemode == deleteNoAsk)
			{
				UString sourfile;
				for (int n =0; n < options.ArchivePathsFullSorted.Size(); n++)
				{
					sourfile = options.ArchivePathsFullSorted.operator [](n);
					MyDeleteFile(sourfile,true,GetActiveWindow(),false);//删除文件
					sourfile.Empty();
				}	 
			}
			if (deletemode == deleteByAsk)
			{
				UString sourfile;
				for (int n =0; n < options.ArchivePathsFullSorted.Size(); n++)
				{
					sourfile = options.ArchivePathsFullSorted.operator [](n);
					MyDeleteFile(sourfile,true,GetActiveWindow(),true);//询问后删除删除文件
					sourfile.Empty();
				}	 
			}
			
	}
    if (result != S_OK)
    {
      if (result != E_ABORT && messageWasDisplayed)
        return NExitCode::kFatalError;
      throw CSystemException(result);
    }
    if (!ecs->IsOK())
      return NExitCode::kFatalError;
  }
  else if (options.Command.IsFromUpdateGroup())
  {
    #ifndef _NO_CRYPTO
    bool passwordIsDefined = options.PasswordEnabled && !options.Password.IsEmpty();
    #endif

    CUpdateCallbackGUI callback;
    

    #ifndef _NO_CRYPTO
    callback.PasswordIsDefined = passwordIsDefined;
    callback.AskPassword = options.PasswordEnabled && options.Password.IsEmpty();
    callback.Password = options.Password;
    #endif

    
    callback.Init();

    if (!options.UpdateOptions.Init(codecs, formatIndices, options.ArchiveName))
    {
      ErrorLangMessage(IDS_UPDATE_NOT_SUPPORTED, 0x02000601);
      return NExitCode::kFatalError;
    }
    bool messageWasDisplayed = false;
    HRESULT result = UpdateGUI(
        codecs,
        options.WildcardCensor, options.UpdateOptions,
        options.ShowDialog,
        messageWasDisplayed, &callback);

	if (result == S_OK)
	{
		if (options.UpdateOptions.ValusComments!=L"")
		{
			UString UpdateFileName;
			UpdateFileName=options.UpdateOptions.ArchivePath.Name+L".";
			UpdateFileName+=options.UpdateOptions.ArchivePath.BaseExtension;
			NWindows::NFile::NIO::COutFile CommentFile;
			UString path;
			GetProgramFolderPath(path);
			if (!CommentFile.Create(path + kDescriptionFileName, true))
				return false;
			UString unicodeString;
			unicodeString=UpdateFileName+L" ";
			unicodeString+=options.UpdateOptions.ValusComments;
			AString utfString;
			ConvertUnicodeToUTF8(unicodeString, utfString);
			UInt32 processedSize;
			if (!IsAscii(unicodeString))
			{
				Byte bom [] = { 0xEF, 0xBB, 0xBF, 0x0D, 0x0A };
				CommentFile.Write(bom , sizeof(bom), processedSize);
			}
			CommentFile.Write(utfString, utfString.Length(), processedSize);
		}
	}
    if (result != S_OK)
    {
		
      if (result != E_ABORT && messageWasDisplayed)
        return NExitCode::kFatalError;
      throw CSystemException(result);
    }
    if (callback.FailedFiles.Size() > 0)
    {
      if (!messageWasDisplayed)
        throw CSystemException(E_FAIL);
      return NExitCode::kWarning;
    }
  }
  else
  {
    throw "Unsupported command";
  }

  
  for (int i = 0; i < options.WildcardCensor.Pairs.Size(); i++)
  {
	  NWildcard::CPair &pair = options.WildcardCensor.Pairs[i];
	  for (int j =0;j < pair.Head.IncludeItems.Size();j++)
	  {
		  for ( int m =0;m < pair.Head.IncludeItems[j].PathParts.Size();m++)
		  {
			   sourceFileName.Add( pair.Head.IncludeItems[j].PathParts[m] );//获得文件名称
		  }
	  }
	  
	  
  }
  if (options.UpdateOptions.Sourcefiledelete &&!sourceFileName.IsEmpty())//如果选择删除源文件的话
  {
	  UString sourfile;
	  for (int n =0; n < sourceFileName.Size(); n++)
	  {
		  sourfile = options.UpdateOptions.WorkingDir +sourceFileName[n];
		  MyDeleteFile(sourfile,true,GetActiveWindow(),false);//删除文件
		  sourfile.Empty();
	  }	 
  }

  return 0;
}

#define NT_CHECK_FAIL_ACTION ErrorMessage(L"Unsupported Windows version"); return NExitCode::kFatalError;

static int MyMain()
{
	try
	{
		return Main2();
	}
	catch(const CNewException &)
	{
		return ShowMemErrorMessage();
	}
	catch(const CArchiveCommandLineException &e)
	{
		ErrorMessage(GetUnicodeString(e));
		return NExitCode::kUserError;
	}
	catch(const CSystemException &systemError)
	{
		if (systemError.ErrorCode == E_ABORT)
			return NExitCode::kUserBreak;
		return ShowSysErrorMessage(systemError.ErrorCode);
	}
	catch(const UString &s)
	{
		ErrorMessage(s);
		return NExitCode::kFatalError;
	}
	catch(const AString &s)
	{
		ErrorMessage(GetUnicodeString(s));
		return NExitCode::kFatalError;
	}
	catch(const wchar_t *s)
	{
		ErrorMessage(s);
		return NExitCode::kFatalError;
	}
	catch(const char *s)
	{
		ErrorMessage(GetUnicodeString(s));
		return NExitCode::kFatalError;
	}
	catch(...)
	{
		ErrorLangMessage(IDS_UNKNOWN_ERROR, 0x0200060C);
		return NExitCode::kFatalError;
	}
}
int APIENTRY WinMain(HINSTANCE  hInstance, HINSTANCE /* hPrevInstance */,
  #ifdef UNDER_CE
  LPWSTR
  #else
  LPSTR
  #endif
  /* lpCmdLine */, int /* nCmdShow */)
{	
  g_hInstance = hInstance;

#ifdef _WIN32
  NT_CHECK
  SetLargePageSize();
  #endif

  InitCommonControls();

  ReloadLang();
  int ret = 0;
  __try
  {
	  
	ret = MyMain();
	 
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

