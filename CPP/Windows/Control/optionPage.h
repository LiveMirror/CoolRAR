// Windows/Control/optionPage.h

#ifndef __WINDOWS_CONTROL_OPTIONPAGE_H
#define __WINDOWS_CONTROL_OPTIONPAGE_H

#include "Dialog.h"

namespace NWindows {
namespace NControl {

INT_PTR APIENTRY OptionPageProcedure(HWND dialogHWND, UINT message, WPARAM wParam, LPARAM lParam);

class COptionPage: public CDialog
{
public:
  COptionPage(HWND window = NULL): CDialog(window){};
  
  void Changed() { PropSheet_Changed(GetParent(), (HWND)*this); }
  void UnChanged() { PropSheet_UnChanged(GetParent(), (HWND)*this); }

  virtual bool OnNotify(UINT controlID, LPNMHDR lParam);

  virtual bool OnKillActive() { return false; } // false = OK
  virtual bool OnKillActive(const PSHNOTIFY *) { return OnKillActive(); }
  virtual LONG OnSetActive() { return false; } // false = OK
  virtual LONG OnSetActive(const PSHNOTIFY *) { return OnSetActive(); }
  virtual void OnNotifyHelp() {}
  virtual void OnNotifyHelp(const PSHNOTIFY *) { OnNotifyHelp(); }
  virtual void OnReset() {}
  virtual void OnReset(const PSHNOTIFY *) { OnReset(); }
};

struct CPgInfo
{
  COptionPage *Page;
  UString Title;
  UINT ID;
};

INT_PTR MyOptionPageSheet(const CObjectVector<CPgInfo> &pagesInfo, HWND hwndParent, const UString &title);

}}

#endif
