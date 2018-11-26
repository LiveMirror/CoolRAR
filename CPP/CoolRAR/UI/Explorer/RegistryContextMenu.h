// RegistryContextMenu.h

#ifndef __REGISTRY_CONTEXT_MENU_H
#define __REGISTRY_CONTEXT_MENU_H

namespace NZipRootRegistry {

#ifndef UNDER_CE
  bool CheckContextMenuHandler();
  bool CheckDragDropHandler();
  void AddContextMenuHandler();
  // void AddDropHandler();
  void DeleteContextMenuHandler();
#endif

}

#endif
