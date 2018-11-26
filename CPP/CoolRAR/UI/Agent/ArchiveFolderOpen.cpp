// Zip/ArchiveFolder.cpp

#include "StdAfx.h"

#include "Agent.h"

#include "Common/StringConvert.h"

extern HINSTANCE g_hInstance;

static inline UINT GetCurrentFileCodePage()
{
  #ifdef UNDER_CE
  return CP_ACP;
  #else
  return AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  #endif
}

void CArchiveFolderManager::LoadFormats()
{
  if (!_codecs)
  {
    _compressCodecsInfo = _codecs = new CCodecs;
    _codecs->Load();
  }
}

int CArchiveFolderManager::FindFormat(const UString &type)
{
  for (int i = 0; i < _codecs->Formats.Size(); i++)
    if (type.CompareNoCase(_codecs->Formats[i].Name) == 0)
      return i;
  return -1;
}

STDMETHODIMP CArchiveFolderManager::OpenFolderFile(IInStream *inStream, const wchar_t *filePath,
    IFolderFolder **resultFolder, IProgress *progress)
{
  CMyComPtr<IArchiveOpenCallback> openArchiveCallback;
  if (progress != 0)
  {
    CMyComPtr<IProgress> progressWrapper = progress;
    progressWrapper.QueryInterface(IID_IArchiveOpenCallback, &openArchiveCallback);
  }
  CAgent *agent = new CAgent();
  CMyComPtr<IInFolderArchive> archive = agent;
  RINOK(agent->Open(inStream, filePath, NULL, openArchiveCallback));
  return agent->BindToRootFolder(resultFolder);
}


static void AddIconExt(const CCodecIcons &lib, UString &dest)
{
  for (int j = 0; j < lib.IconPairs.Size(); j++)
  {
    if (!dest.IsEmpty())
      dest += L' ';
    dest += lib.IconPairs[j].Ext;
  }
}

STDMETHODIMP CArchiveFolderManager::GetExtensions(BSTR *extensions)
{
  LoadFormats();
  *extensions = 0;
  UString res;
  for (int i = 0; i < _codecs->Libs.Size(); i++)
    AddIconExt(_codecs->Libs[i], res);
  AddIconExt(_codecs->InternalIcons, res);
  return StringToBstr(res, extensions);
}

STDMETHODIMP CArchiveFolderManager::GetIconPath(const wchar_t *ext, BSTR *iconPath, Int32 *iconIndex)
{
  LoadFormats();
  *iconPath = 0;
  *iconIndex = 0;
  for (int i = 0; i < _codecs->Libs.Size(); i++)
  {
    const CCodecLib &lib = _codecs->Libs[i];
    int ii;
    if (lib.FindIconIndex(ext, ii))
    {
      *iconIndex = ii;
      return StringToBstr(GetUnicodeString(lib.Path, GetCurrentFileCodePage()), iconPath);
    }
  }
  int ii;
  if (_codecs->InternalIcons.FindIconIndex(ext, ii))
  {
    *iconIndex = ii;
    UString path;
    NWindows::NDLL::MyGetModuleFileName(g_hInstance, path);
    return StringToBstr(path, iconPath);
  }
  return S_OK;
}

