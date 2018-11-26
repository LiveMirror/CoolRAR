// SettingsPage.h
 
#ifndef __SETTINGSPAGE_H
#define __SETTINGSPAGE_H

#include "Windows/Control/PropertyPage.h"
#include "Windows/Control/Edit.h"
struct FileNameStr
{
	UString FilePath;
	UString  FileName;
};
class CSettingsPage: public NWindows::NControl::CPropertyPage
{
  bool OnButtonClicked(int buttonID, HWND buttonHWND);
  bool FindShortcut();
public:
  virtual bool OnInit();
  virtual void OnNotifyHelp();
  virtual LONG OnApply();
};

#endif
