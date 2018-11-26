// FileFolderPluginOpen.cpp

#include "StdAfx.h"

#include "resource.h"

#include "Windows/Thread.h"

#include "../Agent/Agent.h"

#include "LangUtils.h"
#include "OpenCallback.h"
#include "PluginLoader.h"
#include "RegistryAssociations.h"
#include "RegistryPlugins.h"

using namespace NWindows;
using namespace NRegistryAssociations;

struct CThreadArchiveOpen
{
  UString Path;
  CMyComPtr<IInStream> InStream;
  CMyComPtr<IFolderManager> FolderManager;
  CMyComPtr<IProgress> OpenCallback;
  COpenArchiveCallback *OpenCallbackSpec;

  CMyComPtr<IFolderFolder> Folder;
  HRESULT Result;

  void Process()
  {
    try
    {
      CProgressCloser closer(OpenCallbackSpec->ProgressDialog);
      Result = FolderManager->OpenFolderFile(InStream, Path, &Folder, OpenCallback);
    }
    catch(...) { Result = E_FAIL; }
  }
  
  static THREAD_FUNC_DECL MyThreadFunction(void *param)
  {
    ((CThreadArchiveOpen *)param)->Process();
    return 0;
  }
};



HRESULT OpenFileFolderPlugin(
    IInStream *inStream,
    const UString &path,
    HMODULE *module,
    IFolderFolder **resultFolder,
    HWND parentWindow,
    bool &encrypted, UString &password)
{
  CObjectVector<CPluginInfo> plugins;
  ReadFileFolderPluginInfoList(plugins);

  UString extension, name, pureName, dot;

  int slashPos = path.ReverseFind(WCHAR_PATH_SEPARATOR);
  UString dirPrefix;
  UString fileName;
  if (slashPos >= 0)
  {
    dirPrefix = path.Left(slashPos + 1);
    fileName = path.Mid(slashPos + 1);
  }
  else
    fileName = path;

  NFile::NName::SplitNameToPureNameAndExtension(fileName, pureName, dot, extension);

 

  for (int i = 0; i < plugins.Size(); i++)
  {
    const CPluginInfo &plugin = plugins[i];
    if (!plugin.ClassIDDefined)
      continue;
    CPluginLibrary library;

    CThreadArchiveOpen t;

    if (plugin.FilePath.IsEmpty())
      t.FolderManager = new CArchiveFolderManager;
    else if (library.LoadAndCreateManager(plugin.FilePath, plugin.ClassID, &t.FolderManager) != S_OK)
      continue;

    t.OpenCallbackSpec = new COpenArchiveCallback;
    t.OpenCallback = t.OpenCallbackSpec;
    t.OpenCallbackSpec->PasswordIsDefined = encrypted;
    t.OpenCallbackSpec->Password = password;
    t.OpenCallbackSpec->ParentWindow = parentWindow;

    if (inStream)
      t.OpenCallbackSpec->SetSubArchiveName(fileName);
    else
      t.OpenCallbackSpec->LoadFileInfo(dirPrefix, fileName);

    t.InStream = inStream;
    t.Path = path;

    UString progressTitle = LangString(IDS_OPENNING, 0x03020283);
    t.OpenCallbackSpec->ProgressDialog.MainWindow = parentWindow;
    t.OpenCallbackSpec->ProgressDialog.MainTitle = LangString(IDS_APP_TITLE, 0x03000000);
    t.OpenCallbackSpec->ProgressDialog.MainAddTitle = progressTitle + UString(L" ");
    t.OpenCallbackSpec->ProgressDialog.WaitMode = true;

    {
      NWindows::CThread thread;
      RINOK(thread.Create(CThreadArchiveOpen::MyThreadFunction, &t));
      t.OpenCallbackSpec->StartProgressDialog(progressTitle, thread);
    }

    if (t.Result == E_ABORT)
      return t.Result;

    if (t.Result == S_OK)
    {
      {
        encrypted = t.OpenCallbackSpec->PasswordIsDefined;
        password = t.OpenCallbackSpec->Password;
      }
      *module = library.Detach();
      *resultFolder = t.Folder.Detach();
      return S_OK;
    }
    
    if (t.Result != S_FALSE)
      return t.Result;
  }
  return S_FALSE;
}