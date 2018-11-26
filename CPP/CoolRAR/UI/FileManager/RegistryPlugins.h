// RegistryPlugins.h

#ifndef __REGISTRYPLUGINS_H
#define __REGISTRYPLUGINS_H

#include "Common/MyString.h"

enum EPluginType
{
  kPluginTypeFF = 0
};

struct CPluginInfo
{
  UString FilePath;
  EPluginType Type;
  UString Name;
  CLSID ClassID;
  CLSID OptionsClassID;
  bool ClassIDDefined;
  bool OptionsClassIDDefined;

};

void ReadPluginInfoList(CObjectVector<CPluginInfo> &plugins);
void ReadFileFolderPluginInfoList(CObjectVector<CPluginInfo> &plugins);

#endif
