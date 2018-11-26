// RegistryAssociations.h

#ifndef __REGISTRYASSOCIATIONS_H
#define __REGISTRYASSOCIATIONS_H

#include "Common/MyString.h"

namespace NRegistryAssociations {

  bool CheckShellExtensionInfo(const CSysString &extension, UString &iconPath, int &iconIndex);

  void DeleteShellExtensionInfo(const CSysString &extension);

  void AddShellExtensionInfo(const CSysString &extension,
      const UString &programTitle,
      const UString &programOpenCommand,
      const UString &iconPath, int iconIndex,
      const void *shellNewData, int shellNewDataSize);

}

#endif
